//msg.h
//Hendi-Interface
//--------------
//mestrode
//16.02.2018 header inserted
//06.01.2015
//--------------

#ifndef _msg_h_
#define _msg_h_

#include "main.h"
#include "uart.h"

#define msg_len_max 8

#define Msg_Preamble 0x5A
#define Msg_Use_Send_Length true
#define Msg_Use_ChkSum true
#define Msg_Delim_Pre '\r'
#define Msg_Delim '\n'

typedef union {
            uint8_t chr[msg_len_max];
            struct {
              uint8_t ch;
              uint8_t data;
              uint8_t data2;
              uint8_t data3;
              uint8_t data4;
              uint8_t data5;
              uint8_t data6;
              uint8_t data7;
            } ui8;
            struct {
              uint8_t ch;
              uint16_t data;
              uint16_t data2;
              uint16_t data3;
              uint8_t data4;
            } ui16;
            struct {
              uint8_t ch;
              int16_t data;
              int16_t data2;
              int16_t data3;
              int8_t data4;
            } i16;
            struct {
              uint8_t ch;
              union {
                uint8_t bytes[sizeof(float)];
                float data;
              } flt;
              uint8_t data1;
              uint8_t data2;
              uint8_t data3;
            } flt;
          } TMsg;

extern void Msg_SendCycle( void );

extern bool Msg_Do( TMsg* Msg );

extern void Msg_Send_OK( void );                   //"K"
extern void Msg_Send_Fail( void );                 //"F"
extern void Msg_Send_ChkSumFail( void );           //"C"
/*
extern void Msg_Send_CH_Byte( uint8_t ch, uint8_t data );
extern void Msg_Send_CH_Word( uint8_t ch, uint16_t data );
extern void Msg_Send_CH_Float( uint8_t ch, float data );
extern void Msg_Send_CH_Time( uint8_t ch, Tclock_timeset* data );
extern void Msg_Send_String( const char *data );
*/
extern void Msg_Send_Version( uint8_t );           //"V"
extern void Msg_Send_Input( bool, uint16_t );      //"I"
extern void Msg_Send_Status( uint8_t, uint16_t );  //"S"
extern void Msg_Send_RMHighPower( void );          //"H"
extern void Msg_Send_RMLowPower( void );           //"L"
extern void Msg_Send_Timeout( void );              //"T"

#endif //_msg_h_