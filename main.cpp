#include <atomic>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <limits>
#include <thread>
#include <set>
#include <regex>
#include <string>
#include <vector>

#include <getopt.h>
#include <modbus/modbus.h>

#include "modbus_registers.h"
#include "struct.h"
#include "cli_commands.hpp"
#include "Prompt.hpp"

using namespace cli;

void write_settings_to_file(const std::filesystem::path &file);
void read_registers_from_file(const std::filesystem::path &file);

int writeMultipleRegisters(uint16_t *registers, uint16_t addr, uint16_t count);
int updateHoldingRegister(uint16_t from, uint16_t to);
int updateHoldingRegister(uint16_t reg);
int updateInputRegister(uint16_t from, uint16_t to);
int updateInputRegister(uint16_t reg);
int writeRegister(uint16_t reg, uint16_t value);

std::atomic<bool> g_monitor_enable = false;
Prompt my_prompt("AHU_2040");

uint16_t *holdingRegisters{nullptr};
uint16_t inputRegisters[e_input_last_item];

std::vector<std::string> monitor_names;
std::vector<uint16_t> monitor_registers;

std::map<std::string, uint16_t> monitored;

modbus_t *ctx;
std::mutex monitor_mutex;
std::mutex modbus_mutex;

bool isInt(const std::string &s)
{
    try
    {
        size_t pos;
        std::stoi(s, &pos);
        return pos == s.size();
    }
    catch (...)
    {
        return false;
    }
}

void monitor_add(const std::string &str)
{
    std::unique_lock lk(monitor_mutex);
    Tokens tokens = tokenize(str);
    if (tokens.size() != 2)
    {
        printf("Wrong syntax\n");
        return;
    }
    if (isInt(tokens[1]))
    {
        monitored.emplace(tokens[0], std::stoi(tokens[1]));
        printf("Added %s = %u\n", tokens[0].c_str(), monitored[tokens[0]]);
    }
    else
    {
        printf("second parameter must be positive integer\n");
    }
}

void monitor_remove(const std::string &str)
{
    std::unique_lock lk(monitor_mutex);
    auto it = monitored.find(str);
    if (it != monitored.end())
    {
        printf("Removing (%s)\n", str.c_str());
        monitored.erase(it);
    }
    else
    {
        printf("No such register in monitor map (%s)\n", str.c_str());
    }
}

void monitor_show()
{
    std::unique_lock lk(monitor_mutex);
    for (const auto &element : monitored)
    {
        printf("%s = %u (%s)\n", element.first.c_str(), element.second, inputRegToStr(element.second));
    }
}

void set_monitor(const std::string &str)
{
    std::unique_lock lk(monitor_mutex);
    Tokens tokens = tokenize(str);
    monitor_names.clear();
    monitor_registers.clear();

    for (const auto &myToken : tokens)
    {
        auto equal_pos = myToken.find('=');
        if (equal_pos == std::string::npos || equal_pos == 0 || equal_pos == myToken.size() - 1)
        {
            std::cerr << "incorrect syntax\n";
            return;
        }

        std::string name = myToken.substr(0, equal_pos);
        std::string valStr = myToken.substr(equal_pos + 1);

        try
        {
            uint16_t val = static_cast<uint16_t>(std::stoul(valStr));
            if (val < e_input_last_item)
            {
                monitor_names.emplace_back(name);
                monitor_registers.emplace_back(val);
            }
            else
                std::cerr << val << " is too big (max = " << e_input_last_item << ").\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << "conversion failed: " << e.what() << "\n";
            return;
        }
    }
}

void print_monitor(void)
{
    std::unique_lock lk(monitor_mutex);
    if (g_monitor_enable)
    {
        auto it1 = std::min_element(
            monitored.begin(), monitored.end(),
            [](auto &a, auto &b)
            { return a.second < b.second; });

        uint16_t min = it1->second;

        auto it2 = std::max_element(
            monitored.begin(), monitored.end(),
            [](auto &a, auto &b)
            { return a.second < b.second; });

        uint16_t max = it2->second;
        // Find min-max of monitored registers
        {
            // uint16_t min = *std::min_element(monitor_registers.begin(), monitor_registers.end());
            // uint16_t max = *std::max_element(monitor_registers.begin(), monitor_registers.end());
            updateInputRegister(min, max);
        }

        std::string product;
        for (const auto &element : monitored)
        {
            if (getInputRegScaleFactor(element.second) == 0)
                product = product + element.first + "=" + std::to_string((int16_t)inputRegisters[element.second]) + " ";
            else
            {
                product = product + element.first + "=" + to_string_with_precision((float)((int16_t)inputRegisters[element.second]) / 10.0, 1) + " ";
            }
        }
        printf("\r%s\n", product.c_str());
    }
}

void timer_thread(int ms)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        if (g_monitor_enable)
        {
            print_monitor();
        }
    }
}

void special_function(int key)
{
    if (key == 3)
    {
        g_monitor_enable = !g_monitor_enable;
        printf("Monitor is %s\n", g_monitor_enable ? "ENABLED" : "DISABLED");
    }
    else
    {
        std::cout << "Pressed F" << key + 1 << std::endl;
    }
}

void init_monitor(void)
{
    std::unique_lock lk(monitor_mutex);

    monitored.clear();
    monitored.emplace("COMP", e_compressor);
    monitored.emplace("FAN", e_fan);
    monitored.emplace("LEVEL", e_powerLevel_100);
    monitored.emplace("T3", e_condenser_temp);
    monitored.emplace("T4", e_ambient_temp);
    monitored.emplace("T5", e_discharge_temp);
    monitored.emplace("PWR", e_pwr);

    // #if defined(midea) || defined(haier)
    monitored.emplace("EXV", eev_ro);
    // #endif
    // #ifdef gree
    monitored.emplace("EXV_A", eev_ro);
    monitored.emplace("EXV_B", eev1_ro);

    return;
}

void monitor_clear()
{
    printf("Removing all %zu entries from monitored registers.\n", monitored.size());
    monitored.clear();
}

void new_terminal_init(void)
{
    my_prompt.insertMenuItem(std::string("settings show"), [](std::string)
                             {  updateHoldingRegister(0, e_holding_last_item);
                                updateInputRegister(0,e_input_last_item);
                                show_settings(); });
    my_prompt.insertMenuItem("settings save", [](std::string)
                             { writeRegister(e_execute_command, Command::Save); });
    my_prompt.insertMenuItem("settings write_config", [](std::string x)
                             { 
                                updateHoldingRegister(0, e_holding_last_item);
                                write_settings_to_file(x); });
    my_prompt.insertMenuItem("settings read_config", [](std::string x)
                             { read_registers_from_file(x);
                                writeMultipleRegisters(holdingRegisters, 0, e_holding_last_item); });
    my_prompt.insertMenuItem("settings restore_default", [](std::string)
                             { 
                                writeRegister(e_execute_command, Command::DefaultSettings);
                                writeMultipleRegisters(holdingRegisters, 0, e_holding_last_item); });
    my_prompt.insertMenuItem("system bootsel", [](std::string)
                             { writeRegister(e_execute_command, 3); });
    my_prompt.insertMenuItem("system reset", [](std::string)
                             { writeRegister(e_execute_command, 2); });
    my_prompt.insertMenuItem("system show info", [](std::string)
                             { system_info(); });
    // my_prompt.insertMenuItem("system faults show_all", [](std::string)
    //                            { print_fault_list(); });
    // my_prompt.insertMenuItem("system faults show_active", [](std::string)
    //                            { print_active_faults(true); });

    my_prompt.insertMenuItem("flow test", [](std::string x)
                             { test_flow_value(static_cast<uint16_t>(std::stoul(x))); });

    my_prompt.insertMenuItem("operation show", [](std::string)
                             { printf("Set operation mode : %s\nActual operation mode: %s\n", operationToString(holdingRegisters[e_mode]), operationToString(inputRegisters[e_operation_mode_ro])); });
    my_prompt.insertMenuItem("operation set idle", [](std::string x)
                             { writeRegister(e_mode, Operation::Idle); holdingRegisters[e_mode] = Operation::Idle; });
    my_prompt.insertMenuItem("operation set cool_manual", [](std::string x)
                             { writeRegister(e_mode,1); holdingRegisters[e_mode] = 1; });
    my_prompt.insertMenuItem("operation set heat_manual", [](std::string x)
                             { writeRegister(e_mode,2); holdingRegisters[e_mode] = 2; });
    my_prompt.insertMenuItem("operation set cool_auto", [](std::string x)
                             { writeRegister(e_mode,3); holdingRegisters[e_mode] = 3; });
    my_prompt.insertMenuItem("operation set heat_auto", [](std::string x)
                             { writeRegister(e_mode,4); holdingRegisters[e_mode] = 4; });

    my_prompt.insertMenuItem("modbus input_registers show", [](std::string x)
                             { callback(0, x); });

    my_prompt.insertMenuItem("modbus holding_registers show", [](std::string x)
                             { callback(1, x); });
    my_prompt.insertMenuItem("modbus holding_registers set", [](std::string x)
                             {

                                   Tokens tokens = tokenize(x);

                                   if (tokens.size() < 2)
                                   {
                                       printf("Too few parameters.\n");
                                       return;
                                   }
                                   else if (tokens.size() > 2)
                                   {
                                       printf("Too many parameters.\n");
                                       return;
                                   }
                                   writeRegister(std::stoi(tokens[0]),std::stoi(tokens[1]));
                                   callback(2, x); });
    // my_prompt.insertMenuItem("modbus show_info", [](std::string)
    //                            { printf("Serial settings: %u 8N1\nSlave_Id: %u\n", MODBUS_BAUD, MODBUS_SLAVE_ID); });

    my_prompt.insertMenuItem("control set local_0-10V", [](std::string)
                             { writeRegister(e_control_mode,Control::Local); holdingRegisters[e_control_mode] = Control::Local; });
    my_prompt.insertMenuItem("control set remote_0-100", [](std::string)
                             { writeRegister(e_control_mode,Control::RemoteLevel); holdingRegisters[e_control_mode] = Control::RemoteLevel; });
    my_prompt.insertMenuItem("control set remote_temperature", [](std::string)
                             { writeRegister(e_control_mode,Control::RemoteTemperature); holdingRegisters[e_control_mode] = Control::RemoteTemperature; });

    my_prompt.insertMenuItem("level set", [](std::string x)
                             { holdingRegisters[e_level] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_level, holdingRegisters[e_level]); });
    my_prompt.insertMenuItem("level show", [](std::string x)
                             { printf("Power level set: %u \nPower level actual: %u \n", holdingRegisters[e_level], inputRegisters[e_powerLevel_100]); });
    my_prompt.insertMenuItem("level increment", [](std::string x)
                             { holdingRegisters[e_increment] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_increment, holdingRegisters[e_increment]); });
    my_prompt.insertMenuItem("level decrement", [](std::string x)
                             { holdingRegisters[e_decrement] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_decrement, holdingRegisters[e_decrement]); });

    my_prompt.insertMenuItem("temperature target show", [](std::string)
                             { printf("Temperature static setpoint: %2.1f'C\nTemperature actual setpoint: %2.1f'C\n", holdingRegisters[e_temp_setpoint] / 10.0, inputRegisters[e_temp_setpoint_ro] / 10.0); });
    my_prompt.insertMenuItem("temperature set_mode static", [](std::string)
                             { holdingRegisters[e_curve_active] = 0; writeRegister(e_curve_active, holdingRegisters[e_curve_active]); });
    my_prompt.insertMenuItem("temperature set_mode dynamic", [](std::string)
                             { holdingRegisters[e_curve_active] = 1; writeRegister(e_curve_active, holdingRegisters[e_curve_active]); });
    my_prompt.insertMenuItem("temperature target set", [](std::string x)
                             { holdingRegisters[e_temp_setpoint] = std::clamp(static_cast<int16_t>(10 * std::stof(x)), (int16_t)0, (int16_t)500); writeRegister(e_temp_setpoint, holdingRegisters[e_temp_setpoint]); });
    my_prompt.insertMenuItem("temperature show", [](std::string)
                             { updateHoldingRegister(0, e_holding_last_item); updateInputRegister(0,e_input_last_item); show_temperature(); });
    my_prompt.insertMenuItem("temperature delta_low", [](std::string x)
                             { holdingRegisters[e_low_delta] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_low_delta, holdingRegisters[e_low_delta]); });
    my_prompt.insertMenuItem("temperature delta_high", [](std::string x)
                             { holdingRegisters[e_high_delta] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_high_delta, holdingRegisters[e_high_delta]); });
    my_prompt.insertMenuItem("temperature idle_time", [](std::string x)
                             { holdingRegisters[e_on_off_interval] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_on_off_interval, holdingRegisters[e_on_off_interval]); });
    my_prompt.insertMenuItem("temperature auto_off_delay", [](std::string x)
                             { holdingRegisters[e_off_delay] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_off_delay, holdingRegisters[e_off_delay]); });
    my_prompt.insertMenuItem("temperature dynamic test", [](std::string x)
                             { test_equithermal_curve(static_cast<int>(std::stoi(x))); });
    my_prompt.insertMenuItem("temperature dynamic gain", [](std::string x)
                             { holdingRegisters[e_curve_gain] = static_cast<uint16_t>(100 * std::stof(x)); writeRegister(e_curve_gain, holdingRegisters[e_curve_gain]); });
    my_prompt.insertMenuItem("temperature dynamic offset", [](std::string x)
                             { holdingRegisters[e_curve_offset] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_curve_offset, holdingRegisters[e_curve_offset]); });
    my_prompt.insertMenuItem("temperature dynamic calculate_AB", [](std::string x)
                             { calculate_curve(x); });
    my_prompt.insertMenuItem("temperature dynamic ambient_average_scope", [](std::string x)
                             { holdingRegisters[e_ambient_temp_scope] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_ambient_temp_scope, holdingRegisters[e_ambient_temp_scope]); });;

    my_prompt.insertMenuItem("temperature pid k_p", [](std::string x)
                             { holdingRegisters[e_Kp_factor] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_Kp_factor, holdingRegisters[e_Kp_factor]); });
    my_prompt.insertMenuItem("temperature pid k_i", [](std::string x)
                             { holdingRegisters[e_Ki_factor] = static_cast<uint16_t>(100 * std::stof(x)); writeRegister(e_Ki_factor, holdingRegisters[e_Ki_factor]); });
    my_prompt.insertMenuItem("temperature pid k_d", [](std::string x)
                             { holdingRegisters[e_Kd_factor] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_Kd_factor, holdingRegisters[e_Kd_factor]); });
    my_prompt.insertMenuItem("temperature pid sampling_time", [](std::string x)
                             { holdingRegisters[e_pid_sampling_time] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_pid_sampling_time, holdingRegisters[e_pid_sampling_time]); });
    my_prompt.insertMenuItem("temperature pid hysteresis", [](std::string x)
                             { holdingRegisters[e_pid_hysteresis] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_pid_hysteresis, holdingRegisters[e_pid_hysteresis]); });
    my_prompt.insertMenuItem("temperature pid show", [](std::string x)
                             { show_pid(); });

    my_prompt.insertMenuItem("softstart preheat", [](std::string x)
                             { holdingRegisters[e_preheat_temp] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_preheat_temp, holdingRegisters[e_preheat_temp]); });
    my_prompt.insertMenuItem("softstart precool", [](std::string x)
                             { holdingRegisters[e_precool_temp] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_precool_temp, holdingRegisters[e_precool_temp]); });
    my_prompt.insertMenuItem("softstart hysteresis", [](std::string x)
                             { holdingRegisters[e_pre_hysteresis] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_pre_hysteresis, holdingRegisters[e_pre_hysteresis]); });
    my_prompt.insertMenuItem("softstart show", [](std::string x)
                             { show_softstart(); });

    my_prompt.insertMenuItem("oil low_freq set", [](std::string x)
                             { holdingRegisters[e_oil_recovery_low_freq] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_oil_recovery_low_freq, holdingRegisters[e_oil_recovery_low_freq]); });
    my_prompt.insertMenuItem("oil interval set", [](std::string x)
                             { holdingRegisters[e_oil_recovery_low_time] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_oil_recovery_low_time, holdingRegisters[e_oil_recovery_low_time]); });
    my_prompt.insertMenuItem("oil target_frequency set", [](std::string x)
                             { holdingRegisters[e_oil_recovery_restore_freq] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_oil_recovery_restore_freq, holdingRegisters[e_oil_recovery_restore_freq]); });
    my_prompt.insertMenuItem("oil show", [](std::string x)
                             { show_oil(); });

    my_prompt.insertMenuItem("misc relay alarm function", [](std::string x)
                             { holdingRegisters[e_alarm_relay_function] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_alarm_relay_function, holdingRegisters[e_alarm_relay_function]); });
    my_prompt.insertMenuItem("misc relay alarm polarity", [](std::string x)
                             { updateHoldingRegister(0, e_holding_last_item);
                                set_relay_polarity(0, x);
                                writeRegister(e_relay_polarity, holdingRegisters[e_relay_polarity]); });
    my_prompt.insertMenuItem("misc relay defrost function", [](std::string x)
                             { holdingRegisters[e_defrost_relay_function] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_defrost_relay_function, holdingRegisters[e_defrost_relay_function]); });
    my_prompt.insertMenuItem("misc relay defrost polarity", [](std::string x)
                             { updateHoldingRegister(0, e_holding_last_item);
                                set_relay_polarity(1, x);
                                writeRegister(e_relay_polarity, holdingRegisters[e_relay_polarity]); });
    my_prompt.insertMenuItem("misc relay show", [](std::string x)
                             { show_relay_functions(); });
    my_prompt.insertMenuItem("misc input_function heat", [](std::string x)
                             { holdingRegisters[e_heat_input_function] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_heat_input_function, holdingRegisters[e_heat_input_function]); });
    my_prompt.insertMenuItem("misc input_function cool", [](std::string x)
                             { holdingRegisters[e_cool_input_function] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_cool_input_function, holdingRegisters[e_cool_input_function]); });
    my_prompt.insertMenuItem("misc input_function show", [](std::string x)
                             { show_input_functions(); });
    my_prompt.insertMenuItem("misc monitor add", [](std::string x)
                             { monitor_add(x); });
    my_prompt.insertMenuItem("misc monitor remove", [](std::string x)
                             { monitor_remove(x); });
    my_prompt.insertMenuItem("misc monitor clear", [](std::string x)
                             { monitor_clear(); });
    my_prompt.insertMenuItem("misc monitor show", [](std::string x)
                             { monitor_show(); });
    my_prompt.insertMenuItem("misc monitor default", [](std::string x)
                             { init_monitor(); });

    my_prompt.insertMenuItem("misc protections t2_low", [](std::string x)
                             { holdingRegisters[e_t2_low_alarm_value] = static_cast<int16_t>(10 * std::stof(x)); writeRegister(e_t2_low_alarm_value, holdingRegisters[e_t2_low_alarm_value]); });
    my_prompt.insertMenuItem("misc protections flow_low", [](std::string x)
                             { holdingRegisters[e_minimal_flow] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_minimal_flow, holdingRegisters[e_minimal_flow]); });

    my_prompt.insertMenuItem("defrost start", [](std::string x)
                             { holdingRegisters[e_execute_command] = Command::StartDefrost; writeRegister(e_execute_command, holdingRegisters[e_execute_command]); });
    my_prompt.insertMenuItem("defrost stop", [](std::string x)
                             { holdingRegisters[e_execute_command] = Command::StopDefrost; writeRegister(e_execute_command, holdingRegisters[e_execute_command]); });
    my_prompt.insertMenuItem("defrost show", [](std::string x)
                             { show_defrost(); });
    my_prompt.insertMenuItem("defrost temperature_target", [](std::string x)
                             { holdingRegisters[e_defrost_end_t3_target] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_defrost_end_t3_target, holdingRegisters[e_defrost_end_t3_target]); });
    my_prompt.insertMenuItem("defrost compressor_max_speed", [](std::string x)
                             { holdingRegisters[e_defrost_max_frequency] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_defrost_max_frequency, holdingRegisters[e_defrost_max_frequency]); });
    my_prompt.insertMenuItem("defrost duration_max", [](std::string x)
                             { holdingRegisters[e_defrost_max_duration] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_defrost_max_duration, holdingRegisters[e_defrost_max_duration]); });
    my_prompt.insertMenuItem("defrost max_odu_delta", [](std::string x)
                             { holdingRegisters[e_defrost_max_odu_delta] = static_cast<uint16_t>(10 * std::stoul(x)); writeRegister(e_defrost_max_odu_delta, holdingRegisters[e_defrost_max_odu_delta]); });
    my_prompt.insertMenuItem("defrost interval_min", [](std::string x)
                             { holdingRegisters[e_defrost_min_interval] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_defrost_min_interval, holdingRegisters[e_defrost_min_interval]); });
    my_prompt.insertMenuItem("defrost drop_max_t3", [](std::string x)
                             { holdingRegisters[e_defrost_max_t3_drop] = static_cast<uint16_t>(10 * std::stoul(x)); writeRegister(e_defrost_max_t3_drop, holdingRegisters[e_defrost_max_t3_drop]); });

    my_prompt.insertMenuItem("dhw show", [](std::string x)
                             { holdingRegisters[e_dhw_level] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_dhw_level, holdingRegisters[e_dhw_level]); });
    my_prompt.insertMenuItem("dhw level", [](std::string x)
                             { holdingRegisters[e_dhw_level] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_dhw_level, holdingRegisters[e_dhw_level]); });
    my_prompt.insertMenuItem("dhw temperature", [](std::string x)
                             { holdingRegisters[e_dhw_target_temperature] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_dhw_target_temperature, holdingRegisters[e_dhw_target_temperature]); });
    my_prompt.insertMenuItem("dhw mode legacy", [](std::string x)
                             { holdingRegisters[e_dhw_mode] = DHW::Legacy; writeRegister(e_dhw_mode, holdingRegisters[e_dhw_mode]); });
    my_prompt.insertMenuItem("dhw mode const_level", [](std::string x)
                             { holdingRegisters[e_dhw_mode] = DHW::FixedLevel;writeRegister(e_dhw_mode, holdingRegisters[e_dhw_mode]); });
    my_prompt.insertMenuItem("dhw mode const_temp", [](std::string x)
                             { holdingRegisters[e_dhw_mode] = DHW::FixedTemp; writeRegister(e_dhw_mode, holdingRegisters[e_dhw_mode]); });

    my_prompt.insertMenuItem("bivalent temperature_0", [](std::string x)
                             { holdingRegisters[e_bivalent0_temp] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_bivalent0_temp, holdingRegisters[e_bivalent0_temp]); });
    my_prompt.insertMenuItem("bivalent hystesis_0", [](std::string x)
                             { holdingRegisters[e_bivalent0_hysteresis] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_bivalent0_hysteresis, holdingRegisters[e_bivalent0_hysteresis]); });
    my_prompt.insertMenuItem("bivalent temperature_1", [](std::string x)
                             { holdingRegisters[e_bivalent1_temp] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_bivalent1_temp, holdingRegisters[e_bivalent1_temp]); });
    my_prompt.insertMenuItem("bivalent hystesis_1", [](std::string x)
                             { holdingRegisters[e_bivalent1_hysteresis] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_bivalent1_hysteresis, holdingRegisters[e_bivalent1_hysteresis]); });
    my_prompt.insertMenuItem("bivalent show", [](std::string x)
                             { show_bivalent(); });

#if defined(midea) || defined(gree) || defined(generic)
    my_prompt.insertMenuItem("developer odu compressor", [](std::string x)
                             { holdingRegisters[e_override_compressor] = static_cast<uint16_t>(std::stoul(x)); });
#endif
#if defined(midea)
    my_prompt.insertMenuItem("developer odu fan", [](std::string x)
                             { holdingRegisters[e_odu_fan_override] = static_cast<uint16_t>(std::stoul(x)); });
#endif
}

int updateInputRegister(uint16_t reg)
{
    std::unique_lock lk(modbus_mutex);
    // Connect to the Modbus server
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    int rc = modbus_read_input_registers(ctx, reg, 1, &inputRegisters[reg]);
    if (rc == -1)
    {
        fprintf(stderr, "Read failed: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        return -1;
    }

    // Close the connection
    modbus_close(ctx);

    return 0;
}

int updateInputRegister(uint16_t from, uint16_t to)
{
    std::unique_lock lk(modbus_mutex);
    // Connect to the Modbus server
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    int rc = modbus_read_input_registers(ctx, from, to - from + 1, &inputRegisters[from]);
    if (rc == -1)
    {
        fprintf(stderr, "Read failed: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        return -1;
    }

    // Close the connection
    modbus_close(ctx);

    return 0;
}

int updateHoldingRegister(uint16_t reg)
{
    std::unique_lock lk(modbus_mutex);
    // Connect to the Modbus server
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    int rc = modbus_read_registers(ctx, reg, 1, &holdingRegisters[reg]);
    if (rc == -1)
    {
        fprintf(stderr, "Read failed: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        return -1;
    }

    // Close the connection
    modbus_close(ctx);

    return 0;
}

int updateHoldingRegister(uint16_t from, uint16_t to)
{
    std::unique_lock lk(modbus_mutex);
    // Connect to the Modbus server
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    int rc = modbus_read_registers(ctx, from, to - from + 1, &holdingRegisters[from]);
    if (rc == -1)
    {
        fprintf(stderr, "Read failed: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        return -1;
    }

    // Close the connection
    modbus_close(ctx);

    return 0;
}

int writeRegister(uint16_t reg, uint16_t value)
{
    std::unique_lock lk(modbus_mutex);
    // Connect to the Modbus server
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    int rc = modbus_write_register(ctx, reg, value);
    if (rc == -1)
    {
        fprintf(stderr, "Write failed: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        return -1;
    }
    modbus_close(ctx);

    return 0;
}

int writeMultipleRegisters(uint16_t *registers, uint16_t addr, uint16_t count)
{
    std::unique_lock lk(modbus_mutex);
    // Connect to the Modbus server
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    int rc = modbus_write_registers(ctx, addr, count, registers);
    if (rc == -1)
    {
        fprintf(stderr, "Write failed: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        return -1;
    }
    modbus_close(ctx);

    return 0;
}

void write_settings_to_file(const std::filesystem::path &file)
{
    std::ofstream ofs(file);

    if (!ofs)
    {
        fprintf(stderr, "Failed to open file: %s :(\n", file.string().c_str());
        return;
    }

    if (!holdingRegisters)
    {
        throw std::invalid_argument("holdingRegisters is nullptr.");
    }

    const int comment_column = 24; // Column where comments should start
    for (size_t i = 0; i < e_holding_last_item; ++i)
    {
        // Build the left part (reg[N] = value)
        std::ostringstream oss;
        oss << "reg[" << i << "] = " << holdingRegisters[i];

        std::string left_part = oss.str();
        int padding = comment_column - static_cast<int>(left_part.size());
        if (padding < 1)
            padding = 1; // Always at least 1 space before #

        ofs << left_part
            << std::string(padding, ' ') // pad with spaces
            << "# " << holdingRegToStr(i)
            << '\n';
    }

    printf("Successfully written settings to the config file \"%s\" :-)\n", file.string().c_str());
}

void read_registers_from_file(const std::filesystem::path &file)
{
    std::ifstream ifs(file);
    if (!ifs)
    {
        fprintf(stderr, "Failed to open file: %s :(\n", file.string().c_str());
        return;
    }

    std::string line;
    std::regex line_pattern(R"(reg\[(\d+)\]\s*=\s*(\d+))");

    while (std::getline(ifs, line))
    {
        std::smatch match;
        if (std::regex_search(line, match, line_pattern))
        {
            size_t index = std::stoul(match[1].str());
            unsigned long value = std::stoul(match[2].str());

            if (value > std::numeric_limits<uint16_t>::max())
            {
                throw std::out_of_range("Value out of range for uint16_t at reg[" + std::to_string(index) + "]");
            }

            holdingRegisters[index] = static_cast<uint16_t>(value);
        }
    }
    printf("Successfully read config file \"%s\" :-)\n", file.string().c_str());
}

int main(int argc, char **argv)
{
    uint16_t tcp_port{502};
    const char *ip_address{nullptr};
    const char *char_dev{nullptr};
    int opt;
    bool given_ip{false};
    bool given_chardev{false};
    while ((opt = getopt(argc, argv, "i:p:d:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            ip_address = optarg;
            given_ip = true;
            break;
        case 'd':
            char_dev = optarg;
            given_chardev = true;
            break;

        case 'p':
        {
            unsigned long value = std::stoul(optarg);
            if (value > std::numeric_limits<uint16_t>::max())
            {
                throw std::out_of_range("TCP port value should not overflow the range of uint16_t");
            }
            else
            {
                tcp_port = static_cast<uint16_t>(std::stoul(optarg));
            }
            break;
        }

        default:
            fprintf(stderr, "Usage: %s -i ip_address\n", argv[0]);
            fprintf(stderr, "Usage: %s -d /dev/ttyUSB<N>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (given_ip && given_chardev)
    {
        fprintf(stderr, "Pick only one of variants, RTU (-d), or TCP (-i)\n");
        exit(EXIT_FAILURE);
    }

    if (!ip_address && !char_dev)
    {
        fprintf(stderr, "Usage: %s -i ip_address\n", argv[0]);
        fprintf(stderr, "Usage: %s -d /dev/ttyUSB<N>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Start timer thread for some periodic events
    std::thread timerThread(timer_thread, 1000);
    timerThread.detach();

    if (given_ip)
    {
        printf("IP Address: %s port : %hu\n", ip_address, tcp_port);
        ctx = modbus_new_tcp(ip_address, tcp_port);
    }
    else if (given_chardev)
    {
        printf("Serial device : %s\n", char_dev);
        ctx = modbus_new_rtu(char_dev, 9600, 'N', 8, 1);
    }
    else
    {
        throw std::runtime_error("No modbus variant specified");
    }
    if (!ctx)
    {
        fprintf(stderr, "unable to connetc\n");
        std::abort();
    }

    modbus_set_slave(ctx, 53);
    init_monitor();

    // Because Libmodbus API has changed after 3.1.2 version
#ifdef LIBMODBUS_PRE_312
    const timeval response_timeout = {2, 0};
    modbus_set_response_timeout(ctx, &response_timeout);
#else
    modbus_set_response_timeout(ctx, 2, 0);
#endif

    holdingRegisters = new uint16_t[e_holding_last_item];

    memset(holdingRegisters, 0, sizeof(uint16_t) * e_holding_last_item);

    for (int i = static_cast<int>(FnKey::F1); i < static_cast<int>(FnKey::F12) + 1; i++)
    {
        my_prompt.attachFnKeyCallback(static_cast<FnKey>(i), [i]()
                                      { special_function(i); });
    }

    if (updateHoldingRegister(0, e_holding_last_item) == -1 || updateInputRegister(0, e_input_last_item) == -1)
    {
        fprintf(stderr, "unable to connetc\n");
        std::abort();
    }
    new_terminal_init();
    my_prompt.Run();

    delete[] holdingRegisters;
    return 0;
}
