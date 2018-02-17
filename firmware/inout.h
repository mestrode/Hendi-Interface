//inout.h
//Hendi-Interface
//--------------
//mestrode
//16.02.2018 translations
//17.09.2016 get_RxPin: Tastenentprellung durch zyklischen Aufruf
//02.01.2015
//--------------
#ifndef _inout_h_
#define _inout_h_

#include <stdbool.h>
#include <avr/io.h>

//-------switch input = switching contact of pot ------------------
#define Bit_SW_IN		1

#define DDR_SW_IN		DDRC
#define PORT_SW_IN		PORTC
#define PIN_SW_IN		PINC
//-------analog input = analog contact of pot ------------------
#define Bit_ADC_Vin		2

#define ADC_Ch_Poti		2

#define DDR_ADC			DDRC
#define PORT_ADC		PORTC
#define PIN_ADC			PINC
//-------switch output = switch in direction of display------------------
#define Bit_SW_OUT		0

#define PIN_SW_OUT		PINC
#define PORT_SW_OUT		PORTC
#define DDR_SW_OUT		DDRC
//-------analog output = signal for powerelectronic in direction of display------------------
#define Bit_PWM			2 	/* = OC1B von Timer 0*/

#define PIN_PWM			PINB
#define PORT_PWM		PORTB
#define DDR_PWM			DDRB
//-------unused GPIOs for LEDs------------------
#define Bit_LED1		7
#define Bit_LED2		6
#define Bit_LED3		5

#define PIN_LED			PIND
#define PORT_LED		PORTD
#define DDR_LED			DDRD
//-------------------------

extern void InOut_Init ( void );

//###Schalter################
// On  5.0V HIGH
// Off 0.0V LOW

//extern inline bool SW_Read ( void ); MACRO
//#ifdef DEBUG
//	#define get_SW_IN_Value() (0xFF & (1<<Bit_SW_IN))
//#else
//	#define get_SW_IN_Value() 0xFF
	#define get_SW_IN_Value() (PIN_SW_IN & (1<<Bit_SW_IN))
//#endif

// Enable:  Gate gets low, P-channel FET should be conduct
#define Enable_Hendi()    PORT_SW_OUT &= ~(1 << Bit_SW_OUT)
// Disable: Gate gets HIGH, P-channel FET should be suspend
#define Disable_Hendi()   PORT_SW_OUT |=  (1 << Bit_SW_OUT)

//###Pot################
//Voltage invers:
//  500W 5.0V			0x1
// 2000W 2.5V			0x1ff
// 3500W 0.0V			0x3ff

// get_PotiValue() is corrected --> 0x0 = Off, 0x3ff = 3500W 
extern uint16_t get_PotiValue ( void );

// adjust power of Hendi (with range check)
// 0x000 =  500W (Minimalwert)
// 0x3FF = 3500W (Maximalwert)
extern void set_PowerValue ( uint16_t Power );

//###LEDs for display of status#############
// LED is connected to +5V --> inverse Logic --> Low = LED is ON
#define LED1_on()		PORT_LED &= ~(1<<Bit_LED1)  // LED ON
#define LED1_off()		PORT_LED |=  (1<<Bit_LED1)  // LED OFF
#define LED1_toggle()	PORT_LED ^=  (1<<Bit_LED1)  // LED toggle

#define LED2_on()		PORT_LED &= ~(1<<Bit_LED2)  // LED ON
#define LED2_off()		PORT_LED |=  (1<<Bit_LED2)  // LED OFF
#define LED2_toggle()	PORT_LED ^=  (1<<Bit_LED2)  // LED toggle

#define LED3_on()		PORT_LED &= ~(1<<Bit_LED3)  // LED ON
#define LED3_off()		PORT_LED |=  (1<<Bit_LED3)  // LED OFF

//###Debounce System#############
extern uint8_t button_state;
extern uint8_t button_down;

#define get_RxPin_Value() (button_state  & 0x01)

extern void refresh_DebounceValue ( void );

#endif //_inout_h_