//inout.c
//Hendi-Interface
//--------------
//mestrode
//16.02.2018 translations
//17.09.2016 get_RxPin: Key-debouncing by execution per cycle
//03.01.2015
//--------------
#include "inout.h"
#include "uart.h"

uint16_t ADCval_Poti      = 0;
uint16_t ADCval_PotiFlt   = 0;

// pass PWM value
//OutputComparePin works inverse!  --> 0x000 = 5V =  500W
//                                     0x3FF = 0V = 3500W
#define set_PWM_Value(aVal)		OCR1B = aVal

inline uint16_t ADC_Read( const uint8_t channel );
inline void set_PowerValue( uint16_t Power );

void InOut_Init( void )
{
	//disable analog comparator
	ACSR |= (1<<ACD);

	//--Switch IN
	//Input
	DDR_SW_IN &= ~(1<<Bit_SW_IN);
	//Tri-State (Hi-Z)
	PORT_SW_IN &= ~(1<<Bit_SW_IN);

	//--Poti IN (ADC)
	//Input
	DDR_ADC &= ~(1<<Bit_ADC_Vin);
	//Tri-State (Hi-Z)
	PORT_ADC &= ~(1<<Bit_ADC_Vin);

	//unlock global pull-ups
	//MCUCR |=  (1<<PUD);

	//activate ADC
	ADCSRA |= (1<<ADEN) | (0b100<<ADPS0);
	// ADEN = ADC activate
	// ADC Frequency between 50kHz and 200kHz --> 1000 / [50..200] = [20..5]
	// ADP0 = 0b100 = 16
	ADMUX |= (0b01<<REFS0);
	// REFS0 = 0b01 = Reference = AVcc with external C at AREF pin

	//initialize ADC values and filter algorithm
	for (uint8_t i=0; i<=8; i++) {
		get_PotiValue();
	}
	
	//# Output ##############################################
	
	//--Switch OUT
	//Output
	DDR_SW_OUT |= (1<<Bit_SW_OUT);
	//High = PORT_SW_OUT |= (1<<Bit_SW_OUT);
	Disable_Hendi();

	//--Analog OUT = PWM
	//Output
	//set OC1B to outputs
	DDR_PWM |= (1<<Bit_PWM);
	
	//Define std value = High Voltage at Kl7, but PD5 works inverted due to circuit behavior
//	set_PWM_Value(0x0000);
	set_PowerValue(0x000);

	//reset Timer 1 counter
	TCNT1 = 0;

	//Timer 1 for PWM an OC1B
	//TIMSK1 |= (1<<TOIE0);
	//No Interrupt, runs full in HW

	//10-bit FastPWM Mode, direct to OC1B Pin, with max frequency,
	TCCR1A |= ((0b11<<COM1B0) | (1<<WGM11) | (1<<WGM10));
	//COM1B = 0b11   = Set OC1B on compare match (FastPWM Mode) (Inverse!)
	//WGM1  = 0b0111 = Fast PWM 10bit (bits are to write in two steps, due to their placement in two registers!)

	//  TCCR1B |= ((0<<WGM12) | (0b011<<CS00));
	// warning 'timer 1 disabled'
	TCCR1B |= ((0<<WGM13) | (1<<WGM12) | (0b001<<CS10));
	//WGM1 Rest (WGM1[3:2])
	//CS1   = 0b001  = clock / 1 / 10bit--> 1MHz / 1 / 1024 = 976Hz

	//--LED OUT
	//Output
	DDR_LED |= (1<<Bit_LED1);
	DDR_LED |= (1<<Bit_LED2);
	DDR_LED |= (1<<Bit_LED3);
	//High = LED off
//	PORT_SW_OUT |= (1<<Bit_LED1);
//	PORT_SW_OUT |= (1<<Bit_LED2);
//	PORT_SW_OUT |= (1<<Bit_LED3);
	LED1_off();
	LED2_off();
	LED3_off();
}

/* ADC single run */
inline uint16_t ADC_Read( const uint8_t channel )
{
	// choose channel, without touch other Bits
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	ADCSRA |= (1<<ADEN) | (1<<ADSC);
	//ADEN = enable ADC
	//ADSC = single conversion
//	#ifdef DEBUG
//	const uint16_t SimValue = 512;
//	return SimValue;
//	#else
	// wait for data for readout
	while (ADCSRA & (1<<ADSC) ) {}
	// read value of ADC and return
	return ADCW;
//	#endif
}

//cyclebuffer - mean-value-filter
//by addition additional bits are created
//--> Output will be downscaled to initial bit-width
// input  max 0x03FF
// output max 0x03FF
static inline uint16_t RingBuffer_16 (const uint16_t NewVal)
{
	#define InputBits         10
	#define OutputBits        10
	#define FilterBits         3

	#define FilterItems       (1<<FilterBits) // == 16
    // take count of items also while initialisation in account!
	static uint16_t Arr[FilterItems] = {0, 0, 0, 0, 0, 0, 0, 0};
	static uint16_t ArrSum = 0;
	static uint8_t i = 0;

	//use next buffer line:
	i = (i+1) % FilterItems;
	//sub old value from sum
	ArrSum -= Arr[i];
	//insert new value
	Arr[i]  = NewVal;
	//add new value to sum
	ArrSum += Arr[i];
	//sum results into ArrSum = [0x000..0xFFF0] - adjust now!
	return (ArrSum >> FilterBits);
}

//read values of GPIO, ADC and switch output and DAC
uint16_t get_PotiValue ( void )
{
	//read ADC-value (10bit)
	ADCval_Poti = ADC_Read(ADC_Ch_Poti);
	//filter now!
	ADCval_PotiFlt = RingBuffer_16(ADCval_Poti);
	//use inverse value: Power should be pos. proportional to value
	return (0x3FF-ADCval_PotiFlt);
}

// set Power Value of HENDI (includes Range Check of Value)
inline void set_PowerValue( uint16_t Power )
{
	//if Power is out of range, use lowest value
	if (Power > 0x3FF)
	{
		Power = 0x0; // 5V = 500W
	}
	set_PWM_Value(Power);
}

uint8_t button_state = 0;
uint8_t button_down = 0;

inline void refresh_DebounceValue ( void )
{
	// RX Pin will be covered by Bit 0
	uint8_t BUTTON_PIN = (PINusrtRx & (1<<BITuartRx));
	
	// (may Invert logic like: uint8_t state_changed = ~BUTTON_PIN ^ button_state;)
	// detect changed bits
	uint8_t state_changed = BUTTON_PIN ^ button_state;
	
	// initiate vertical counter variables to 11 (due to decrease count)
	static uint8_t vcount_low = 0xFF;
	static uint8_t vcount_high = 0xFF;
	
	// Decrease vertical counters where state_changed has a bit set (1)
	// and set the others to 11 (reset state)
	vcount_low = ~(vcount_low & state_changed);
	vcount_high = vcount_low ^ (vcount_high & state_changed);
	
	// Update state_changed to have a 1 only if the counter overflowed
	state_changed &= vcount_high & vcount_low;
	
	// Change button_state for the buttons who's counters rolled over
	button_state ^= state_changed;
	
	// Update button_down with buttons who's counters rolled over
	// and who's state is 1 (pressed)
	button_down |= button_state & state_changed;
}

//eof