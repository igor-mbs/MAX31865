/*
 * Created	: 25/11/2021
 * Author	: Igor M B Silva
 * e-mail	: igor-mbs@outlook.com
 * GitHub	: Angry-Robot
 */ 
//
// MAX31865 with PT100 and 400 Ohms reference resistor
//
// -30 �C :   7227 bits
// 0 �C   :   8192 bits
// 200 �C :   14406 bits
//
// tArray[0]	= 7227 bits equivalent ADC register number	= -30 �C
// tArray[1012]	= 8192 bits equivalent ADC register number	= 0 �C
// tArray[7180]	= 14406 bits equivalent ADC register number	= 200 �C
//
// 7227 - 7227	= line 0	= -30 �C
// 8192 - 7227	= line 965	= 0 �C
// 14406 - 7227	= line 7179	= 200 �C
//
#ifndef LUT_H
#define LUT_H
	extern const float tArray[];
#endif