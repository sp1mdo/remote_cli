#include <stdio.h>
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <set>
#include <cstring>
#include <map>
#include "modbus_registers.h"
#include "struct.h"
#include "cli_commands.hpp"
#include <modbus/modbus.h>
#include <getopt.h>

#include "../../cli/src/Prompt.hpp"

int updateHoldingRegister(uint16_t from, uint16_t to);
int updateHoldingRegister(uint16_t reg);

int updateInputRegister(uint16_t from, uint16_t to);
int updateInputRegister(uint16_t reg);

int writeRegister(uint16_t reg, uint16_t value);

Prompt my_prompt("AHU_2040");

uint16_t *holdingRegisters;
uint16_t inputRegisters[e_input_last_item];

modbus_t *ctx;

void special_function(int key)
{
    std::cout << "Pressed F" << key + 1 << std::endl;
}

void new_terminal_init(void)
{
    my_prompt.insertMapElement("settings show", [](std::string)
                               {  updateHoldingRegister(0, e_holding_last_item); updateInputRegister(0,e_input_last_item); show_settings(); });
    my_prompt.insertMapElement("settings save", [](std::string)
                               { return; });

    // my_prompt.insertMapElement("settings read", [](std::string)
    //                            { read_settings(); });
    // my_prompt.insertMapElement("settings restore_default", [](std::string)
    //                            { restore_default_settings(); });
    // my_prompt.insertMapElement("system bootsel", [](std::string)
    //                            { bootsel_mode(); });
    // my_prompt.insertMapElement("system reset", [](std::string)
    //                            { reset_board(); });
    my_prompt.insertMapElement("system show info", [](std::string)
                               { system_info(); });
    // my_prompt.insertMapElement("system faults show_all", [](std::string)
    //                            { print_fault_list(); });
    // my_prompt.insertMapElement("system faults show_active", [](std::string)
    //                            { print_active_faults(true); });

    my_prompt.insertMapElement("flow test", [](std::string x)
                               { test_flow_value(static_cast<uint16_t>(std::stoul(x))); });

    // my_prompt.insertMapElement("operation show", [](std::string)
    //                            { printf("Set operation mode : %s\nActual operation mode: %s\n", operationToString(holdingRegisters[e_mode]), operationToString(g_operationMode)); });
    my_prompt.insertMapElement("operation set idle", [](std::string x)
                               { writeRegister(e_mode,IDLE_MODE); holdingRegisters[e_mode] = IDLE_MODE; });
    my_prompt.insertMapElement("operation set cool_manual", [](std::string x)
                               { writeRegister(e_mode,1); holdingRegisters[e_mode] = 1; });
    my_prompt.insertMapElement("operation set heat_manual", [](std::string x)
                               { writeRegister(e_mode,2); holdingRegisters[e_mode] = 2; });
    my_prompt.insertMapElement("operation set cool_auto", [](std::string x)
                               { writeRegister(e_mode,3); holdingRegisters[e_mode] = 3; });
    my_prompt.insertMapElement("operation set heat_auto", [](std::string x)
                               { writeRegister(e_mode,4); holdingRegisters[e_mode] = 4; });

    my_prompt.insertMapElement("modbus input_registers show", [](std::string x)
                               {updateInputRegister(0,e_input_last_item);callback(0, x); });

    my_prompt.insertMapElement("modbus holding_registers show", [](std::string x)
                               {updateHoldingRegister(0, e_holding_last_item); callback(1, x); });
    my_prompt.insertMapElement("modbus holding_registers set", [](std::string x)
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
                                   callback(2, x);
                               });
    // my_prompt.insertMapElement("modbus show_info", [](std::string)
    //                            { printf("Serial settings: %u 8N1\nSlave_Id: %u\n", MODBUS_BAUD, MODBUS_SLAVE_ID); });

    my_prompt.insertMapElement("control set local_0-10V", [](std::string)
                               { writeRegister(e_control_mode,e_ctrl_mode_local); holdingRegisters[e_control_mode] = e_ctrl_mode_local; });
    my_prompt.insertMapElement("control set remote_0-100", [](std::string)
                               { writeRegister(e_control_mode,e_ctrl_mode_remote_100); holdingRegisters[e_control_mode] = e_ctrl_mode_remote_100; });
    my_prompt.insertMapElement("control set remote_temperature", [](std::string)
                               { writeRegister(e_control_mode,e_ctrl_mode_remote_temp); holdingRegisters[e_control_mode] = e_ctrl_mode_remote_temp; });

    my_prompt.insertMapElement("level set", [](std::string x)
                               { holdingRegisters[e_level] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_level, holdingRegisters[e_level]); });
    my_prompt.insertMapElement("level show", [](std::string x)
                               { printf("Power level set: %u \nPower level actual: %u \n", holdingRegisters[e_level], inputRegisters[e_powerLevel_100]); });
    my_prompt.insertMapElement("level increment", [](std::string x)
                               { holdingRegisters[e_increment] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_increment, holdingRegisters[e_increment]); });
    my_prompt.insertMapElement("level decrement", [](std::string x)
                               { holdingRegisters[e_decrement] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_decrement, holdingRegisters[e_decrement]); });

    my_prompt.insertMapElement("temperature static set_target", [](std::string x)
                               { holdingRegisters[e_temp_setpoint] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_temp_setpoint, holdingRegisters[e_temp_setpoint]); });
    my_prompt.insertMapElement("temperature target show", [](std::string)
                               { printf("Temperature static setpoint: %2.1f'C\nTemperature actual setpoint: %2.1f'C\n", holdingRegisters[e_temp_setpoint] / 10.0, inputRegisters[e_temp_setpoint_ro] / 10.0); });
    my_prompt.insertMapElement("temperature set_mode static", [](std::string)
                               { holdingRegisters[e_curve_active] = 0; writeRegister(e_curve_active, holdingRegisters[e_curve_active]); });
    my_prompt.insertMapElement("temperature set_mode dynamic", [](std::string)
                               { holdingRegisters[e_curve_active] = 1; writeRegister(e_curve_active, holdingRegisters[e_curve_active]); });
    my_prompt.insertMapElement("temperature set_target", [](std::string x)
                               { holdingRegisters[e_temp_setpoint] = std::clamp(static_cast<int16_t>(10 * std::stof(x)), (int16_t)0, (int16_t)500); writeRegister(e_temp_setpoint, holdingRegisters[e_temp_setpoint]); });
    my_prompt.insertMapElement("temperature show", [](std::string)
                               { updateHoldingRegister(0, e_holding_last_item); updateInputRegister(0,e_input_last_item); show_temperature(); });
    my_prompt.insertMapElement("temperature delta_low set", [](std::string x)
                               { holdingRegisters[e_low_delta] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_low_delta, holdingRegisters[e_low_delta]); });
    my_prompt.insertMapElement("temperature delta_high set", [](std::string x)
                               { holdingRegisters[e_high_delta] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_high_delta, holdingRegisters[e_high_delta]); });
    my_prompt.insertMapElement("temperature idle_time set", [](std::string x)
                               { holdingRegisters[e_on_off_interval] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_on_off_interval, holdingRegisters[e_on_off_interval]); });
    my_prompt.insertMapElement("temperature dynamic test", [](std::string x)
                               { test_equithermal_curve(static_cast<int>(std::stoi(x))); });
    my_prompt.insertMapElement("temperature dynamic set_gain", [](std::string x)
                               { holdingRegisters[e_curve_gain] = static_cast<uint16_t>(100 * std::stof(x)); writeRegister(e_curve_gain, holdingRegisters[e_curve_gain]); });
    my_prompt.insertMapElement("temperature dynamic set_offset", [](std::string x)
                               { holdingRegisters[e_curve_offset] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_curve_offset, holdingRegisters[e_curve_offset]); });
    my_prompt.insertMapElement("temperature pid k_p set", [](std::string x)
                               { holdingRegisters[e_Kp_factor] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_Kp_factor, holdingRegisters[e_Kp_factor]); });
    my_prompt.insertMapElement("temperature pid k_i set", [](std::string x)
                               { holdingRegisters[e_Ki_factor] = static_cast<uint16_t>(100 * std::stof(x)); writeRegister(e_Ki_factor, holdingRegisters[e_Ki_factor]); });
    my_prompt.insertMapElement("temperature pid k_d set", [](std::string x)
                               { holdingRegisters[e_Kd_factor] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_Kd_factor, holdingRegisters[e_Kd_factor]); });
    my_prompt.insertMapElement("temperature pid sampling_time set", [](std::string x)
                               { holdingRegisters[e_pid_sampling_time] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_pid_sampling_time, holdingRegisters[e_pid_sampling_time]); });
    my_prompt.insertMapElement("temperature pid hysteresis set", [](std::string x)
                               { holdingRegisters[e_pid_hysteresis] = static_cast<uint16_t>(10 * std::stof(x)); writeRegister(e_pid_hysteresis, holdingRegisters[e_pid_hysteresis]); });
    my_prompt.insertMapElement("temperature pid show", [](std::string x)
                               { show_pid(); });

    my_prompt.insertMapElement("oil low_freq set", [](std::string x)
                               { holdingRegisters[e_oil_recovery_low_freq] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_oil_recovery_low_freq, holdingRegisters[e_oil_recovery_low_freq]); });
    my_prompt.insertMapElement("oil interval set", [](std::string x)
                               { holdingRegisters[e_oil_recovery_low_time] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_oil_recovery_low_time, holdingRegisters[e_oil_recovery_low_time]); });
    my_prompt.insertMapElement("oil target_frequency set", [](std::string x)
                               { holdingRegisters[e_oil_recovery_restore_freq] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_oil_recovery_restore_freq, holdingRegisters[e_oil_recovery_restore_freq]); });
    my_prompt.insertMapElement("oil show", [](std::string x)
                               { show_oil(); });

    my_prompt.insertMapElement("misc relay_function alarm set", [](std::string x)
                               { holdingRegisters[e_alarm_relay_function] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_alarm_relay_function, holdingRegisters[e_alarm_relay_function]); });
    my_prompt.insertMapElement("misc relay_function defrost set", [](std::string x)
                               { holdingRegisters[e_defrost_relay_function] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_defrost_relay_function, holdingRegisters[e_defrost_relay_function]); });
    my_prompt.insertMapElement("misc relay_function show", [](std::string x)
                               { show_relay_functions(); });
    my_prompt.insertMapElement("misc input_function heat set", [](std::string x)
                               { holdingRegisters[e_heat_input_function] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_heat_input_function, holdingRegisters[e_heat_input_function]); });
    my_prompt.insertMapElement("misc input_function cool set", [](std::string x)
                               { holdingRegisters[e_cool_input_function] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_cool_input_function, holdingRegisters[e_cool_input_function]); });
    my_prompt.insertMapElement("misc input_function show", [](std::string x)
                               { show_input_functions(); });
    // my_prompt.insertMapElement("misc monitor set", [](std::string x)
    //                            { set_monitor(x); });
    // my_prompt.insertMapElement("misc monitor show", [](std::string x)
    //                            { show_monitor(); });
    // my_prompt.insertMapElement("misc monitor restore_default", [](std::string x)
    //                            { init_monitor(); });

    my_prompt.insertMapElement("hot_water show", [](std::string x)
                               { holdingRegisters[e_hot_water_level] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_hot_water_level, holdingRegisters[e_hot_water_level]); });
    my_prompt.insertMapElement("hot_water level", [](std::string x)
                               { holdingRegisters[e_hot_water_level] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_hot_water_level, holdingRegisters[e_hot_water_level]); });
    my_prompt.insertMapElement("hot_water temperature", [](std::string x)
                               { holdingRegisters[e_hotwater_target_temperature] = static_cast<uint16_t>(std::stoul(x)); writeRegister(e_hotwater_target_temperature, holdingRegisters[e_hotwater_target_temperature]); });
    my_prompt.insertMapElement("hot_water mode odu_ori", [](std::string x)
                               { holdingRegisters[e_hotwater_mode] = cwu_odu_ori; writeRegister(e_hotwater_mode, holdingRegisters[e_hotwater_mode]); });
    my_prompt.insertMapElement("hot_water mode const_level", [](std::string x)
                               { holdingRegisters[e_hotwater_mode] = cwu_fixed_comp;writeRegister(e_hotwater_mode, holdingRegisters[e_hotwater_mode]); });
    my_prompt.insertMapElement("hot_water mode const_temp", [](std::string x)
                               { holdingRegisters[e_hotwater_mode] = cwu_fixed_temp; writeRegister(e_hotwater_mode, holdingRegisters[e_hotwater_mode]); });

#ifdef WIRELESS
    my_prompt.insertMapElement("wifi ssid set", [](std::string x)
                               { wifi_set_ssid(x); });
    my_prompt.insertMapElement("wifi password set", [](std::string x)
                               { wifi_set_password(x); });
    my_prompt.insertMapElement("wifi show status", [](std::string x)
                               { show_networking_info(); });
    // my_prompt.insertMapElement("wifi scan", [](std::string x)
    //                            { scan_wifi_networks(); });
#endif

#ifdef DEVELOPER

#if defined(midea) || defined(gree) || defined(generic)
    my_prompt.insertMapElement("developer odu compressor", [](std::string x)
                               { holdingRegisters[e_override_compressor] = static_cast<uint16_t>(std::stoul(x)); });
#endif
#if defined(midea)
    my_prompt.insertMapElement("developer odu fan", [](std::string x)
                               { holdingRegisters[e_odu_fan_override] = static_cast<uint16_t>(std::stoul(x)); });
#endif
    // my_prompt.insertMapElement("expert odu eev", [](std::string x)
    //                            { holdingRegisters[e_pid_sampling_time] = static_cast<uint16_t>(std::stoul(x)); });

    my_prompt.insertMapElement("developer byte_override", [](std::string x)
                               { byte_override(x); });
#endif

    my_prompt.insertMapElement("quit", [](std::string x)
                               { exit(0); });

    my_prompt.updateAuxMenu("");
}

int updateInputRegister(uint16_t reg)
{
    // Connect to the Modbus server
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    uint32_t response_timeout_sec = 2;
    uint32_t response_timeout_usec = 0;
    modbus_set_response_timeout(ctx, response_timeout_sec, response_timeout_usec);

    int rc = modbus_read_input_registers(ctx, reg, 1, &inputRegisters[reg]);
    if (rc == -1)
    {
        fprintf(stderr, "Read failed: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        return -1;
    }

    // Close the connection and free the context
    modbus_close(ctx);

    return 0;
}

int updateInputRegister(uint16_t from, uint16_t to)
{
    // Connect to the Modbus server
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    uint32_t response_timeout_sec = 2;
    uint32_t response_timeout_usec = 0;
    modbus_set_response_timeout(ctx, response_timeout_sec, response_timeout_usec);

    int rc = modbus_read_input_registers(ctx, from, to - from, &inputRegisters[from]);
    if (rc == -1)
    {
        fprintf(stderr, "Read failed: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        return -1;
    }

    // Close the connection and free the context
    modbus_close(ctx);

    return 0;
}

int updateHoldingRegister(uint16_t reg)
{
    // Connect to the Modbus server
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    uint32_t response_timeout_sec = 2;
    uint32_t response_timeout_usec = 0;
    modbus_set_response_timeout(ctx, response_timeout_sec, response_timeout_usec);

    int rc = modbus_read_registers(ctx, reg, 1, &holdingRegisters[reg]);
    if (rc == -1)
    {
        fprintf(stderr, "Read failed: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        return -1;
    }

    // Close the connection and free the context
    modbus_close(ctx);

    return 0;
}

int updateHoldingRegister(uint16_t from, uint16_t to)
{
    // Connect to the Modbus server
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    uint32_t response_timeout_sec = 2;
    uint32_t response_timeout_usec = 0;
    modbus_set_response_timeout(ctx, response_timeout_sec, response_timeout_usec);

    int rc = modbus_read_registers(ctx, from, to - from, &holdingRegisters[from]);
    if (rc == -1)
    {
        fprintf(stderr, "Read failed: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        return -1;
    }

    // Close the connection and free the context
    modbus_close(ctx);

    return 0;
}

int writeRegister(uint16_t reg, uint16_t value)
{
    // Connect to the Modbus server
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    uint32_t response_timeout_sec = 2;
    uint32_t response_timeout_usec = 0;
    modbus_set_response_timeout(ctx, response_timeout_sec, response_timeout_usec);

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

int main(int argc, char **argv)
{
    uint16_t tcp_port = 502;
    char adress_port[25];
    char ip_address[17] = {0};
    uint8_t addr[4];
    int opt;
    while ((opt = getopt(argc, argv, "i:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            strncpy(adress_port, optarg, sizeof(adress_port));
            break;

        default:
            fprintf(stderr, "Usage: %s -i ip_address\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    sscanf(adress_port, "%hhu.%hhu.%hhu.%hhu:%hu", &addr[0], &addr[1], &addr[2], &addr[3], &tcp_port);

    sprintf(ip_address, "%hhu.%hhu.%hhu.%hhu", addr[0], addr[1], addr[2], addr[3]);
    printf("IP Address: %s port : %hu\n", ip_address, tcp_port);

    ctx = modbus_new_tcp(ip_address, tcp_port);
    modbus_set_slave(ctx, 53);

    holdingRegisters = (uint16_t *)malloc(sizeof(uint16_t) * e_holding_last_item);
    memset(holdingRegisters, 0, sizeof(uint16_t) * e_holding_last_item);

    for (int i = static_cast<int>(FnKey::F1); i < static_cast<int>(FnKey::F12) + 1; i++)
    {
        my_prompt.attachFnKeyCallback(static_cast<FnKey>(i), std::bind(special_function, i));
    }

    new_terminal_init();
    my_prompt.spin_loop();

    return 0;
}
