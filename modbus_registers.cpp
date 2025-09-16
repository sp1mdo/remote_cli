#include "modbus_registers.h"

const char *inputRegToStr(uint8_t reg)
{
    switch (reg)
    {
    case e_tx_count:
        return "Counter vale of sent LNS frames";
    case e_rx_err_count:
        return "Number of incorrectly received LNS frames";
    case e_xfer_ok_ratio:
        return "Ratio of correctly sent frames to received errors";
    case index_enum:
        return "index_enum";
    case eev1_ro:
        return "eev1_ro";
    case e_operation_mode_ro:
        return "e_operation_mode_ro";
    case selector_switch:
        return "Power selector switch value";
    case control_mode_ro:
        return "IDU operation mode";
    case eev_ro:
        return "Expansion valve position";
    case e_fan:
        return "ODU Fan speed [Hz]";
    case e_compressor:
        return "ODU Compressor speed [Hz]";
    case e_pwr:
        return "ODU Active power [W]";
    case e_outdoor_mode:
        return "ODU operation mode";
    case e_ambient_temp_avg_ro:
        return "e_ambient_temp_avg_ro";
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
    case e_curve_gain_ro:
        return "Equithermal curve gain coefficient";
    case e_curve_offset_ro:
        return "Equithermal curve offset value";
    case e_ipm_temperature:
        return "IPM module temperature";
    case e_ssuction_temperature:
        return "Compressor ssuction temperature";
    case e_water_in_ro:
        return "Return water temperature [\'C x 10]";
    case e_refrigerant_in:
        return "Vapor Refrigerant temperature [\'C x 10]";
    case e_refrigerant_out:
        return "Condensed refrigerant temperature [\'C x 10]";
    case e_COP:
        return "COP - coeficient factor ";
    case e_heat_power:
        return "Heating power [W]";
    case e_ac_voltage:
        return "AC Voltage [V]";
    case e_ac_current:
        return "AC Current [mA])";
    case e_dc_bus_voltage:
        return "e_dc_bus_voltage";
    case e_settings_saved:
        return "HMI save settings counter";
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
        return "Remaining time [s] to defrost";
    default:
        return "???";
    }

    return "unknown?";
};


const char *holdingRegToStr(uint8_t reg)
{
    switch (reg)
    {
    case e_control_mode:
        return "control_mode - 0-local, 1-remote_0-100, 2-temperature";
    case e_mode:
        return "operation mode : 0  - idle, 1 - cool_manual, 2 - heat_manual, 3-coo_auto, 4-heat_auto";
    case e_level:
        return "power level 0-100";
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
        return "e_pid_hysteresis - not used";
    case e_off_delay:
        return "Delay before going to OFF/STBY [min]";
    case e_override_compressor:
        return "Force compressor speed - not recommended to use";
    case e_Kp_factor:
        return "P.I.D. controller K_p coefficient [x 10]";
    case e_hot_water_level:
        return "Hot water heating level";
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
        return "e_ambient_temp_scope";
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
        return "P.I.D. controller K_i coefficient [x 100]";
    case e_Kd_factor:
        return "P.I.D. controller K_d coefficient [x 10]";
    case e_save_button:
        return "not used";
    case e_minimal_flow:
        return "Minimal water flow value before alarm [Hz]";
    case e_t2_low_alarm_value:
        return "Exchanger low temperature alarm level (x10)";
    case e_oil_recovery_low_freq:
        return "Compressor speed [Hz] below which, oil recovery timer runs.";
    case e_oil_recovery_low_time:
        return "Time until oil recovery mode stars.";
    case e_oil_recovery_restore_freq:
        return "Compressor speed [Hz] till which oil recovery mode ends.";
    case e_alarm_relay_function:
        return "ALARM relay function (bitmask)";
    case e_defrost_relay_function:
        return "DEFROST relay function (bitmask)";
    case e_multisplit_power_option:
        return "Multisplit power selector position";
    case e_heat_input_function:
        return "HEAT input function";
    case e_cool_input_function:
        return "COOL input function";
    case e_defrost_max_frequency:
        return "Defrost max. compressor frequency";
    case e_defrost_end_t3_target:
        return "Defrost end T3 target";
    case e_defrost_max_duration:
        return "Defrost max duration [min]";
    case e_holding_last_item:
        return "e_holding_last_item";
    }

    return "unknown?";
};
