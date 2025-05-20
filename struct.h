#ifndef _STRUCT_H
#define _STRUCT_H

#ifdef DEVELOPER
#warning "Building in DEVELOPER mode !!"
#endif

#include <inttypes.h>

#define sbi(port, bit) ((port) |= (1 << bit))
#define cbi(port, bit) ((port) &= ~(1 << (bit)))
#define tbi(port, bit) (port ^= (1 << bit))
#define bis(port, bit) (port & (1 << (bit)))

#define ALARM_MASK       0  // 1
#define DEFROST_MASK     1  // 2
#define HEAT_MASK        2  // 4
#define COOL_MASK        3  // 8
#define IDLE_MASK        4  // 16
#define OIL_MASK         5  // 32
#define OPERATION_MASK   6  // 64
#define COMP             7  // 128
#define HOT_WATER_MASK   8  // 256
#define BIVALENT         9  // 512
#define BOOT             10  // 1024

enum operation_mode_t
{
    IDLE_MODE = 0,
    COOLING_MODE,
    HEATING_MODE,
    FANONLY_MODE,
    e_empty1,
    e_empty2,
    e_empty3,
    DEFROST_MODE,
};

typedef struct outdoorunit_t
{
    uint8_t compressor;
    uint8_t level;
    int16_t discharge_temp; // T5 Discharge temperature (outdoor)
    int16_t condenser_temp; // T3 condenser tempp (outdoor)
    int16_t ambient_temp; // T4 AmbientTemp (outdoor)
    int16_t indoor_temp; // T1 indoor temp   (indoor)  i T2 ta sama skala !!
    uint16_t eev;
    uint16_t eev2;
    uint8_t ac_voltage;
    uint8_t outdoor_mode;
    #ifdef gree
    int16_t inlet_temperature;
    int16_t ipm_module_temperature;
    #endif
} outdoorunit_t;

enum cwu_t
{
    cwu_fixed_comp = 0,
    cwu_fixed_temp,
    cwu_legacy,
};

enum command_t
{
    command_save = 1,
    command_reset,
    command_bootsel,
    command_default_settings,
};

#endif // _STRUCT_H