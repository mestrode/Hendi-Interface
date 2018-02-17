//msg.c
//Hendi-Interface
//--------------
//mestrode
//06.01.2015
//--------------

//       80  Preamble
//        n  Count of Bits (n)
//  1   Adr  Adress Byte
//  2    ?   Content (n-1 Bytes)
//  ..   ?
//  n    ?
//      chk  Checksum (über alles von Preamble .. Content[n]
// (/r) (13) (Delimier_Pre)
//  /n   10  Delimiter

#include "msg.h"

//###########################################################################
//für ChkSum Berechnung

#ifdef Msg_Use_ChkSum
  static uint8_t Msg_ChkSum = 0;
#endif

void Msg_Send_Preamble( void );
void Msg_Send_Lenght( const uint8_t len );
void Msg_Send_CharChkSum( const uint8_t c );
void Msg_Send_ChkSum_And_Delim( void );

void Msg_Send_OK( void );
/*
void Msg_Send_CH_Byte(  const uint8_t ch, const uint8_t         data );
void Msg_Send_CH_Word(  const uint8_t ch, const uint16_t        data );
void Msg_Send_CH_Float( const uint8_t ch, const float           data );
void Msg_Send_CH_Time(  const uint8_t ch,       Tclock_timeset* data );
void Msg_Send_String(   const char* data );
*/

//------

bool Msg_Do( TMsg* Msg )
{
	bool MsgDone = true;
	uint8_t MsgID = (*Msg).chr[0];

	switch (MsgID)
	{

		case 0x48:  // "H" = PowerValue for High Power Logic Mode
		{
			uint16_t aVal = (*Msg).ui16.data;
			//check state and confirm value
			if (((Status == Status_RemotePassive) |
				(Status == Status_RemoteActiveOff) |
				(Status == Status_RemoteActiveOn)) &
				(/*(aVal >= 0) &*/ (aVal <= 0x3FF)))
			{
				RemoteLogicHighPowerValue = aVal;
				eeprom_update_word (&eeRemoteLogicHighPowerValue, RemoteLogicHighPowerValue);
				
				RemoteTimeoutReset();
				Msg_Send_RMHighPower();
			} else {
				RemotePowerValue = 0;
				RemoteTimeout = 0;
				Msg_Send_Fail();
			}
			break;
		}

		case 0x4C:  // "L" = PowerValue for Low Power Logic Mode
		{
			uint16_t aVal = (*Msg).ui16.data;
			//check state and confirm value
			if (((Status == Status_RemotePassive) |
				(Status == Status_RemoteActiveOff) |
				(Status == Status_RemoteActiveOn)) &
				(/*(aVal >= 0) &*/ (aVal <= 0x3FF)))
			{
				RemoteLogicLowPowerValue = aVal;
				eeprom_update_word (&eeRemoteLogicLowPowerValue, RemoteLogicLowPowerValue);
				
				RemoteTimeoutReset();
				Msg_Send_RMLowPower();
			} else {
				RemotePowerValue = 0;
				RemoteTimeout = 0;
				Msg_Send_Fail();
			}
			break;
		}

		case 0x50:  // "P" = PowerValue for direct use
		{
			uint16_t aVal = (*Msg).ui16.data;
			//check state and confirm value
			if (((Status == Status_RemotePassive) |
				(Status == Status_RemoteActiveOff) |
				(Status == Status_RemoteActiveOn)) &
				(/*(aVal >= 0) &*/ (aVal <= 0x3FF)))
			{
				RemotePowerValue = aVal;
				RemoteTimeoutReset();
				Msg_Send_OK();
			} else {
				RemotePowerValue = 0;
				RemoteTimeout = 0;
				Msg_Send_Fail();
			}
			break;
		}

		//(*Msg) ID not valid --> refect first byte
		default:
			MsgDone = false;
	} //switch MsgID

	return MsgDone;
}

//#######################################

inline void Msg_Send_Preamble (void)
{
  #ifdef Msg_Use_ChkSum
    Msg_ChkSum = 0;
  #endif
  
  #ifdef Msg_Preamble
    Msg_Send_CharChkSum( Msg_Preamble );
  #endif
}

inline void Msg_Send_Lenght ( const uint8_t len )
{
  #ifdef Msg_Use_Send_Length
    Msg_Send_CharChkSum( len );
  #endif
}

//insert Data into transmit-buffer and includ into ChkSum calculation
inline void Msg_Send_CharChkSum( const uint8_t c )
{
  #ifdef Msg_Use_ChkSum
    //include Data in ChkSum
//    Msg_ChkSum = (Msg_ChkSum<<1) + c;
    Msg_ChkSum ^= c;

    UART_SendChar( c );
  #endif
}

//send final character
inline void Msg_Send_ChkSum_And_Delim( void )
{
  #ifdef Msg_Use_ChkSum
    //ChkSum includes Header and Data
    UART_SendChar( Msg_ChkSum );
  #endif

  // rushing ahead a \r Delimiter, or just a single \n
  #ifdef Msg_Delim_Pre
    UART_SendChar( Msg_Delim_Pre );
  #endif

  // Send Delim
  UART_SendChar( Msg_Delim );
}

//#######################################

void Msg_Send_OK( void )
{
	//Header
	Msg_Send_Preamble();

	//count of data bytes
	Msg_Send_Lenght( 0x01 );

	//Payload: only channel
	Msg_Send_CharChkSum( 0x4B ); //"K"
	//End of Payload

	//complete Msg with ChkSum and Delimiter
	Msg_Send_ChkSum_And_Delim();
}

void Msg_Send_Fail( void )
{
	//Header
	Msg_Send_Preamble();

	//Count of data bytes
	Msg_Send_Lenght( 0x01 );

	//Payload: only channel
	Msg_Send_CharChkSum( 0x46 ); //"F"
	//End of Payload

	//complete Msg with ChkSum and Delimiter
	Msg_Send_ChkSum_And_Delim();
}

void Msg_Send_ChkSumFail( void )
{
	//Header
	Msg_Send_Preamble();

	//Count of Payload bytes
	Msg_Send_Lenght( 0x01 );

	//Payload: only channel
	Msg_Send_CharChkSum( 0x43 ); //"C"
	//End of Payload

	//complete Msg with ChkSum and Delimiter
	Msg_Send_ChkSum_And_Delim();
}

/*
void Msg_Send_CH_Byte( const uint8_t ch, const uint8_t data )
{
  //Header
  Msg_Send_Preamble();

	//Count of Payload bytes
  Msg_Send_Lenght( 0x02 );

	//Payload: channel, single Byte
  Msg_Send_CharChkSum( ch );
  Msg_Send_CharChkSum( data );
	//End of Payload

	//complete Msg with ChkSum and Delimiter
  Msg_Send_ChkSum_And_Delim();
}

void Msg_Send_CH_Word( const uint8_t ch, const uint16_t data )
{
  //Header
  Msg_Send_Preamble();

	//Count of Payload bytes
  Msg_Send_Lenght( 0x03 );

	//Payload: channel, High Byte, Low Byte
  Msg_Send_CharChkSum( ch );
  Msg_Send_CharChkSum( (char)(data>>8) );
  Msg_Send_CharChkSum( (char)(data&0xFF) );
	//End of Payload

	//complete Msg with ChkSum and Delimiter
  Msg_Send_ChkSum_And_Delim();
}

void Msg_Send_CH_Float( const uint8_t ch, const float data )
{
  union {
    float r;
    uint8_t i[sizeof(float)];
  } u;
  u.r = data;

  //Header
  Msg_Send_Preamble();

	//Count of Payload bytes
  Msg_Send_Lenght( 1 + sizeof(float) );

	//Payload: channel, High Byte, Low Byte
  Msg_Send_CharChkSum( ch );
  for (uint8_t i = 0; i < sizeof(float); i++) {
    Msg_Send_CharChkSum( (char)(u.i[i]) );
  }
	//End of Payload

	//complete Msg with ChkSum and Delimiter
  Msg_Send_ChkSum_And_Delim();
}

void Msg_Send_CH_Time( uint8_t ch, Tclock_timeset* data )
{
  //Header
  Msg_Send_Preamble();

	//Count of Payload bytes
  Msg_Send_Lenght( 0x05 );

	//Payload: channel, Hour, Minute, Sec, tick
  Msg_Send_CharChkSum( ch );
  Msg_Send_CharChkSum( (*data).hour );
  Msg_Send_CharChkSum( (*data).min );
  Msg_Send_CharChkSum( (*data).sec );
  Msg_Send_CharChkSum( Clock_tic );
  //Ende der DatenBytes

	//complete Msg with ChkSum and Delimiter
  Msg_Send_ChkSum_And_Delim();
}

void Msg_Send_String( const char *data )
{
  //Header
  Msg_Send_Preamble();

	//Count of Payload bytes
  Msg_Send_Lenght( 1 + strlen(data) );

  //ChannelID
  Msg_Send_CharChkSum( 0xF0 );

  //Payload: String
  while( *data != '\0' ) {
    Msg_Send_CharChkSum( *data++ );
  }

	//complete Msg with ChkSum and Delimiter
  Msg_Send_ChkSum_And_Delim();
}
*/
//#######################################

void Msg_Send_Version( const uint8_t SWVersion )
{
	//Header
	Msg_Send_Preamble();

	//Count of Payload bytes
	Msg_Send_Lenght( 2 );

	//Payload: Channel, ID, SWVersion
	Msg_Send_CharChkSum( 0x56 ); //"V"
	Msg_Send_CharChkSum( SWVersion );
	//End of Payload

	//complete Msg with ChkSum and Delimiter
	Msg_Send_ChkSum_And_Delim();
}

void Msg_Send_Input( const bool SWValue, const uint16_t PotiValue )
{
  //Header
  Msg_Send_Preamble();

	//Count of Payload bytes
  Msg_Send_Lenght( 4 );

  //Payload: Channel, ID, Payload, high, low Potvalue
  Msg_Send_CharChkSum( 0x49 ); //"I"
  Msg_Send_CharChkSum( SWValue );
  Msg_Send_CharChkSum( PotiValue>>8 );
  Msg_Send_CharChkSum( PotiValue & 0xFF );
	//End of Payload

	//complete Msg with ChkSum and Delimiter
  Msg_Send_ChkSum_And_Delim();
}

void Msg_Send_Status( const uint8_t Status, const uint16_t Power)
{
	//Header
	Msg_Send_Preamble();

	//Count of Payload bytes
	Msg_Send_Lenght( 4 );

	//Payload: ID, Status, High, Low Byte of Power
	Msg_Send_CharChkSum( 0x53 ); //"S"
	Msg_Send_CharChkSum( Status );
	Msg_Send_CharChkSum( Power>>8 );
	Msg_Send_CharChkSum( Power & 0xFF );
	//End of Payload

	//complete Msg with ChkSum and Delimiter
	Msg_Send_ChkSum_And_Delim();
}

void Msg_Send_RMHighPower( void )
{
	//Header
	Msg_Send_Preamble();

	//Count of Payload bytes
	Msg_Send_Lenght( 3 );

	//Payload: ID, high EEPvalue, High, Low Byte of Pot
	Msg_Send_CharChkSum( 0x48 ); //"H"
	uint16_t aVal = eeprom_read_word (&eeRemoteLogicHighPowerValue);
	Msg_Send_CharChkSum( aVal>>8 );
	Msg_Send_CharChkSum( aVal & 0xFF );
	//End of Payload

	//complete Msg with ChkSum and Delimiter
	Msg_Send_ChkSum_And_Delim();
}

void Msg_Send_RMLowPower( void )
{
	//Header
	Msg_Send_Preamble();

	//Count of Payload bytes
	Msg_Send_Lenght( 3 );

	//Payload: ID, low EEPvalue, High, Low Byte of Pot
	Msg_Send_CharChkSum( 0x4C ); //"L"
	uint16_t aVal = eeprom_read_word (&eeRemoteLogicLowPowerValue);
	Msg_Send_CharChkSum( aVal>>8 );
	Msg_Send_CharChkSum( aVal & 0xFF );
	//End of Payload

	//complete Msg with ChkSum and Delimiter
	Msg_Send_ChkSum_And_Delim();
}

void Msg_Send_Timeout( void )
{
	//Header
	Msg_Send_Preamble();

	//Count of Payload bytes
	Msg_Send_Lenght( 1 );

	//Payload: ID
	Msg_Send_CharChkSum( 0x54 ); //"T"
	//End of Payload

	//complete Msg with ChkSum and Delimiter
	Msg_Send_ChkSum_And_Delim();
}
//eof