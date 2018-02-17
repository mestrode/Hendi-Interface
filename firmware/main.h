//main.h
//Hendi-Interface
//--------------
//mestrode
//16.02.2018 translation
//17.09.2016 add Logic-Remote mode with High and Low Power
//05.01.2015 Initial
//--------------
#ifndef _main_h_
#define _main_h_

#ifdef DEBUG
	# warning "DEBUG mode defined"
#else
	# warning "PRODUCTIVE mode defined"
#endif

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#include "inout.h"
#include "clock.h"
#include "uart.h"
#include "msg.h"

#ifndef F_CPU
	#define F_CPU 1000000UL
	#warning "F_CPU manuel in main.h definiert!"
#endif

#define Main_SWVersion 0x02

enum StatusENUM { Status_Off = 0,
				  Status_RemoteLogicLowPowerOff = 6,
				  Status_RemoteLogicLowPowerOn = 7,
				  Status_RemoteLogicHighPowerOff = 8,
				  Status_RemoteLogicHighPowerOn = 9,
				  Status_RemotePassive = 1,
				  Status_RemoteActiveOff = 2,
				  Status_RemoteActiveOn = 3,
				  Status_Manual = 4 };

extern enum StatusENUM Status;

//Timeout counter, to avoid the HENDI is heating without control
extern uint8_t RemoteTimeout;
/* 40x 62.5ms = 2,562s */
#define RemoteTimeoutReset() RemoteTimeout = (40+1)

//Power, received by the UART
extern uint16_t RemotePowerValue;

//Power, fixed by high and low Power Logic Mode
// adjustes Power of induction cooker (including range check)
// 0x3FF = 3500W (Max-Value)
// 0xAA  = 1000W (requested by Udo)
// 0x0   =  500W (Min-Value)

extern uint16_t RemoteLogicHighPowerValue;
extern uint16_t RemoteLogicLowPowerValue;
#define eeRemoteLogicHighPowerValueDefault 0x3FF
#define eeRemoteLogicLowPowerValueDefault 0xF8
extern uint16_t eeRemoteLogicHighPowerValue; // = 0x3FF;
extern uint16_t eeRemoteLogicLowPowerValue; // = 0xAA = 800;

// left   = Low Power Logic Mode
// up     = High Power Logic Mode
// right  = Remote Mode
// max    = enable Manual Mode

#define Poti_Threshold_Off 16U
// below this threshold, the Logic Mode with low power will be active
// if the pot gets beyond this threshold, the Logic Mode with higher power gets active
/* 800W/3500W * 511*/
#define Poti_Threshold_LogicHighPower 150U
// if the pot gets beyond this threshold, the Remote Mode gets active
/*2000W/3500W * 511*/
#define Poti_Threshold_Remote 550U
// if the pot gets beyond this threshold, the Manual Mode get active until you full power off
/*3300W/3500W * 511*/
#define Poti_Threshold_Manual 900U

// hysteresis (will be used as substraction value)
#define Poti_Threshold_Hysteresis 25U

#endif //_main_h_