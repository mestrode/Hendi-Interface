//uart.h
//for use with msg.c
//fully interrupt driven
//uses transmission and receive buffer
//--------------
//mestrode
//17.02.2018 translation
//02.01.2015
//--------------

#ifndef _uart_h_
#define _uart_h_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>

#include "msg.h"

//uses hardware flow-control?
#define _Use_RTS_CTS false
#if _Use_RTS_CTS
#warning _Use_RTS_CTS is active!
#endif

//Transmission rate (baud rate)
//#define UART_BAUD_RATE  19200UL
//#define _UART_UseU2X false
#define UART_BAUD_RATE  9600UL
#define _UART_UseU2X	true

//Size of transmission and receive buffer
//TX uses a cycle-buffer (FiFo) can carry one character less than actual size!
#define UART_TX_BufferSize 64
#define UART_RX_BufferSize 64

//counter value, until data which couldn't send would be discarded
#define DataTimeToLiveMax 36000

//Message Type for receive
// 05A = "Z" = Messagetype: Preamble + ID + LEN + Data[LEN] + ChkSum
#define UART_Rx_Preamble 0x5A

//######################
//Hardware
//######################

#define DDRuartRx   DDRD
#define PORTusrtRx  PORTD
#define PINusrtRx  PIND
#define BITuartRx   0

#define DDRuartTx   DDRD
#define PORTusrtTx  PORTD
#define BITuartTx   1

#ifdef _Use_RTS_CTS
//Pin, to receive receiver is able to get data from us
#define DDRuartCTS  DDRD
#define PORTuartCTS PORTD
#define PINuartCTS  PIND
#define BITuartCTS  4

//Pin, to sign we ware able to receive data
#define DDRuartRTS  DDRD
#define PORTuartRTS PORTD
#define PINuartRTS  PIND
#define BITuartRTS  5

#endif //_Use_RTS_CTS

//######################
//interface

void UART_Init( void );

//transmission w/o interrupt
//int UART_SendByte(char Data, FILE* stream) // w/o Interrupt

// put data into the transmission buffer - may we lose data, if this is full!
void UART_SendChar( const uint8_t );         // w/  Interrupt

// handle data of receive buffer: will be checked and finally hit Msg_Do()
void UART_ProgressRX( void );                // w/  Interrupt

#endif //_uart_h_