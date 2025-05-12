#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <vector>
#include <set>
#include <sstream>
#include <iomanip>

#include "cli_commands.hpp"
#include "struct.h"
#include "modbus_registers.h"
#if defined PICO_ON_DEVICE
#include "pico/unique_id.h"
#endif

using Tokens = std::vector<std::string>;
uint32_t git_hash = 0;

class LookupTableVector
{
private:
    std::vector<std::pair<int32_t, int32_t>> table;

public:
    explicit LookupTableVector(std::vector<std::pair<int32_t, int32_t>> input)
        : table(std::move(input))
    {
    }

    int32_t get(int32_t key) const
    {
        // Clamp low
        if (key <= table.front().first)
        {
            return table.front().second;
        }

        // Clamp high
        if (key >= table.back().first)
        {
            return table.back().second;
        }

        // Exact match or interpolate
        for (std::size_t i = 1; i < table.size(); ++i)
        {
            if (key < table[i].first)
            {
                return interpolate(
                    key,
                    table[i - 1].first, table[i - 1].second,
                    table[i].first, table[i].second);
            }
        }

        // Fallback (shouldn’t be hit due to clamping)
        return table.back().second;
    }

private:
    static int32_t interpolate(int32_t x, int32_t x0, int32_t y0, int32_t x1, int32_t y1)
    {
        float ratio = static_cast<float>(x - x0) / (x1 - x0);
        return static_cast<int32_t>(y0 + ratio * (y1 - y0));
    }
};

void holding_registers_show_all(void)
{
    for (size_t reg = 0; reg < e_holding_last_item; reg++)
    {
        printf("register[%zu] = %d \t[ %s ]\n", reg, (int16_t)holdingRegisters[reg], holdingRegToStr(reg));
    }
}

std::set<uint16_t> registers_to_show(std::vector<std::string> &tokens, size_t max)
{
    std::set<uint16_t> registers;
    uint16_t num1 = 0, num2 = 0;
    for (auto &token : tokens)
    {
        int count = -1;
        count = sscanf(token.c_str(), "%hu-%hu", &num1, &num2);

        if (num2 > max) // check against malicious input
        {
            printf("Number %u (or %d) is out of available register range\n", num2, (int16_t)num2);
            registers.clear();
            return registers;
        }

        if (num1 > max) // check against malicious input
        {
            printf("Number %u (or %d) is out of available register range\n", num1, (int16_t)num1);
            registers.clear();
            return registers;
        }
        if (0 == count)
        {
            printf("Incorrect syntax.\n");
            return registers;
        }

        if (1 == count)
        {
            registers.insert(num1);
        }

        if (2 == count)
        {
            for (uint16_t i = num1; i <= num2; i++)
            {
                registers.insert(i);
            }
        }
    }

    return registers;
}

// Show registers function
void show_holding_registers(std::vector<std::string> &tokens)
{   
    if (!tokens.empty() && tokens[0] == "all")
    {
        tokens[0] = "0-" + std::to_string(e_holding_last_item-1);
    }

    std::set<uint16_t> registers = registers_to_show(tokens, e_holding_last_item-1);

    if (tokens.size() == 0)
    {
        printf("Example usage : %s 1 2 5-10 15 16\n", __func__);
        return;
    }
    printf("Showing %2zu registers:\n", registers.size());
    for (auto &reg : registers)
    {
        if (reg < e_holding_last_item)
            printf("register[%u] = %u\t(%s)\n", reg, holdingRegisters[reg], holdingRegToStr(reg));
        else
            printf("%u is too big!\n", reg);
    }
}

void show_input_registers(std::vector<std::string> &tokens)
{
    if (!tokens.empty() && tokens[0] == "all")
    {
        tokens[0] = "0-" + std::to_string(e_input_last_item-1);
    }
    
    std::set<uint16_t> registers = registers_to_show(tokens, e_input_last_item);



    if (tokens.size() == 0)
    {
        printf("Example usage : %s 1 2 5-10 15 16\n", __func__);
        return;
    }
    printf("Showing %2zu registers:\n", registers.size());
    for (auto &reg : registers)
    {
        if (reg < e_input_last_item)
            printf("register[%u] = %u\t(%s)\n", reg, inputRegisters[reg], inputRegToStr(reg));
        else
            printf("%u is too big!\n", reg);
    }
}

std::vector<std::string> tokenize(const std::string &input)
{
    std::vector<std::string> retval;
    std::stringstream ss(input);
    std::string token;
    retval.reserve(4);
    while (ss.good())
    {
        ss >> token;
        if (!token.empty())
            retval.emplace_back(std::move(token));
    }
    return retval;
}

// Set register function
void set_register(unsigned int reg_number, int value)
{
    if (reg_number >= e_holding_last_item)
    {
        printf("Register number is too big. Max allowed is %u\n", e_holding_last_item);
        return;
    }
    holdingRegisters[reg_number] = value;
    printf("Setting register %u [%s] to %d\n", reg_number, holdingRegToStr(reg_number), (int16_t)holdingRegisters[reg_number]);
}

void callback(int id, const std::string &str)
{
    if (str.empty())
        return;
    Tokens tokens = tokenize(str);

    switch (id)
    {
    case 0:
        show_input_registers(tokens);
        return;

    case 1:
        show_holding_registers(tokens);
        return;

    case 2:
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
        

        set_register(std::stoi(tokens[0]), std::stoi(tokens[1]));
        return;

    default:
        printf("Received : id=%d arg=[%s]\n ", id, str.c_str());
    }
}

void show_settings()
{
    char tempStr[21];
    switch (holdingRegisters[e_control_mode])
    {
    case e_ctrl_mode_local:
        strncpy(tempStr, "Local 0-10V", sizeof(tempStr) - 1);
        break;

    case e_ctrl_mode_remote_100:
        strncpy(tempStr, "Remote 0-100%", sizeof(tempStr) - 1);
        break;

    case e_ctrl_mode_remote_temp:
        strncpy(tempStr, "Remote temp.", sizeof(tempStr) - 1);
        break;

    default:
        snprintf(tempStr, sizeof(tempStr) - 1, "Unknown [%u]", holdingRegisters[e_control_mode]);
        break;
    }

    printf("Current settings:\n");
    printf("Type of power control                       %s\n", tempStr);
    printf("Type of temperature control                 %s\n", holdingRegisters[e_curve_active] == 0 ? "Static" : "Dynamic");
    show_pid();
    show_temperature();
    printf("T2_low temperature alarm value            %2.1f ['C]\n", (holdingRegisters[e_t2_low_alarm_value] / 10.0));
    printf("Flow_low alarm value                       %u [Hz]\n", (holdingRegisters[e_minimal_flow]));
    show_oil();
    printf("Hot water heating level                    %2u [%%]\n", (holdingRegisters[e_hot_water_level]));

    show_relay_functions();
    show_input_functions();

#ifdef WIRELESS
    printf("Wifi SSID name :                           %s\n", wifi_ssid);
    printf("Wifi PASSWORD  :                           %s\n", wifi_pass);
#endif
    // printf("Multisplit power selector position     (%01X)%u\n", holdingRegisters[e_multisplit_power_option], holdingRegisters[e_multisplit_power_option]);

    // TODO: function parsing power selector to horse-power notation
}

void show_monitor(void)
{
    for (size_t i = 0; i < monitor_names.size(); i++)
    {
        printf("%s = %u (%s)\n", monitor_names[i].c_str(), monitor_registers[i], inputRegToStr(monitor_registers[i]));
    }
}

void show_relay_functions()
{
    printf("ALARM relay function                       %s\n", getRelayModeStr(holdingRegisters[e_alarm_relay_function]).c_str());
    printf("DEFROST relay function                     %s\n", getRelayModeStr(holdingRegisters[e_defrost_relay_function]).c_str());
}

void show_hot_water(void)
{
    const char* prefix = "Hot water";
    printf("%s Mode                               %s\n", prefix, getHotWaterModeStr(inputRegisters[e_hotwater_mode]).c_str());
    printf("%s Target temeprature             %4.1f ['C]\n", prefix, holdingRegisters[e_hotwater_target_temperature] / 10.0);
    printf("%s Target level                   %4u [%%]\n", prefix, holdingRegisters[e_hot_water_level]);
}

void show_temperature(void)
{
    printf("Actual temperature target                %4.1f ['C]\n", inputRegisters[e_temp_setpoint_ro] / 10.0);
    printf("Set temperature target                   %4.1f ['C]\n", holdingRegisters[e_temp_setpoint] / 10.0);
    printf("Upper delta treshold                    +%4.1f ['K]\n", (holdingRegisters[e_high_delta] / 10.0));
    printf("Lower delta treshold                    -%4.1f ['K]\n", (holdingRegisters[e_low_delta] / 10.0));
    printf("Dynamic temperature gain factor          %4.2f\n", (holdingRegisters[e_curve_gain]) / 100.0);
    printf("Dynamic temperature offset value         %4.1f ['K]\n", (((int16_t)holdingRegisters[e_curve_offset]) / 10.0));
    printf("Auto-OFF delay time                      %4u [min]\n", (holdingRegisters[e_off_delay]));
    printf("Minimum OFF -> ON interval               %4u [min]\n", (holdingRegisters[e_on_off_interval]));
    printf("Remaining time until next start          %4u [s]\n", (inputRegisters[e_interval_on_off_remaining]));
    show_hot_water();
}

std::string to_string_with_precision(float value, int precision = 1)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}

int8_t getInputRegScaleFactor(uint16_t reg)
{
    switch (reg)
    {
    case e_discharge_temp:
    case e_condenser_temp:
    case e_ambient_temp:
    case e_indoor_temp:
    case e_evaporator_temp:
    case e_curve_offset_ro:
    case e_water_in_ro:
    case e_refrigerant_in:
    case e_refrigerant_out:
    case e_COP:
    case e_pid_p_component:
    case e_pid_i_component:
    case e_pid_d_component:
        return -1;

    case e_curve_gain_ro:
        return -2;

    default:
        return 0;
    }

    return 0;
}

void system_info(void)
{
    #if defined PICO_ON_DEVICE
    char usbd_serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1]={0};
    pico_get_unique_board_id_string(usbd_serial_str, sizeof(usbd_serial_str));

    printf("Unique ID %s\n", usbd_serial_str);
    printf("Build: %s %s\nGit hash: %08lx \n", __DATE__, __TIME__, git_hash);
    #endif
    printf("Compressor operation range %u-%u [Hz]\n", inputRegisters[e_compressor_min_frequency], inputRegisters[e_compressor_max_frequency]);
}

void show_input_functions()
{
    const char* suffix = " input function                        ";
    printf("HEAT%s%s\n",suffix, getRelayModeStr(holdingRegisters[e_heat_input_function]).c_str());
    printf("COOL%s%s\n",suffix, getRelayModeStr(holdingRegisters[e_cool_input_function]).c_str());
}

void show_pid(void)
{
    const char* prefix = "PID ";
    printf("%sK_p                                  %4.1f\n",prefix, (holdingRegisters[e_Kp_factor] / 10.0));
    printf("%sK_i                                  %4.2f\n",prefix, (holdingRegisters[e_Ki_factor] / 100.0));
    printf("%sK_d                                  %4.1f\n",prefix, (holdingRegisters[e_Kd_factor] / 10.0));
    printf("%s sampling time                       %4u [s]\n",prefix, (holdingRegisters[e_pid_sampling_time]));
    printf("%shysteresis                           %4.1f ['K]\n",prefix, (holdingRegisters[e_pid_hysteresis] / 10.0));
    // TODO print PID components
}

void show_oil(void)
{
    const char* prefix = "Oil recovery ";
    printf("%slower comp limit            %4u [Hz]\n",prefix, (holdingRegisters[e_oil_recovery_low_freq]));
    printf("%stime below limit            %4u [min]\n",prefix, (holdingRegisters[e_oil_recovery_low_time]));
    printf("%sLVL restore.                %4u [Hz]\n",prefix, (holdingRegisters[e_oil_recovery_restore_freq]));
    printf("Next oil flushing in                     %4u [s]\n", inputRegisters[e_time_to_oil_recovery]);
    // TODO print time remaining
}

const char *operationToString(uint16_t num)
{

    switch (num)
    {
    case 0:
        return "Idle";
    case 1:
    case 3:
        return "Cooling";
    case 2:
    case 4:
        return "Heating";
    }

    return "?";
}

uint16_t hz_to_ltr_hr(uint16_t freq)
{
    LookupTableVector ht_to_ltr_table({ // Think how this could be optimized
        {0, 0},
        {holdingRegisters[e_flow_x1], holdingRegisters[e_flow_y1]},
        {holdingRegisters[e_flow_x2], holdingRegisters[e_flow_y2]},
        {holdingRegisters[e_flow_x3], holdingRegisters[e_flow_y3]},
    });

    return ht_to_ltr_table.get(freq);
}

void test_flow_value(uint16_t val)
{
    printf("Flow from %u Hz is equivalent to %u [ltr/hour].\n", val, hz_to_ltr_hr(val));
}

float curve(float t4, float A, float B)
{
    float retval = A * (20 - t4) + 20 + B;
    // printf("curve(%2.2f, %2.2f, %2.2f) = %2.1f\n", t4, A, B, retval);
    return retval;
}

void test_equithermal_curve(int16_t ambient_temp)
{
    float temperature = curve(ambient_temp, holdingRegisters[e_curve_gain] / 100.0, (int16_t)holdingRegisters[e_curve_offset] / 10.0);
    printf("T_supply(Gain = %1.2f, Offset = %2.1f, T_ambient = %d) = %2.1f ['C]\n", holdingRegisters[e_curve_gain] / 100.0, (int16_t)holdingRegisters[e_curve_offset] / 10.0, (int16_t)ambient_temp, temperature);
}

void restore_default_settings(void)
{
    // Typowa chinska turbinka 1"
    holdingRegisters[e_flow_x1] = 30;
    holdingRegisters[e_flow_y1] = 378;
    holdingRegisters[e_flow_x2] = 75;
    holdingRegisters[e_flow_y2] = 653;
    holdingRegisters[e_flow_x3] = 240;
    holdingRegisters[e_flow_y3] = 1479;

    // Krzywa grzewcza
    holdingRegisters[e_curve_active] = 0;
    holdingRegisters[e_curve_gain] = 25;
    holdingRegisters[e_curve_offset] = 10;
    holdingRegisters[e_low_delta] = 20;
    holdingRegisters[e_high_delta] = 20;

    // Regulator PID
    #ifdef haier
    holdingRegisters[e_Kp_factor] = 100; // x10
    holdingRegisters[e_Ki_factor] = 10;  // x100
    holdingRegisters[e_Kd_factor] = 0;
    #else
    holdingRegisters[e_Kp_factor] = 10; // x10
    holdingRegisters[e_Ki_factor] = 10;  // x100
    holdingRegisters[e_Kd_factor] = 0;
    #endif

    holdingRegisters[e_pid_sampling_time] = 1;    // 1s
    holdingRegisters[e_pid_hysteresis] = 1;       // 0.1 K

    // Zabezpieczenia
    holdingRegisters[e_minimal_flow] = 15;
    holdingRegisters[e_t2_low_alarm_value] = 40;

    // Wybieg sprezarki celem smarowania
    holdingRegisters[e_oil_recovery_low_time] = 40;     // minuty
    holdingRegisters[e_oil_recovery_low_freq] = 40;     // Hz
    holdingRegisters[e_oil_recovery_restore_freq] = 56; // Hz

    // Obecny stan licznika smarowania w sekundach [s]
    inputRegisters[e_time_to_oil_recovery] = holdingRegisters[e_oil_recovery_low_time] * 60;

    holdingRegisters[e_mode] = 0;
    holdingRegisters[e_control_mode] = 0;
    holdingRegisters[e_curve_active] = 0;

    holdingRegisters[e_on_off_interval] = 30; // minuty
    holdingRegisters[e_off_delay] = 5;        // minuty

    holdingRegisters[e_alarm_relay_function] = 1 << ALARM_MASK;
    holdingRegisters[e_defrost_relay_function] = 1 << DEFROST_MASK;

    holdingRegisters[e_heat_input_function] = 1 << HEAT_MASK;
    holdingRegisters[e_cool_input_function] = 1 << COOL_MASK;

    holdingRegisters[e_multisplit_power_option] = 2; // TODO check. and test different values

    holdingRegisters[e_hot_water_level] = 90;              // Domylsny poziom mocy do grzania CWU
    holdingRegisters[e_hotwater_target_temperature] = 460; // domyslny target temp dla CWU
    holdingRegisters[e_hotwater_mode] = cwu_fixed_comp;    // domyslny tryb dla CWU

    holdingRegisters[e_ambient_temp_scope] = 60; // liczba minut wstecz z ktorej brana srednia Tzew dla pogodowki

#ifdef WIRELESS
    memset(wifi_ssid, 0, sizeof(wifi_ssid));
    memset(wifi_pass, 0, sizeof(wifi_pass));
#endif
    printf("Restored default settings.\n");
}


std::string getHotWaterModeStr(uint16_t mode)
{
    switch(mode)
    {
        case cwu_fixed_comp:
        return "Level";

        case cwu_fixed_temp:
        return "Temperature";

        case cwu_legacy:
        return "Legacy";
    }
    return "Unknown?";
}

std::string getRelayModeStr(uint16_t mask)
{
    std::string retval;
    std::string append_string;
    for (uint8_t bit = 0; bit < sizeof(mask) * 8; bit++)
    {
        if (bis(mask, bit))
        {
            switch (bit)
            {
            case ALARM_MASK:
                append_string = "ALARM";
                break;

            case DEFROST_MASK:
                append_string = "DEFROST";
                break;

            case HEAT_MASK:
                append_string = "HEAT";
                break;

            case COOL_MASK:
                append_string = "COOL";
                break;

            case IDLE_MASK:
                append_string = "IDLE";
                break;

            case OIL_MASK:
                append_string = "OIL_RECOVERY";
                break;

            case OPERATION_MASK:
                append_string = "OPERATION";
                break;

            case BIVALENT:
                append_string = "BIVALENT";
                break;

            case COMP:
                append_string = "COMPRESSOR";
                break;

            case BOOT:
                append_string = "BOOT";
                break;

            case HOT_WATER_MASK:
                append_string = "HOT_WATER";
                break;

            default:
                append_string = std::to_string(bit);
            }
            if (retval.empty() == true)
                retval = retval + append_string;
            else
                retval = retval + " | " + append_string;
        }
    }

    return retval;
}