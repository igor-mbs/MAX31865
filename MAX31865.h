/*
 * Created  : 25/11/2021
 * Author	: Igor M B Silva
 * e-mail	: igor-mbs@outlook.com
 * GitHub	: Angry-Robot
 */ 

#include "../MAX31865_port.h"

#ifndef _MAX31865_H
    #define _MAX31865_H

    //---------------// Functions //-----------------------------------//
    STATUS_ENUM max31865_read(TEMPERATURE_STRUCT temperature);
    STATUS_ENUM max31865_start(MODE_ENUM operation);

    //---------------// Custom types and declarations //---------------//
    typedef enum {
        OK,
        OLD_READING,
        ERROR_PRECONFIGURING,
        ERROR_CONFIGURING,
        BROKEN_RTD_WIRE,
        CIRCUIT_MALFUNCTION,
        UNKNOWN_ERROR,
        WIRE_INPUT_ERROR,
        HZ_INPUT_ERROR,
        MODE_INPUT_ERROR
    }STATUS_ENUM;

    typedef enum {
        ONESHOT,
        AUTOMATIC
    }MODE_ENUM;

    typedef struct {
        F64 celsius;
        F64 kelvin;
        F64 fahrenheit;
    }TEMPERATURE_STRUCT;

    typedef union { 
        struct {
            U8 HZFILTER	    :1;     // 0	:	50/60Hz				:	Noise rejection filter frequency select (do not change frequency while in auto reading mode) (0: 60Hz, 1: 50Hz)
            U8 FAULTCLR	    :1;	    // 1	:	Fault status clear	:	Write a 1 to this bit while writing 0 to bits D5, D3, and D2 to return all fault status bits (D[7:2]) in the "Fault Status Register" to 0.
            U8 FAULTCYCLE   :2;	    // 2-3	:	Fault detection		:	Confusing AF, go read the data sheet : https://datasheets.maximintegrated.com/en/ds/MAX31865.pdf
            U8 WIRE		    :1;	    // 4	:	Three wire mode		:	Changes the RTD wiring mode (1: three wires mode, 0: two or four wires mode)
            U8 ONESHOT	    :1;	    // 5	:	One shot conversion	:	Reads sensor a single time, after a pulse on CS pin (requires "CONVMODE" off and "VBIAS" on; each conversion takes about 53ms in 60Hz and 63ms in 50HZ) (1: on, 0: off)
            U8 CONVMODE	    :1;	    // 6	:	Conversion mode		:	Turn on/off continuous reading at 50/60Hz rate (1: on, 0: off)
            U8 VBIAS	    :1;	    // 7	:	Voltage bias		:	Turn on/off voltage bias on the platinum thermistor (requires 11 time constants of the RC net + 1ms before starting the next reading) (1: on, 0: off)
        } bit;					    // Structure used for bit access
        U8 reg;					    // Type used for register access
    }MAX31865CFG;

    typedef union { 
        struct {
            U16 faultFlag	:1;	    // FaultFlag
            U16 rtd		    :15;    // Actual value
        } val;
        U16 reg;				    // Type used for register manipulation
    }RTDREG;

#endif