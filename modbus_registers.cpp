#include "modbus_registers.h"

#if 0
enum class RegisterType : uint8_t
{
    Holding = 0,
    Input = 1
};

enum InputRegisters : uint16_t
{
    INPUT_LNS_TX_COUNT = 0, // counter for transmitted frames
    INPUT_LNS_ERR_COUNT,    // counter for received valid frames
    INPUT_LNS_TX_ERR_RATIO,   // index in g_lns_rcv_buf when valid frame was found last time
    INPUT_LNS_RCV_BUF_IND,       // index local variable
    INPUT_SPARE1, // 0-10V confirmation from outdoor unit //spare
    INPUT_SPARE2, // 0-11 from indoor // spare
    INPUT_SELECTOR_SWITCH, // wartosc binarnej reprezentacji pozycji selektora zakresu czestotliwosci
    INPUT_IDU_CONTROL_MODE,
    INPUT_ODU_EEV0_POSITION, // pozycja zaworu rozpreznego (0 gdy agregat jest na kapilarze)
    INPUT_ODU_FAN_SPEED, // predkosc wentylatora w [Hz]
    INPUT_ODU_COMP_SPEED, // predkosc kompresora w [Hz]
    INPUT_ODU_POWER_W, // moc pobierana przez agregat w [W]
    INPUT_ODU_MODE, // 0 stop, 1-chlodzenie, 2-grzanie, 7-defrost, +128 - tryb auto/taktowanie
    INPUT_IDU_SETPOINT_BYTE,
    INPUT_ODU_T5, //T5 temperatura goracego konca sprezarki
    INPUT_ODU_T3, //T3 temperatura za elementem rozpreznym
    INPUT_ODU_T4, //T4 temperatura zewnetrzna
    INPUT_IDU_T2, //T2 temperatura zasilania/pomieszczenia (zaleznie gdzie czujnik)
    INPUT_IDU_T1, //T2 temperatura wymiennika ciepla
    INPUT_SPARE3, //sent byte check only
    INPUT_WATER_FLOW,
    INPUT_FAULTS1, // bledy z JZ
    INPUT_FAULTS2, // bledy z JZ
    INPUT_FAULTS3, // bledy z JZ
    INPUT_ODU_COMP_TARGET_FREQ,
    INPUT_ODU_TARGET_PWR_LVL,
    INPUT_ODU_COMP_MIN_FREQ,
    INPUT_ODU_COMP_MAX_FREQ,
    INPUT_WATER_FLOW_LTR_HR,
    INPUT_TEMP_SETPOINT,
    INPUT_CURVE_GAIN,
    INPUT_CURVE_OFFSET,
    INPUT_CURVE_ACTIVE,
    INPUT_OFF_ON_MIN_INTERVAL,
    INPUT_WATER_RETURN_TEMP,
    INPUT_REFR_VAPOR_TEMP,
    INPUT_REFR_CONDENS_TEMP,
    INPUT_COP,
    INPUT_HEAT_POWER_W,
    INPUT_ODU_AC_VOLTAGE,
    INPUT_ODU_AC_CUIRRENT,
    INPUT_SPARE4, // SPARE ?
    INPUT_SETTINGS_SAVED,
    INPUT_T3_DELTA, // SPARE ?
    INPUT_PID_P_COMPONENT,
    INPUT_PID_I_COMPONENT,
    INPUT_PID_D_COMPONENT,
    INPUT_PID_OUTPUT,
    INPUT_REM_TIME_TO_OIL_RECOVERY,
    INPUT_REM_TIME_TO_ON_OFF,
    INPUT_REM_TIME_AUTO_OFF,
    INPUT_LAST_ITEM,
};

enum HoldingRegisters : uint16_t
{
    INPUT_CONTROL = 0,
    HOLDING_OPERATION_MODE, // 0  - idle, 1 - cooling, 2 - heating
    HOLDING_POWER_LEVEL, // from 0 to 100
    HOLDING_SPARE1, // wymuszenie/oszukanie temepratury czujnika, domyslnie zero, gdy inna niz zero wtedy ta wartosc jest brana pod uwage
    HOLDING_TEMP_SETPOINT, // nastawa temperatury skalowanie x10 (od 170 do 300) w trybie klimatyzatorowym, poki co eksperymentalne
    HOLDING_INCREMENT, // wpisanie 1 powoduje jednorazowe zwiekszenie o 1 poziomu mocy (zakres 0-10). Mozna wpisac wiecej niz 1 wtedy zwiekszy wiecej
    HOLDING_DECREMENT, // wpisanie 1 powoduje jednorazowe zmniejszeni o 1 poziomu mocy (zakres 0-10). Mozna wpisac wiecej niz 1 wtedy zmniejszy wiecej
    HOLDING_SPARE2, //wymuszenie wartosci innej niz zadana/0-10v lecz w postaci zakodowanej w ramce danych
    HOLDING_SPARE3,  //wymuszenie wartosci innej niz temperatura wymiennika lecz w postaci zakodowanej w ramce danych. Mozna dzieki temu wymusic awarie np z powodu przegrzania/zamrozenia
    HOLDING_PID_INTERVAL,
    HOLDING_SPARE4, // juz nie uzywany, mozna to czyms zastapic, bylo hysteresis
    HOLDING_OFF_DELAY,
    HOLDING_OVERRIDE_COMPRESSOR,
    HOLDING_PID_KP,
    HOLDING_SPARE5, // x 100
    HOLDING_SPARE6,
    HOLDING_CURVE_GAIN, // 0-100
    HOLDING_CURVE_OFFSET, // 0-100 
    HOLDING_CURVE_ACTIVE,// Czy temperatura nastawiona rzeczywista brana jest z e_temp_setpoint czy liczona z e_curve_gain i e_curve_offset
    HOLDING_LOW_DELTA,
    HOLDING_HIGH_DELTA,
    HOLDING_ON_OFF_INTERVAL,
    HOLDING_FLOW_X1,
    HOLDING_FLOW_Y1,
    HOLDING_FLOW_X2,
    HOLDING_FLOW_Y2,
    HOLDING_FLOW_X3,
    HOLDING_FLOW_Y3,
    HOLDING_SPARE7,
    HOLDING_SPARE8, //SPARE ?
    HOLDING_SPARE9, //SPARE ?
    HOLDING_PID_KI,
    HOLDING_PID_KD,
    HOLDING_SAVE_BUTTON,
    HOLDING_MIN_FLOW,
    HOLDING_T2_MIN_ALARM,
    HOLDING_ALARM_RELAY_FUNCTION,
    HOLDING_DEFROST_RELAY_FUNCTION,
    HOLDING_HEAT_INPUT_FUNCTION,
    HOLDING_COOL_INPUT_FUNCTION,
    HOLDING_MULTISPLIT_POWER_SELECTOR, // consider to not use, instead reg 44 and 45
    HOLDING_OIL_RECOVER_LOW_PERCENTAGE,
    HOLDING_OIL_RECOVERY_TIME,
    HOLDING_OIL_RECOVERY_RESTORE_LEVEL,
    HOLDING_LAST_ITEM
};


class ModbusRegister
{
public:
    ModbusRegister() {}

    ModbusRegister(const std::string &label, uint16_t min,
                   uint16_t max, float scale, const std::string &phyUnit,
                   bool sg = false, uint16_t value = 0) : m_Min(min),
                                                          m_Max(max),
                                                          m_Label(label),
                                                          m_Scale(scale),
                                                          m_PhyUnit(phyUnit),
                                                          m_Signed(sg),
                                                          m_Value(value)
    {
    }

    uint16_t getValue(void)
    {
        return m_Value;
    }

    std::string getRealValueStr(void)
    {
        return std::to_string(getRealValue());
    }

    float getRealValue(void)
    {
        if (m_Signed == true)
            return static_cast<int16_t>(m_Value) * m_Scale;
        else
            return m_Value * m_Scale;
    }

    std::string getLabel(void)
    {
        return m_Label;
    }

    std::string getPhysicalUnit(void)
    {
        return m_PhyUnit;
    }

    void setRealValue(float realValue)
    {
    }

    void setValue(uint16_t value)
    {

        m_Value = value;
        if (m_Signed == false)
        {
            if (value > m_Max)
            {
                m_Value = m_Max;
            }
            else if (value < m_Min)
            {
                m_Value = m_Min;
            }
        }
        else
        {
            if (static_cast<int16_t>(value) > static_cast<int16_t>(m_Max))
            {
                m_Value = m_Max;
            }
            else if (static_cast<int16_t>(value) < static_cast<int16_t>(m_Min))
            {
                m_Value = m_Min;
            }
        }
    }

private:
    uint16_t m_Min;
    uint16_t m_Max;
    std::string m_Label;
    float m_Scale;
    std::string m_PhyUnit;
    bool m_Signed;
    RegisterType m_Type;
    uint16_t m_Value;
};


ModbusRegister holdingRegs[HOLDING_LAST_ITEM];

ModbusRegister inputRegs[INPUT_LAST_ITEM];

void init_holding_registers(void)
{

}
#endif

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
        return "AC Voltage [V] (TODO)";
    case e_ac_current:
        return "AC Current [10x A] (TODO)";
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
    case e_holding_last_item:
        return "e_holding_last_item";
    }

    return "unknown?";
};
