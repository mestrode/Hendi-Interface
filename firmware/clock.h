//clock.h
//Hendi-Interface
//--------------
//mestrode
//16.02.2018 translations
//02.01.2015
//--------------

#ifndef _clock_h_
#define _clock_h_

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// 1.000.000 Hz /1024 /61  /16 = 1,000576332 Hz

// 1.000.000 Hz /1024 /244 /4  = 1,000576332 Hz
//Crystal / Timer clock / clock HW max / tick max

// F_CPU                1MHz  Systemclock
// Timer Prescaler   1024	/*via TCCR2:WGM2 adjusted -> 976,5625 Hz*/
#define clock_HW_max   61	/*HW-Timer  -> 16 Hz*/
#define clock_tic_max  16	/*SW:tictoc ->  1 Hz*/
#define clock_sec_max  60	/*SW:Second*/
#define clock_min_max  60	/*SW:Minute*/
#define clock_hour_max 24	/*SW:Hour*/


typedef struct {
                  unsigned short hour;
                  unsigned short min;
                  unsigned short sec;
			   } Tclock_timeset;

extern Tclock_timeset Clock_time;
extern uint8_t Clock_tic;

extern void Clock_Init(void);
extern void Clock_tic_up( void );

extern void Clock_time_up( Tclock_timeset* );

#endif //_clock_h_