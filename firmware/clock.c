//clock.c
//Hendi-Interface
//--------------
//mestrode
//16.02.2018 translations
//02.01.2015
//--------------

#include "clock.h"

Tclock_timeset Clock_time = {0,0,0};
uint8_t Clock_tic = 0;

void Clock_Init(void)
{
	//Timer 2 used for clock
	TIMSK2 |= (1<<OCIE2A);
	//OCIE2A = Timer 2 Compare Match Interrupt
	// Value of OCIE2A is included: [0..ORC2A]
	OCR2A = clock_HW_max-1;
	//CTC mode
	TCCR2A |= ((1<<WGM21) | (0<<WGM20));
	//WGM2  = 0b010 = CTC Model (Overflow bei OCR2A
	TCCR2B |= ((0<<WGM22) | (0b111<<CS20));
	//WGM2 reste
	//CS0   = 0b111  = clock / 1024 / OCR2A --> 1MHz / 1024 / 61 = 16,0092 Hz
}

ISR(TIMER2_COMPA_vect)
{
  Clock_tic_up();
}

inline void Clock_tic_up( void )
{
  Clock_tic = (Clock_tic + 1) % clock_tic_max;
}

void Clock_time_up( Tclock_timeset *time )
{
  (*time).sec++;
  if ((*time).sec >= clock_sec_max) { // 0 up to 59
    (*time).sec = 0;

    (*time).min++;
    if ((*time).min >= clock_min_max) { // 0 up to 59
      (*time).min = 0;

      (*time).hour++;
      if ((*time).hour >= clock_hour_max) { // 0 up to 23
        (*time).hour = 0;
      }
    }
  }
}

//eof
