#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <vector>
#include <set>
#include <sstream>
#include <iomanip>

#include "cli_commands.hpp"
#include "lookup_table.hpp"
#include "struct.h"
#include "modbus_registers.h"

#if defined PICO_ON_DEVICE
#include "settings.hpp"
#include "pico/unique_id.h"
#include "stopwatch.hpp"
#include "lns.h"
#endif

#include "nanomodbus.h"

using namespace std::literals;

#ifdef WIRELESS
#include "wireless.hpp"
#endif
using Tokens = std::vector<std::string>;
uint32_t git_hash{0};

#if !defined PICO_ON_DEVICE // linux
bool g_preheat_request;
extern int updateHoldingRegister(uint16_t from, uint16_t to);
extern int updateHoldingRegister(uint16_t reg);
extern int updateInputRegister(uint16_t from, uint16_t to);
extern int updateInputRegister(uint16_t reg);
void registers_restore_default();
uint8_t g_operationMode;
bool g_defrost_request;
#else // pico
extern int8_t figlarz;
extern stopwatch_t g_defrost_timer;
extern bool g_defrost_request;
extern size_t getHash(const std::string &unique_id_str, size_t depth);
#endif

extern uint8_t g_operationMode;

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
        tokens[0] = "0-" + std::to_string(e_holding_last_item - 1);
    }

    std::set<uint16_t> registers = registers_to_show(tokens, e_holding_last_item - 1);

    if (tokens.size() == 0)
    {
        printf("Example usage : %s 1 2 5-10 15 16\n", __func__);
        return;
    }
    printf("Showing %2zu registers:\n", registers.size());

#if !defined PICO_ON_DEVICE
    if (!registers.empty())
        updateHoldingRegister(*registers.begin(), *registers.rbegin());
#endif

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
        tokens[0] = "0-" + std::to_string(e_input_last_item - 1);
    }

    std::set<uint16_t> registers = registers_to_show(tokens, e_input_last_item);

    if (tokens.size() == 0)
    {
        printf("Example usage : %s 1 2 5-10 15 16\n", __func__);
        return;
    }
    printf("Showing %2zu registers:\n", registers.size());

#if !defined PICO_ON_DEVICE
    if (!registers.empty())
        updateInputRegister(*registers.begin(), *registers.rbegin());
#endif

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

void show_defrost(void)
{
    if (g_defrost_request) // TODO print which condition is met
    {
        printf("Defrost already pending.\n");
#if defined PICO_ON_DEVICE
        printf("Time remaining to finish : %lu s\n", holdingRegisters[e_defrost_max_duration] * 60 - g_defrost_timer.getSec());
#endif
        printf("Temperature delta to finish : ");
        if ((int16_t)inputRegisters[e_condenser_temp] >= 0)
            printf("%2.1f-%2.1f=%2.1f ['C]\n", holdingRegisters[e_defrost_end_t3_target] / 10.0, (int16_t)inputRegisters[e_condenser_temp] / 10.0, (holdingRegisters[e_defrost_end_t3_target] - (int16_t)inputRegisters[e_condenser_temp]) / 10.0);
        else
            printf("%2.1f+%2.1f=%2.1f ['C]\n", holdingRegisters[e_defrost_end_t3_target] / 10.0, -(int16_t)inputRegisters[e_condenser_temp] / 10.0, (holdingRegisters[e_defrost_end_t3_target] - (int16_t)inputRegisters[e_condenser_temp]) / 10.0);
    }
    else if (g_operationMode == Operation::Heating)
    {
        printf("%u s till defrost\n", inputRegisters[e_till_defrost]);
    }
    else
    {
        printf("No pending defrost, and it's not going to happen.\n");
    }

    printf("pre-defrost min interval                    %u [min]\n", holdingRegisters[e_defrost_min_interval]);
    printf("pre-defrost max odu t4-t3 delta           %4.1f ['K]\n", holdingRegisters[e_defrost_max_odu_delta] / 10.0);
    printf("pre-defrost max t30-t3 drop               %4.1f ['K]\n", holdingRegisters[e_defrost_max_t3_drop] / 10.0);
    printf("Defrost target T3 temperature             %4.1f ['C]\n", holdingRegisters[e_defrost_end_t3_target] / 10.0);
    printf("Defrost compressor max speed                %u [Hz]\n", holdingRegisters[e_defrost_max_frequency]);
    printf("Defrost max duration                        %u [min]\n", holdingRegisters[e_defrost_max_duration]);
}

void show_settings()
{
    std::string tempStr;
    switch (holdingRegisters[e_control_mode])
    {
    case Control::Local:
        tempStr = "Local 0-10V";
        break;

    case Control::RemoteLevel:
        tempStr = "Remote 0-100%";
        break;

    case Control::RemoteTemperature:
        tempStr = "Remote temp.";
        break;

    default:
        tempStr = "Unknown [" + std::to_string(holdingRegisters[e_control_mode]) + "]";
        break;
    }

    printf("Current settings:\n");
    printf("Type of power control                       %s\n", tempStr.c_str());
    printf("Type of temperature control                 %s\n", holdingRegisters[e_curve_active] == 0 ? "Static" : "Dynamic");
    show_pid();
    show_temperature();
    show_bivalent();
#if defined(midea4) || defined(gree)
    show_defrost();
#endif

    printf("T2_low temperature alarm value            %2.1f ['C]\n", (((int16_t)holdingRegisters[e_t2_low_alarm_value]) / 10.0));
    printf("Flow_low alarm value                       %u [Hz]\n", (holdingRegisters[e_minimal_flow]));
#ifndef MULTISPLIT
    show_oil();
#endif
    printf("DHW heating level                          %2u [%%]\n", (holdingRegisters[e_dhw_level]));

    show_relay_functions();
    show_input_functions();

#ifdef WIRELESS
    printf("Wifi SSID name :                           %s\n", wifi_ssid.c_str());
    printf("Wifi PASSWORD  :                           %s\n", hidden_password(wifi_pass).c_str());
#endif
#ifdef MULTISPLIT
    printf("Multisplit power selector position     (%01X)%u\n", holdingRegisters[e_multisplit_power_option], holdingRegisters[e_multisplit_power_option]);
#endif
    // TODO: function parsing power selector to horse-power notation
}

void set_relay_polarity(uint8_t relay_no, const std::string &str)
{
    if (str == "0" || str == "no")
        cbi(holdingRegisters[e_relay_polarity], relay_no);
    else if (str == "1" || str == "nc")
        sbi(holdingRegisters[e_relay_polarity], relay_no);
    else
        printf("Incorrect arguments\n");
}

void show_relay_functions()
{
    printf("ALARM relay function                       %s\n", getRelayModeStr(holdingRegisters[e_alarm_relay_function]).c_str());
    printf("ALARM relay polarity                       %s\n", bis(holdingRegisters[e_relay_polarity], 0) ? "N.C." : "N.O.");
    printf("DEFROST relay function                     %s\n", getRelayModeStr(holdingRegisters[e_defrost_relay_function]).c_str());
    printf("DEFROST relay polarity                     %s\n", bis(holdingRegisters[e_relay_polarity], 1) ? "N.C." : "N.O.");
}

void show_control(void)
{
    printf("Control set to %s\n", controlToSv(holdingRegisters[e_control_mode]).data());
}

void show_dhw(void)
{
    const char *prefix = "DHW";
    printf("%s Mode                                    %s\n", prefix, getDHWModeStr(holdingRegisters[e_dhw_mode]));
    printf("%s Target temeprature                   %4.1f ['C]\n", prefix, holdingRegisters[e_dhw_target_temperature] / 10.0);
    printf("%s Target level                         %4u [%%]\n", prefix, holdingRegisters[e_dhw_level]);
}

void show_bivalent(void)
{
    const char *prefix = "Bivalent";
    printf("%s0 temeprature                        %4.1f ['C]\n", prefix, ((int16_t)holdingRegisters[e_bivalent0_temp]) / 10.0);
    printf("%s0 hysteresis                          %4.1f ['K]\n", prefix, ((int16_t)holdingRegisters[e_bivalent0_hysteresis]) / 10.0);
    printf("%s0 level                               %4u [%%]\n", prefix, holdingRegisters[e_bivalent0_level]);
    printf("%s1 temeprature                        %4.1f ['C]\n", prefix, ((int16_t)holdingRegisters[e_bivalent1_temp]) / 10.0);
    printf("%s1 hysteresis                          %4.1f ['K]\n", prefix, ((int16_t)holdingRegisters[e_bivalent1_hysteresis]) / 10.0);

}

void show_temperature(void)
{
    printf("Actual temperature target                %4.1f ['C]\n", inputRegisters[e_temp_setpoint_ro] / 10.0);
    printf("Set temperature target                   %4.1f ['C]\n", holdingRegisters[e_temp_setpoint] / 10.0);
    printf("Upper delta treshold                    +%4.1f ['K]\n", holdingRegisters[e_high_delta] / 10.0);
    printf("Lower delta treshold                    -%4.1f ['K]\n", holdingRegisters[e_low_delta] / 10.0);
    printf("Dynamic temperature gain factor          %4.2f\n", holdingRegisters[e_curve_gain] / 100.0);
    printf("Dynamic temperature offset value         %4.1f ['K]\n", ((int16_t)holdingRegisters[e_curve_offset]) / 10.0);
    printf("Auto-OFF delay time                      %4u [min]\n", holdingRegisters[e_off_delay]);
    printf("Minimum OFF -> ON interval               %4u [min]\n", holdingRegisters[e_on_off_interval]);
    printf("Remaining time until next start          %4u [s]\n", inputRegisters[e_interval_on_off_remaining]);
    printf("Ambient temperature range scope          %4u [h]\n", holdingRegisters[e_ambient_temp_scope]);
    show_dhw();
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

    default:
        return 0;
    }

    return 0;
}

#if defined PICO_ON_DEVICE
std::string get_unique_id_string()
{
    char usbd_serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1] = {0};
    pico_get_unique_board_id_string(usbd_serial_str, sizeof(usbd_serial_str));

    return std::string(usbd_serial_str);
}
#endif

void system_info(void)
{
#if defined PICO_ON_DEVICE
    char usbd_serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1] = {0};
    pico_get_unique_board_id_string(usbd_serial_str, sizeof(usbd_serial_str));

    printf("Unique ID %s\n", get_unique_id_string().c_str());
    printf("Build: %s %s\nGit hash: %08lx \n", __DATE__, __TIME__, git_hash);
#ifdef gree
    printf("ODU model: %s (0x%04x)\n", get_odu_model().data(), odu_model_key);
#endif
    printf("Compressor operation range %u-%u [Hz]\n", inputRegisters[e_compressor_min_frequency], inputRegisters[e_compressor_max_frequency]);

    if (getHash(get_unique_id_string(), 10) == UNIQUE_ID)
    {
        printf("License: OK\n");
        figlarz = 0;
    }
    else
    {
        printf("License invalid :(\n");
        figlarz = 0;
    }
#endif
}

void show_input_functions()
{
    const char *suffix = " input function                        ";
    printf("HEAT%s%s\n", suffix, getInputModeStr(holdingRegisters[e_heat_input_function]).c_str());
    printf("COOL%s%s\n", suffix, getInputModeStr(holdingRegisters[e_cool_input_function]).c_str());
}

void show_pid(void)
{
    const char *prefix = "PID ";
    printf("%sK_p                                  %4.1f\n", prefix, (holdingRegisters[e_Kp_factor] / 10.0));
    printf("%sK_i                                  %4.2f\n", prefix, (holdingRegisters[e_Ki_factor] / 100.0));
    printf("%sK_d                                  %4.1f\n", prefix, (holdingRegisters[e_Kd_factor] / 10.0));
    printf("%ssampling time                        %4u [s]\n", prefix, (holdingRegisters[e_pid_sampling_time]));
    printf("%shysteresis                           %4.1f ['K]\n", prefix, (holdingRegisters[e_pid_hysteresis] / 10.0));
    printf("%sC_p                                  %4.1f\n", prefix, ((int16_t)inputRegisters[e_pid_p_component]) / 10.0);
    printf("%sC_i                                  %4.1f\n", prefix, ((int16_t)inputRegisters[e_pid_i_component]) / 10.0);
    printf("%sC_d                                  %4.1f\n", prefix, ((int16_t)inputRegisters[e_pid_d_component]) / 10.0);
}

void show_softstart(void)
{
    printf("Pre-heat temperature limit            %4.1f ['C]\n", (holdingRegisters[e_preheat_temp] / 10.0));
    printf("Pre-cool temperature limit            %4.1f ['C]\n", (holdingRegisters[e_precool_temp] / 10.0));
    printf("Pre hysteresis                        %4.1f ['K]\n", (holdingRegisters[e_pre_hysteresis] / 10.0));
    printf("T2 temperature                        %4.1f\n", ((int16_t)inputRegisters[e_evaporator_temp]) / 10.0);
    printf("Pre-heat state                        %u\n", g_preheat_request);
}

#ifndef MULTISPLIT
void show_oil(void)
{
    const char *prefix = "Oil recovery ";
    printf("%slower comp limit            %4u [Hz]\n", prefix, (holdingRegisters[e_oil_recovery_low_freq]));
    printf("%stime below limit            %4u [min]\n", prefix, (holdingRegisters[e_oil_recovery_low_time]));
    printf("%sLVL restore.                %4u [Hz]\n", prefix, (holdingRegisters[e_oil_recovery_restore_freq]));
    printf("Next oil recovery in                     %4u [s]\n", inputRegisters[e_time_to_oil_recovery]);
}
#endif
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
    LookupTable<int32_t> ltr_table({
        // Think how this could be optimized
        {0, 0},
        {holdingRegisters[e_flow_x1], holdingRegisters[e_flow_y1]},
        {holdingRegisters[e_flow_x2], holdingRegisters[e_flow_y2]},
        {holdingRegisters[e_flow_x3], holdingRegisters[e_flow_y3]},
    });

    return ltr_table.get(freq);
}

void calculate_curve(float x1, float y1, float x2, float y2)
{
    float a = (y2 - y1) / (x1 - x2);
    float b = y1 - a * (20 - x1) - 20;

    printf("A = %2.2f, B = %2.1f\n", a, b); // TODO: make bool = optional use printf
    holdingRegisters[e_curve_gain] = static_cast<uint16_t>(a * 100);
    holdingRegisters[e_curve_offset] = static_cast<uint16_t>(b * 10);
}

void calculate_curve(const std::string &str)
{
    Tokens tokens = tokenize(str);
    if (tokens.size() != 4)
    {
        printf("Usage : give 4 parameters, T_ambient1, T_supply1, T_ambient2, T_supply2\n");
        return;
    }

    float x1, x2, y1, y2;
    if (tokens[0] == tokens[2] || tokens[1] == tokens[3])
    {
        printf("Temperatures must not be the same.\n");
        return;
    }
    x1 = std::stof(tokens[0]);
    y1 = std::stof(tokens[1]);
    x2 = std::stof(tokens[2]);
    y2 = std::stof(tokens[3]);

    calculate_curve(x1, y1, x2, y2);
}

void test_flow_value(int16_t val)
{
    if (val < 0)
    {
        printf("Flow cannot be negative.\n");
        return;
    }
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

const char *getDHWModeStr(uint16_t mode)
{
    switch (mode)
    {
    case DHW::FixedLevel:
        return "Level";

    case DHW::FixedTemp:
        return "Temperature";

    case DHW::Legacy:
        return "Legacy";
    }
    return "Unknown?";
}

std::string_view controlToSv(uint16_t val)
{
    switch (val)
    {
    case Control::Local:
        return "Local 0-10V"sv;
    case Control::RemoteLevel:
        return "Remote 0-100"sv;
    case Control::RemoteTemperature:
        return "Remote temperature"sv;

    default:
        return "Unknown?"sv;
    }
}

std::string getInputModeStr(uint16_t mask)
{
    switch (mask)
    {
    case 0:
        return "NoFunction";
    case 1:
        return "Cooling";
    case 2:
        return "Heating";
    case 3:
        return "DHW";
    default:
        return "??";
    }
    return "";
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

            case BIVALENT_MASK:
                append_string = "BIVALENT";
                break;

            case OPERATION_MASK:
                append_string = "OPERATION";
                break;

            case PREHEAT:
                append_string = "PREHEAT";
                break;

            case COMP:
                append_string = "COMPRESSOR";
                break;

            case WAKEUP:
                append_string = "WAKE";
                break;

            case DHW_MASK:
                append_string = "DHW";
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