#include "modbus_registers.h"
#include <map>
#include <cstdio>
#include <cstdlib>
#include "struct.h"

std::map<uint16_t, std::pair<int16_t, int16_t>> holdingLimits{
    {e_control_mode, {Control::Local, Control::RemoteTemperature}},
    {e_mode, {Operation::Idle, 4}},
    {e_level, {0, 100}},
    {e_temp_setpoint, {0, 500}},
    {e_increment, {0, 100}},
    {e_decrement, {0, 100}},
    {e_pipe_override, {0, 255}},
    {e_pid_sampling_time, {0, 60}},
    {e_override_compressor, {0, 80}},
    {e_off_delay, {5, 20}},
    {e_Kp_factor, {0, 500}},
    {e_Ki_factor, {0, 500}},
    {e_Kd_factor, {0, 500}},
    {e_curve_gain, {0, 100}},
    {e_curve_offset, {-150, 150}},
    {e_curve_active, {0, 1}},
    {e_low_delta, {1, 50}},
    {e_high_delta, {1, 50}},
    {e_on_off_interval, {10, 240}},
    {e_multisplit_power_option, {0, 15}},
    {e_oil_recovery_low_time, {5, 60}},
    {e_t2_low_alarm_value, {-300, 100}},
    {e_minimal_flow, {0, 50}},
    {e_dhw_level, {1, 100}},
    {e_defrost_max_odu_delta, {50, 150}},
    {e_defrost_max_frequency, {40, 80}},
    {e_defrost_end_t3_target, {100, 400}},
    {e_defrost_max_duration, {1, 15}},
    {e_defrost_min_interval, {30, 120}},
    {e_defrost_max_t3_drop, {10, 50}},
    {e_pre_hysteresis, {0, 50}},
    {e_preheat_temp, {150, 450}},
    {e_precool_temp, {50, 200}},
    {e_dhw_mode, {DHW::FixedLevel, DHW::Legacy}},
    {e_dhw_target_temperature, {250, 600}},
    {e_cool_input_function, {0, Function::DHW}},
    {e_heat_input_function, {0, Function::DHW}},
    {e_ambient_temp_scope, {1, 24}},
    {e_bivalent0_temp, {-250, 100}},
    {e_bivalent0_hysteresis, {0, 50}},
    {e_bivalent1_temp, {-250, 100}},
    {e_bivalent1_hysteresis, {0, 50}},
    {e_bivalent0_level, {0, 100}},

};

auto getLimits(uint16_t reg)
{
    std::pair<int16_t, int16_t> dummy{0,0};
    try
    {
        return holdingLimits.at(reg);
    }
    catch (const std::exception &e)
    {
        printf("no such reg number %u (%s)\n", reg, e.what());
    }
    return dummy;
}

const char *inputRegToStr(uint8_t reg)
{
    switch (reg)
    {
    case e_tx_count:
        return "Counter vale of sent LNS frames";
    case e_rx_err_count:
        return "Number of incorrectly received LNS frames";
    case e_xfer_ok_ratio:
        return "good/bad ratio of LNS rx frames";
    case index_enum:
        return "rcv_index";
    case eev1_ro:
        return "Expansion valve (B) position";
    case e_operation_mode_ro:
        return "e_operation_mode_ro";
    case selector_switch:
        return "Power selector switch value";
    case control_mode_ro:
        return "IDU operation mode";
    case eev_ro:
        return "Expansion valve (A) position";
    case e_fan:
        return "ODU Fan speed [RPM]";
    case e_compressor:
        return "ODU Compressor speed [Hz]";
    case e_pwr:
        return "ODU Active power [W]";
    case e_outdoor_mode:
        return "ODU operation mode";
    case e_ambient_temp_avg_ro:
        return "Ambient average temperature (T4a ['C x 10]";
    case e_discharge_temp:
        return "ODU Discharge temperature (T5) ['C x 10]";
    case e_condenser_temp:
        return "ODU exchanger temperature (T3) [\'C x 10]";
    case e_ambient_temp:
        return "ODU ambient temperature (T4) [\'C x 10]";
    case e_indoor_temp:
        return "IDU supply temperature (T1) [\'C x 10]";
    case e_evaporator_temp:
        return "IDU exchanger temperature (T2) [\'C x 10]";
    case e_indoor_ro:
        return "e_indoor_ro";
    case e_water_flow:
        return "IDU water flow pulse frequency [Hz]";
    case e_faults_ro:
        return "IDU Fault status register";
    case e_fault_byte11:
        return "ODU Fault status register_1";
    case e_fault_byte12:
        return "ODU Fault status register_2";
    case e_target_frequency:
        return "IDU compressor target frequency [Hz]";
    case e_powerLevel_100:
        return "Target power level (0-100) %%";
    case e_compressor_min_frequency:
        return "Minimum compressor frequency [Hz]";
    case e_compressor_max_frequency:
        return "Maximum compressor frequency [Hz]";
    case e_water_flow_ltr_per_hour:
        return "Water flow [ltr/hr]";
    case e_temp_setpoint_ro:
        return "Supply temperature setpoint [\'C x 10]";
    case e_superheat_ro:
        return "Superheat value [\'K x 10]";
    case e_curve_offset_ro:
        return "Equithermal curve offset value";
    case e_ipm_temperature:
        return "IPM module temperature";
    case e_suction_temperature:
        return "Compressor suction temperature";
    case e_water_in_ro:
        return "Return water temperature [\'C x 10]";
    case e_refrigerant_in:
        return "Vapor Refrigerant temperature [\'C x 10]";
    case e_refrigerant_out:
        return "Condensed refrigerant temperature [\'C x 10]";
    case e_COP:
        return "COP";
    case e_heat_power:
        return "Heating power [W]";
    case e_ac_voltage:
        return "AC Voltage [V]";
    case e_ac_current:
        return "AC Current [mA]";
    case e_dc_bus_voltage:
        return "e_dc_bus_voltage [V]";
    case e_settings_saved:
        return "Command executed";
    case e_pid_p_component:
        return "PID_P_component";
    case e_pid_i_component:
        return "PID_I_component";
    case e_pid_d_component:
        return "PID_D_component";
    case e_pid_output:
        return "PID controler output value";
    case e_time_to_oil_recovery:
        return "Time till oil recovery mode starts";
    case e_interval_on_off_remaining:
        return "Remaining interval before next start";
    case e_auto_off_remaining:
        return "Remaining interval till stop";
    case e_till_defrost:
        return "Remaining time till defrost";
    default:
        break;
    }

    return "???";
};

const char *holdingRegToStr(uint8_t reg)
{
    switch (reg)
    {
    case e_control_mode:
        return "control_mode : 0-local, 1-remote_0-100, 2-temperature";
    case e_mode:
        return "operation mode : 0-idle, 1-cool_manual, 2-heat_manual, 3-coo_auto, 4-heat_auto";
    case e_level:
        return "Power level 0-100";
    case e_temp_setpoint:
        return "Temeprature setpoint [\'C x 10]";
    case e_increment:
        return "increment level value by %%";
    case e_decrement:
        return "decrement level value by %%";
    case e_pipe_override: // wymuszenie wartosci innej niz temperatura wymiennika lecz w postaci zakodowanej w ramce danych. Mozna dzieki temu wymusic awarie np z powodu przegrzania/zamrozenia
        return "exchanger temperature override - byte coded.";
    case e_pid_sampling_time:
        return "PID controller cycle time [s]";
    case e_pid_hysteresis:
        return "e_pid_hysteresi";
    case e_off_delay:
        return "Delay before going to OFF/STBY [min]";
    case e_override_compressor:
        return "Override compressor speed";
    case e_Kp_factor:
        return "PID controller K_p coefficient [x 10]";
    case e_dhw_level:
        return "DHW heating level";
    case e_curve_gain: // 0-100
        return "gain/slope of equithermal curve [x 100]";
    case e_curve_offset: // 0-100
        return "equithermal curve offset value (from 20'C)";
    case e_curve_active: // Czy temperatura nastawiona rzeczywista brana jest z e_temp_setpoint czy liczona z e_curve_gain i e_curve_offset
        return "Constant temperature mode - 0, equithermal mode active - 1. ";
    case e_low_delta:
        return "low_delta";
    case e_high_delta:
        return "high_delta";
    case e_on_off_interval:
        return "Interval between Off and On [min]";
    case e_ambient_temp_scope:
        return "Ambient avg scope [hour]";
    case e_flow_x1:
        return "x1 flow [Hz]";
    case e_flow_y1:
        return "y1 flow [ltr/hr]";
    case e_flow_x2:
        return "x2 flow [Hz]";
    case e_flow_y2:
        return "y2 flow [ltr/hr]";
    case e_flow_x3:
        return "x3 flow [Hz]";
    case e_flow_y3:
        return "y3 flow [ltr/hr]";
    case e_Ki_factor:
        return "PID controller K_i coefficient [x 100]";
    case e_Kd_factor:
        return "PID controller K_d coefficient [x 10]";
    case e_execute_command:
        return "Execute command";
    case e_minimal_flow:
        return "Minimal water flow value before alarm [Hz]";
    case e_t2_low_alarm_value:
        return "Exchanger low temperature alarm level (x10)";
    case e_alarm_relay_function:
        return "ALARM relay function (bitmask)";
    case e_defrost_relay_function:
        return "DEFROST relay function (bitmask)";
    case e_heat_input_function:
        return "HEAT input function";
    case e_cool_input_function:
        return "COOL input function";
    case e_multisplit_power_option:
        return "Multisplit power selector position";
    case e_oil_recovery_low_freq:
        return "Compressor speed [Hz] below which, oil recovery timer runs.";
    case e_oil_recovery_low_time:
        return "Time until oil recovery mode stars.";
    case e_oil_recovery_restore_freq:
        return "Compressor speed [Hz] till which oil recovery mode ends.";
    case e_dhw_mode:
        return "dhw mode";
    case e_dhw_target_temperature:
        return "dhw target temperature";
    case e_defrost_max_frequency:
        return "Defrost max. compressor frequency";
    case e_defrost_end_t3_target:
        return "Defrost end T3 target ['C x 10]";
    case e_defrost_max_duration:
        return "Defrost max duration [min]";
    case e_defrost_min_interval:
        return "Defrost minimal interval [min]";
    case e_defrost_max_odu_delta:
        return "Defrost max t4-t3 delta ['K x 10]";
    case e_defrost_max_t3_drop:
        return "Defrost max t30-t3 drop ['K x 10]";
    case e_10v_scale:
        return "scaling of 0-10V input [%%]";
    case e_preheat_temp:
        return "Preheat temperature treshold ['C x 10]";
    case e_precool_temp:
        return "Precool temperature treshold ['C x 10]";
    case e_pre_hysteresis:
        return "Preheating hysteresis ['C x 10]";
    case e_relay_polarity:
        return "Polarity/logic of output relays [NO/NC]";
    case e_pwr_override:
        return "Override Electrical Power [W]";

    case e_bivalent0_temp:
        return "Bivalent0 ambient temperature";
    case e_bivalent0_hysteresis:
        return "Bivalent0 hysteresis";
    case e_bivalent1_temp:
        return "Bivalent0 ambient temperature";
    case e_bivalent1_hysteresis:
        return "Bivalent1 hysteresis";
    case e_bivalent0_level:
        return "Bivalent0 level";

    case e_holding_last_item:
        return "e_holding_last_item";
    }

    return "????";
};
