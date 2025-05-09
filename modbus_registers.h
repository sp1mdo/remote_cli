#ifndef _modbus_registers_h_
#define _modbus_registers_h_



#include <inttypes.h>
#include <stdbool.h>
#include "modbus_registers.h"

extern const char *inputRegToStr(uint8_t reg);
extern const char *holdingRegToStr(uint8_t reg);
extern uint16_t *holdingRegisters;
extern uint16_t inputRegisters[];

enum input_reg
{
    e_tx_count = 0, // counter for transmitted frames
    e_rx_err_count,    // counter for received valid frames
    e_xfer_ok_ratio,   // index in g_lns_rcv_buf when valid frame was found last time
    index_enum,       // index local variable
    eev1_ro, // 0-10V confirmation from outdoor unit //spare
    e_operation_mode_ro, // 0-11 from indoor // spare
    selector_switch, // wartosc binarnej reprezentacji pozycji selektora zakresu czestotliwosci
    control_mode_ro,
    eev_ro, // pozycja zaworu rozpreznego (0 gdy agregat jest na kapilarze)
    e_fan, // predkosc wentylatora w [Hz]
    e_compressor, // predkosc kompresora w [Hz]
    e_pwr, // moc pobierana przez agregat w [W]
    e_outdoor_mode, // 0 stop, 1-chlodzenie, 2-grzanie, 7-defrost, +128 - tryb auto/taktowanie
    e_ambient_temp_avg_ro,
    e_discharge_temp, //T5 temperatura goracego konca sprezarki
    e_condenser_temp, //T3 temperatura za elementem rozpreznym
    e_ambient_temp, //T4 temperatura zewnetrzna
    e_indoor_temp, //T1 temperatura zasilania/pomieszczenia (zaleznie gdzie czujnik)
    e_evaporator_temp, //T2 temperatura wymiennika ciepla
    e_indoor_ro, //sent byte check only
    e_water_flow,
    e_faults_ro, // bledy z JZ
    e_fault_byte11, // bledy z JZ
    e_fault_byte12, // bledy z JZ
    e_target_frequency,
    e_powerLevel_100,
    e_compressor_min_frequency,
    e_compressor_max_frequency,
    e_water_flow_ltr_per_hour,
    e_temp_setpoint_ro,
    e_curve_gain_ro,
    e_curve_offset_ro,
    e_ipm_temperature, //update web
    e_ssuction_temperature, //update web
    e_water_in_ro,
    e_refrigerant_in,
    e_refrigerant_out,
    e_COP,
    e_heat_power,
    e_ac_voltage,
    e_ac_current,
    e_dc_bus_voltage, // update webpage ?
    e_settings_saved,
    e_water_delta, // SPARE ?
    e_pid_p_component,
    e_pid_i_component,
    e_pid_d_component,
    e_pid_output,
    e_time_to_oil_recovery,
    e_interval_on_off_remaining,
    e_auto_off_remaining,
    e_till_defrost,
    e_input_last_item,
};

enum holding_reg
{
    e_control_mode = 0, // 0 - local, 1 - remote_classic (0-10V)
    e_mode, // 0  - idle, 1 - cooling, 2 - heating
    e_level, // from 0 to 100
    e_delta_offset, // korekta obliczenia delty na potrzebe cop ( K x 10)
    e_temp_setpoint, // nastawa temperatury skalowanie x10 (od 170 do 300) w trybie klimatyzatorowym, poki co eksperymentalne
    e_increment, // wpisanie 1 powoduje jednorazowe zwiekszenie o 1 poziomu mocy (zakres 0-10). Mozna wpisac wiecej niz 1 wtedy zwiekszy wiecej
    e_decrement, // wpisanie 1 powoduje jednorazowe zmniejszeni o 1 poziomu mocy (zakres 0-10). Mozna wpisac wiecej niz 1 wtedy zmniejszy wiecej
    e_odu_fan_override, //wymuszenie wartosci innej niz zadana/0-10v lecz w postaci zakodowanej w ramce danych
    e_pipe_override,  //update webpage
    e_pid_sampling_time,
    e_pid_hysteresis, // update webpage
    e_off_delay,
    e_override_compressor,
    e_Kp_factor,
    e_ambient_temp_scope, // x 100
    e_hot_water_level,
    e_curve_gain, // 0-100
    e_curve_offset, // 0-100 
    e_curve_active,// Czy temperatura nastawiona rzeczywista brana jest z e_temp_setpoint czy liczona z e_curve_gain i e_curve_offset
    e_low_delta,
    e_high_delta,
    e_on_off_interval,
    e_flow_x1,
    e_flow_y1,
    e_flow_x2,
    e_flow_y2,
    e_flow_x3,
    e_flow_y3,
    HOLDING_SPARE_4, // gree room
    HOLDING_SPARE_5, // gree setpoint
    HOLDING_SPARE_6, // gree idu fan
    e_Ki_factor,
    e_Kd_factor,
    e_save_button,
    e_minimal_flow,
    e_t2_low_alarm_value,
    e_alarm_relay_function, //update web
    e_defrost_relay_function, //update web
    e_heat_input_function,
    e_cool_input_function,
    e_multisplit_power_option, 
    e_oil_recovery_low_freq,
    e_oil_recovery_low_time,
    e_oil_recovery_restore_freq,
    e_hotwater_mode,
    e_hotwater_target_temperature,
    e_holding_last_item,
};

enum control_mode
{
    e_ctrl_mode_local = 0, 
    e_ctrl_mode_remote_100, 
    e_ctrl_mode_remote_temp,
    e_ctrl_mode_last_item, 
};

#endif