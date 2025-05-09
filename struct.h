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
#define OPERATION_MASK  6  // 64
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
    uint8_t level; //0-10V confirmation from outdoor unit
    int16_t discharge_temp; //T5 Discharge temperature (outdoor)
    int16_t condenser_temp; //T3 condenser tempp (outdoor)
    int16_t ambient_temp; //T4 AmbientTemp (outdoor)
    int16_t indoor_temp; //T1 indoor temp   (indoor)  i T2 ta sama skala !!
    //int16_t evaporator_temp; //T2 evaporator temp (indoor)
    uint16_t eev;
    uint16_t eev2;
    uint8_t ac_voltage;
    uint8_t outdoor_mode;
    #ifdef gree
    int16_t inlet_temperature;
    int16_t ipm_module_temperature;
    #endif
} outdoorunit_t;


/*

b11:
b11.0	F2	// T3 sensor error (tester e5||E52)
b11.1	F1	// T4 sensor error or EE error (tester e5||E53)	
b11.2	F3	// TP sensor error or EE error (tester e5||E54)		
b11.3	F6	// sensor error or EE error (tester e5)			
b11.4	P0	// IPM protect (tester P0)			
b11.5	?	// Temp. protect (tester P32) zdublowane
b11.6	P6	// Temp. protect (tester P32)
b11.7	P4	// Comp. protect (tester P4)

b12:
b12.0	?
b12.1	?		
b12.2	P1	// DC volt protect (tester P1) delayed		
b12.3	P2	// temp protect (tester P32)
b12.4	F4	// sensor or EE error (tester E5||E51)	
b12.5	F5	// ODU fan error (tester E7||E71)
b12.6	?
b12.7	F0	// current protect (tester P8)
*/
enum cwu_t
{
    cwu_fixed_comp = 0,
    cwu_fixed_temp,
    cwu_odu_ori,
};

enum faults_t
{
    fault_e0,
    fault_e1, // Communication malfunction between indoor and outdoor units  X
    fault_e3, // IDU fan error
    fault_e5, // Indoor temperature sensor (T1) malfunction  X
    fault_f0, // Overcurrent protection  X
    fault_f1, // Ambient temperature sensor (T4) malfunction  X
    fault_f2, // Outdoor heat-exchanger temperature sensor (T3) malfunction X
    fault_f3, // Discharge temperature sensor (T5) malfunction X
    fault_f4, // Outdoor EEPROM malfunction X
    fault_f5, // Outdoor fan speed malfunction X
    fault_f6, // T2b sensor outdoor unit malfunction X
    fault_f7, // Heat exchanger temperature alarm
    fault_p0, // IPM module protection  X
    fault_p1, // DC voltage too high/too low protection  X
    fault_p2,  // High compressor temp (T5)
    fault_p3, // Ultra-low ambient temperature protection  X
    fault_p4, // compressor rotor position protection   X
    fault_p6, // Low pressure compressor protection   X
    fault_last_item,                                                                                                                                                                                            

//    fault_p7, // IGBT sensor malfunction
//    fault_j0, // Evaporator high temperature protection
//    fault_j1, // Condenser high temperature protection
//    fault_j2, // High discharge temperature protection
//    fault_j3, // PFC module protection
//    fault_j4, // Communication errror between outdoor main chip and compressor driven chip
//    fault_j5, // High pressure protection
//    fault_j6, // Low pressure protection
//    fault_j8, // AC power input voltage protection

};

/* REVIO

Going to BOOTSEL mode
Handshake...
LNS_ODU: [ aa 00 01 00 a1 00 00 00 6b 69 1d 00 00 00 00 40 2d 55 ]
auxtemp=0
Handshake...[ OK ]
LNS_IDU: [ a5 00 00 00 00 00 00 00 00 00 3c 02 00 00 00 00 c2 5a ]
Modbus RTU slave started with slave ID: 53, baudrate: 9600
Echo test 1... 
LNS_ODU: [ a5 01 00 00 00 00 00 00 00 00 3c 02 00 00 00 00 c1 5a ]
auxtemp=0
Echo test 1... [ OK ]
LNS_IDU: [ a5 00 0d 00 00 00 00 00 00 00 00 00 00 00 00 00 f3 5a ]
Echo test 2... 
LNS_ODU: [ a5 01 0d 00 00 00 00 00 00 00 00 00 00 00 00 00 f2 5a ]
auxtemp=0
Echo test 2... [ OK ]
LNS_IDU: [ a5 00 1a 00 00 00 00 00 00 00 00 00 00 00 00 00 e6 5a ]
Echo test 3... 
LNS_ODU: [ a5 01 1a 00 00 00 00 00 00 00 00 00 00 00 00 00 e5 5a ]
auxtemp=0
Echo test 3... [ OK ]
LNS_IDU: [ a5 00 27 00 00 00 00 00 00 00 00 00 00 00 00 00 d9 5a ]
Echo test 4... 
LNS_ODU: [ a5 01 27 00 00 00 00 00 00 00 00 00 00 00 00 00 d8 5a ]
auxtemp=0
Echo test 4... [ OK ]
LNS_IDU: [ a5 00 34 00 00 00 00 00 00 00 00 00 00 00 00 00 cc 5a ]
Echo test 5... 
LNS_ODU: [ a5 01 34 00 00 00 00 00 00 00 00 00 00 00 00 00 cb 5a ]
auxtemp=0
Echo test 5... [ OK ]
LNS_IDU: [ a5 00 41 00 00 00 00 00 00 00 00 00 00 00 00 00 bf 5a ]
LNS_ODU: [ a5 01 41 00 00 00 00 00 00 00 00 00 00 00 00 00 be 5a ]

*/

#endif // _STRUCT_H