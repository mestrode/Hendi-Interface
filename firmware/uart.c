//uart.c
//for use with msg.c system
//fully driven by interrupt
//uses transmission and receive buffer
//--------------
//mestrode
//17.02.2018 translation
//03.01.2015
//--------------

#include "uart.h"

//calculate baud rate, depends on FCPU, baud rate and U2X
#ifndef UART_BAUD_SELECT
	#include "main.h" //if there is no F_CPU available

	#if _UART_UseU2X
		#define UART_BAUD_SELECT ((F_CPU+UART_BAUD_RATE*4)/(UART_BAUD_RATE*8)-1)   // clever runden
		#define UART_BAUD_REAL (F_CPU/(8*(UART_BAUD_SELECT+1)))     // Reale Baudrate
	#else
		#define UART_BAUD_SELECT ((F_CPU+UART_BAUD_RATE*8)/(UART_BAUD_RATE*16)-1)   // clever runden
		#define UART_BAUD_REAL (F_CPU/(16*(UART_BAUD_SELECT+1)))     // Reale Baudrate
	#endif

  #define BAUD_ERROR ((UART_BAUD_REAL*1000)/UART_BAUD_RATE) // Fehler in Promille, 1000 = kein Fehler.

  #if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
    #error Systematischer Fehler der Baudrate groesser 1% und damit zu hoch!
  #endif
#endif

//################
#ifdef _Use_RTS_CTS
//CTS: LOW = Ready, HIGH = Busy
	#define RecieverIsReady bit_is_clear(PINuartCTS, BITuartCTS)
//RTS: LOW = Ready, HIGH = Busy
	#define SetUartReady()  PORTuartRTS &= ~(1<<BITuartRTS)
	#define SetUartBusy()   PORTuartRTS |=  (1<<BITuartRTS)
#endif //_Use_RTS_CTS
//################

//###########################################################################
static unsigned char UART_TX_Buffer[UART_TX_BufferSize];
static unsigned char UART_TX_BufferRead = 0;
static volatile unsigned char UART_TX_BufferWrite = 0;
static unsigned char UART_RX_Buffer[UART_RX_BufferSize];
static volatile unsigned char UART_RX_BufferRead = 0;
static volatile unsigned char UART_RX_BufferWrite = 0;

//    Length = 16            0123456789012345
//    Content               (----------------)
//if w equal r then empty   (-----------r----)
//     w == r               (-----------w----)
#define UART_TX_Buffer_isempty (UART_TX_BufferWrite == UART_TX_BufferRead)
#define UART_RX_Buffer_isempty (UART_RX_BufferWrite == UART_RX_BufferRead)

//    Length = 16            0123456789012345
//    Content               (-----------abcd-)
//if w after r, then delta  (-----------r---w)
//     l = w - r                        |<>|

//    Length = 16            0123456789012345       0123456789012345 6789012
//    Content               (fghi-------abcde)     (fghi-------abcde fghi---
//if w bevore r, then add l (----w------r----) --> (----w------r----)----w--
//     l = ((w+s) - r ) % s  -->|       |<---       -->|       |<--- -->|
//#define UART_TX_Buffer_lev (((UART_TX_BufferWrite + UART_TX_BufferSize) - UART_TX_BufferRead) % UART_TX_BufferSize)
//#define UART_RX_Buffer_lev (((UART_RX_BufferWrite + UART_RX_BufferSize) - UART_RX_BufferRead) % UART_RX_BufferSize)

static inline uint8_t UART_TX_Buffer_lev (void)
{
	uint16_t temp;
	temp = ((UART_TX_BufferWrite + UART_TX_BufferSize) - UART_TX_BufferRead);
	return temp % UART_TX_BufferSize;
}

static inline uint8_t UART_RX_Buffer_lev (void)
{
	uint16_t temp;
	temp = ((UART_RX_BufferWrite + UART_RX_BufferSize) - UART_RX_BufferRead);
	return temp % UART_RX_BufferSize;
}

//    Length = 16            0123456789012345       0123456789012345
//    Content               (fghijklmno-abcde)     (abcdefghijklmno-)
//if w+1 == r, then full  (----------wr----) --> (r--------------w)
//     (w + 1) % s == r      --------->||<---       |<------------>|
#define UART_TX_Buffer_isfull ((UART_TX_BufferWrite + 1) % UART_TX_BufferSize == UART_TX_BufferRead)
#define UART_RX_Buffer_isfull ((UART_RX_BufferWrite + 1) % UART_RX_BufferSize == UART_RX_BufferRead)

//###########################################################################
//init

void UART_Init(void)
{
  //reset buffer pointers
  UART_TX_BufferRead = 0;
  UART_TX_BufferWrite = 0;
  UART_RX_BufferRead = 0;
  UART_RX_BufferWrite = 0;

  //Input
  DDRuartRx &= ~(1<<BITuartRx);
  //Tri-State (Hi-Z)
  PORTusrtRx &= ~(1<<BITuartRx);

  //Output
  DDRuartTx |=  (1<<BITuartTx);
  //Zero
  PORTusrtTx &= ~(1<<BITuartTx);

#if _Use_RTS_CTS
  //Input
  DDRuartCTS &= ~(1<<BITuartCTS);
  //Output
  DDRuartRTS |=  (1<<BITuartRTS);
#endif //_Use_RTS_CTS

  //Set Baud Rate
  UBRR0H = (unsigned char)UART_BAUD_SELECT>>8;
  UBRR0L = (unsigned char)UART_BAUD_SELECT;

#if _UART_UseU2X
  //Double Baud Rate
  UCSR0A = (1<<U2X0);
    // Double the USART Transmission Speed
#endif

  //Enable Int RX Complete, TX Complete, UDR Empty; Enable RX / TXUDRIE 5 10
  UCSR0B = (1<<RXCIE0)|(0<<TXCIE0)|(0<<UDRIE0)|(1<<RXEN0)|(1<<TXEN0);
    //RXCIE0 = RX Complete Interrupt Enable
    //TXCIE0 = TX Complete Interrupt Enable
    //UDRIE0 = Data register empty Interrupt Enable
    //RXEN0 = Receiver enable
    //TXEN0 = Transmission enable

  //Set Mode to Asynchron 8-N-1
  //NOTE: set URSEL bit, to reach access on UCRSRC
  UCSR0C = (0b00<<UMSEL00)|(1<<UCSZ01)|(1<<UCSZ00);
    //UMSEL0 = 0b00 = Asynchronouse USART
    //UMSEL0 = 0b01 = Synchronouse USART
    //UCSZ0  = 0b011 = 8-Bit Character Size
    //UPM0   = 0b00  = no parity bit
    //USBS   = 0b0   = 1 Stopbit

#if _Use_RTS_CTS
  SetUartReady();
#endif //_Use_RTS_CTS
}

//###########################################################################
//Transmission

//direct transmission of a single byte
/*int UART_SendByte(char Data, FILE* stream)
{
	//wait for USART if clear for transmission
	while (!( UCSRA & (1<<UDRE))) ;

#if _Use_RTS_CTS
	//wait for clear to receive
	while (!(RecieverIsReady)) ;
#endif //_Use_RTS_CTS

	//Transmit data
	UDR = Data;

	return 0;
}
*/

//---------------
//Transmission routine
//---------------

//include data into transmittion buffer
void UART_SendChar( const uint8_t c )
{
	//if Buffer is full, the wait until Timeout
	uint16_t cnt_timeout = 0;
	while (UART_TX_Buffer_isfull && (cnt_timeout < DataTimeToLiveMax)) {
		cnt_timeout++;
		//delay_us(1);
		asm volatile ("nop");
	}

	//if TX_Buffer if full, discard a single char and include the new one
	if (UART_TX_Buffer_isfull) {
		//--> Data will be lost!
		UART_TX_BufferRead = (UART_TX_BufferRead + 1) % UART_TX_BufferSize;
	}
	//Insert char in TxBuffer (Pointer is already prepared)
	UART_TX_Buffer[UART_TX_BufferWrite] = c;
	//Prepare the Tx-Write-Pointer for the next char
	UART_TX_BufferWrite = (UART_TX_BufferWrite + 1) % UART_TX_BufferSize;
	//UDR Empty Interrupt aktivieren
	UCSR0B |= (1<<UDRIE0);
}

//Interrupt: if UDR is empty, send next char out of Buffer
ISR(USART_UDRE_vect)
{
	//if Buffer is not Empty
	if (!UART_TX_Buffer_isempty)
	{
		//copy char into transmit register
		UDR0 = UART_TX_Buffer[UART_TX_BufferRead];
		//shift read pointer
		UART_TX_BufferRead = (UART_TX_BufferRead + 1) % UART_TX_BufferSize;
	} else {
		//UDR Empty Interrupt deaktivieren
		UCSR0B &= ~(1<<UDRIE0);
	}
}

//###########################################################################
//receive routine

//the receive process
ISR(USART_RX_vect)
{
	//last the current and last char
	static uint8_t ThisChar = 0x00;
	ThisChar = UDR0;

	//if buffer is full, the also shift the read-pointer
	//cause the read-pointer has always to lead the write pointer!
	if (UART_RX_Buffer_isfull)
	{

#if !_Use_RTS_CTS
		//--> Data will be lost!
		UART_RX_BufferRead = (UART_RX_BufferRead + 1) % UART_RX_BufferSize;
#else //_Use_RTS_CTS 
		//--> No Data will be lost
		SetUartBusy();
		//exit;
# warning "not ready yet!"
#endif 
	}

	//put new character into buffer
	UART_RX_Buffer[UART_RX_BufferWrite] = ThisChar;

	//shift the write-pointer
	UART_RX_BufferWrite = (UART_RX_BufferWrite + 1) % UART_RX_BufferSize;
}

void UART_ProgressRX( void )
{
	bool LastMsgComplete = true;
	//while more than 3 characters are available AND the last Message was complete
	//Preamble + length=0 + Checksum (if length==0: no payload included)
	while ((UART_RX_Buffer_lev() >= 3) & (LastMsgComplete))
	{
		//a temporary read pointer will be used
		uint8_t tempMsgReadPointer = UART_RX_BufferRead;
		//may there is a Message: check Preamble (thats not the MsgID!)
		switch (UART_RX_Buffer[UART_RX_BufferRead])
		{
			case UART_Rx_Preamble: // Message-Type: Preamble + ID + LEN + Data[LEN] + ChkSum received
			{
				//include Preamble also into ChkSum
				uint8_t Msg_ChkSum = 0;
				Msg_ChkSum ^= UART_RX_Buffer[tempMsgReadPointer]; //XOR Checksum
				//shift temporary pointer
				tempMsgReadPointer = (tempMsgReadPointer + 1) % UART_RX_BufferSize;

				//get LEN-Byte
				uint8_t MsgLEN = UART_RX_Buffer[tempMsgReadPointer];
				bool MsgFormalOK = (MsgLEN <= msg_len_max);
				Msg_ChkSum ^= MsgLEN; //XOR Checksum
				tempMsgReadPointer = (tempMsgReadPointer + 1) % UART_RX_BufferSize;

				//Message completely received? (ID, LEN, Data[1..LEN], ChkSum)
				if (UART_RX_Buffer_lev() < (1 + 1 + MsgLEN + 1))
				{
					//msg not complete in buffer
					LastMsgComplete = false;
				} else { //msg complete in buffer
					TMsg Msg;

					//copy every single Character of Payload one by one
					for (uint8_t i=0; i<MsgLEN; i++) {
						//copy character
						Msg.chr[i] = UART_RX_Buffer[tempMsgReadPointer];
						Msg_ChkSum ^= Msg.chr[i]; //XOR Checksum
						tempMsgReadPointer = (tempMsgReadPointer + 1) % UART_RX_BufferSize;
					}

					//get and check ChkSum
					uint8_t aMsgByte = UART_RX_Buffer[tempMsgReadPointer];
					tempMsgReadPointer = (tempMsgReadPointer + 1) % UART_RX_BufferSize;
					MsgFormalOK = MsgFormalOK & (Msg_ChkSum == aMsgByte);

					//if message valid?
					if (MsgFormalOK)
					{
						//hit event
						bool MsgDone = Msg_Do(&Msg);
						if (MsgDone)
						{
							//Message is complete and checked: could be removed form buffer
							//assume temporary Pointer
							UART_RX_BufferRead = tempMsgReadPointer;
						} else { //Message is invalid
							//Fehlermeldung ausgeben
							//							Output_Beep(32);
							//erstes Byte verwerfen, Lesepointer also nur um 1 shiften
							UART_RX_BufferRead = (UART_RX_BufferRead + 1) % UART_RX_BufferSize;
						} //Msg Failed
					} else {
						//!MsgFormalOK (length and ChkSum were checked)
						Msg_Send_ChkSumFail();
					}

				} //msg complete in buffer
				break;
			} //case Preamble = $5A

			default: { //Preamble not known: Error, remove a single byte
				//raise an Error
				//Output_Beep(32);
				//discard the first byte , shift read pointer by one
				UART_RX_BufferRead = (UART_RX_BufferRead + 1) % UART_RX_BufferSize;
			}  //default unknown ID
		} //switch (check ID)
	} //while there is at least one msg in the buffer (x characters)

} //UART_ProgressRX( void )
