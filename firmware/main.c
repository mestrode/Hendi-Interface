//main.c
//Hendi-Interface
//--------------
//mestrode
//16.02.2018 translations
//17.09.2016 add Logic-Remote mode with High and Low Power
//05.01.2015 initial release
//--------------

#include "main.h"

enum StatusENUM Status = Status_Off;

uint16_t RemoteLogicHighPowerValue;
uint16_t RemoteLogicLowPowerValue;
uint16_t EEMEM eeRemoteLogicHighPowerValue = eeRemoteLogicHighPowerValueDefault;
uint16_t EEMEM eeRemoteLogicLowPowerValue = eeRemoteLogicLowPowerValueDefault;


uint8_t RemoteTimeout = 0;
uint16_t RemotePowerValue = 0;

void DecRemoteTimeout ( void )
{
	//decrement Timeout
	if (RemoteTimeout > 0)
	{
		RemoteTimeout -= 1;
	}
	if (RemoteTimeout == 0)
	{
		Msg_Send_Timeout();
	}
}

int main(void)
{
	//activate watchdog: will be reset by a 1 sec cycle
	wdt_enable(WDTO_2S);

	//initialize GPIO, ADC-Converter, PWM (Timer1)
	InOut_Init();

	//initialize Clock - TIMER 2
	Clock_Init();

	sei();

	//initialize UART - Interrupt für RX und TX
	UART_Init();
	Msg_Send_Version( Main_SWVersion );

	//Initial values
	Status = Status_Off;
	uint16_t PotiValue = get_PotiValue();
	uint16_t PowerValue = 0;
	RemoteTimeout = 0;    // No Remote received
	RemotePowerValue = 0; // Off
	RemoteLogicLowPowerValue  = eeprom_read_word (&eeRemoteLogicLowPowerValue);
	RemoteLogicHighPowerValue = eeprom_read_word (&eeRemoteLogicHighPowerValue);

	Enable_Hendi();
	Disable_Hendi();

	while(1)
	{
		static uint8_t Clock_tic_old = clock_tic_max;
		if (Clock_tic != Clock_tic_old) {
			Clock_tic_old = Clock_tic;
			// cycle: 16 Hz = 62,5 ms

			//refresh Poti values
			PotiValue = get_PotiValue();
			//refresh remote value
			UART_ProgressRX();
			//refresh remote logic value
			refresh_DebounceValue();

			//check position of switch
			if (get_SW_IN_Value() )//& (PotiValue > Poti_Threshold_Off))
			{	//switch is ON: differentiate [Logic Low / Logic High / Remote / Manual]
				if ((Status == Status_Manual) | (Poti_Threshold_Manual < PotiValue))
				{
					// Manual Mode
					Status = Status_Manual;
				} else {
					
					if ((Poti_Threshold_Remote <= PotiValue) & (PotiValue < Poti_Threshold_Manual))
					{
						if (RemoteTimeout == 0)
						{	// REMOTE MODE (Passive, save condition)
							Status = Status_RemotePassive;
						} else { //REMOTE ACTIVE
							if (RemotePowerValue != 0)
							{			// HENDI is activated by remote
								Status = Status_RemoteActiveOn;
							} else {	// HENDI is disabled  by remote
								Status = Status_RemoteActiveOff;
							}
						}
					}
					if ((Poti_Threshold_LogicHighPower <= PotiValue) & (PotiValue < (Poti_Threshold_Remote-Poti_Threshold_Hysteresis)))
					{
						//LOGIC (digital on/off)
						//debouncing with a clock of 62.5 ms
						if get_RxPin_Value()
						{
							Status = Status_RemoteLogicHighPowerOn;
						} else {
							Status = Status_RemoteLogicHighPowerOff;
						}
					}
					if (PotiValue < (Poti_Threshold_LogicHighPower-Poti_Threshold_Hysteresis))
					{
						//LOGIC (digital on/off)
						//debouncing with a clock of 62.5 ms
						if get_RxPin_Value()
						{
							Status = Status_RemoteLogicLowPowerOn;
						} else {
							Status = Status_RemoteLogicLowPowerOff;
						}
					}
				} // if else: no manual mode
				
			} else { // SW_IN_Value (= switch) is off
				Status = Status_Off; // = Off
			} //if else get_SW_IN_Value()

			switch (Status) {
				case Status_RemoteLogicLowPowerOn:
					// HENDI is switched on by Logic-Level (low power mode)
					Enable_Hendi();
					PowerValue = RemoteLogicLowPowerValue;
					RemotePowerValue = RemoteLogicLowPowerValue;
					RemoteTimeout = 0;
					break;
				
				case Status_RemoteLogicHighPowerOn:
					// HENDI is switched on by Logic-Level (high power mode)
					Enable_Hendi();
					PowerValue = RemoteLogicHighPowerValue;
					RemotePowerValue = RemoteLogicHighPowerValue;
					RemoteTimeout = 0;
					break;
				
				case Status_RemoteActiveOn:
					// HENDI is activated by remote
					Enable_Hendi();
					PowerValue = RemotePowerValue;
					DecRemoteTimeout();
					break;
				case Status_RemoteActiveOff:
					// HENDI is switched off by remote
					Disable_Hendi();
					PowerValue = RemotePowerValue;
					DecRemoteTimeout();
					break;
				
				case Status_Manual:
					// HENDI activated by manual mode
					Enable_Hendi();
					PowerValue = PotiValue;
					RemotePowerValue = 0x0;
					RemoteTimeout = 0;
					break;

 				case Status_RemoteLogicLowPowerOff:		// HENDI is due to Logic-Level (Low Power) disabled
 				case Status_RemoteLogicHighPowerOff:	// HENDI is due to Logic-Level (High Power) disabled
 				case Status_RemotePassive:				// HENDI is due to missing link disabled
 				case Status_Off:						// HENDI is due to HW-switch disabled
				default:
					Disable_Hendi();
					PowerValue = 0x0;
					RemotePowerValue = 0x0;
					RemoteTimeout = 0;
				//break;
			}

			//pass PWM value to HW
			set_PowerValue(PowerValue);

			if ((Clock_tic % 4) == 0)
			{	//0.25s cycle
				switch (Status) {
					case Status_Manual: //Manual
						LED1_off();		// 1 = Off   = Manual
						LED2_off();     // 2 = Off   = just look transparent
						break;
						
					case Status_RemotePassive: // Remote passive, waiting for connection
						LED1_on();		// 1 = On    = Remote
						LED2_toggle();	// 2 = Blink = passive, no connection
						break;
					case Status_RemoteActiveOn: //Remote active, with Power
						LED1_on();		// 1 = On    = Remote
						LED2_on();		// 2 = On	 = Active (On)
						break;
					case Status_RemoteActiveOff: //Remote active, but switched off
						LED1_on();		// 1 = On    = Remote
						LED2_off();		// 2 = Off   = Active (Off)
						break;

					case Status_RemoteLogicHighPowerOn: // Remote Logic High Power On
						LED1_toggle();	// 1 = Blink Normal = Logic High Power
						LED2_on();		// 2 = On    = On (RemoteLogicPowerValue)
						break;
					case Status_RemoteLogicHighPowerOff: // Remote Logic High Power Off
						LED1_toggle();	// 1 = Blink Normal = Logic High Power
						LED2_off();		// 2 = Off   = Off
						break;
						
					case Status_RemoteLogicLowPowerOn: // Remote Logic Low Power On
						if ((Clock_tic) == 0)
						{
							LED1_toggle();	// 1 = Blink slow = Logic Low Power
						}
						LED2_on();		// 2 = On    = On (RemoteLogicPowerValue)
						break;
					case Status_RemoteLogicLowPowerOff: // Remote Logic Low Power Off
						if ((Clock_tic) == 0)
						{
							LED1_toggle();	// 1 = Blink slow = Logic Low Power
						}
						LED2_off();		// 2 = Off   = Off
						break;

					case Status_Off:	// Off
						LED1_off();		// 1 = Off   = Manual
						LED2_off();     // 2 = Off   = just look transparent
						break;

					default: //Error State
						LED1_toggle(); // Error State
						LED2_toggle(); // Error State
				}
			} // 0.25s Cycle

			if (Clock_tic == 0)
			{ // 1 sec cycle aligned to 0
				Clock_time_up(&Clock_time);
				//Watchdog: Reset by 1 sec cycle (2 sec is allowable)
				wdt_reset();
			} // 1 sec cycle

			if (Clock_tic == 2)
			{ // 1 sec cycle aligned to 2
				Msg_Send_Status(Status, PowerValue);
			} // 1 sec cycle

			if (Clock_tic == 10)
			{ // 1 sec cycle aligned to 10
				Msg_Send_Input(get_SW_IN_Value(), PotiValue);
			} // 1 sec cycle

		} //Takt: tic (OldTic != NewTic)

		//Sleep now
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();

	} //while(1)
} //main()