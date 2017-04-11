
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __DECODERSTATEMACHINE__
#define __DECODERSTATEMACHINE__

	#include "stdint.h"

#define INPUTAUDIOPIN (1<<PB3) //
#define PINVALUE (PINB&INPUTAUDIOPIN)
#define INITAUDIOPORT {DDRB&=~INPUTAUDIOPIN;} // audio pin is input

#define PINLOW (PINVALUE==0)
#define PINHIGH (!PINLOW)

#define LEDPORT    ( 1<<PB0 ); //PB1 pin 6 Attiny85
#define INITLED    { DDRB|=LEDPORT; }

#define LEDON      { PORTB|=LEDPORT;}
#define LEDOFF     { PORTB&=~LEDPORT;}
#define TOGGLELED  { PORTB^=LEDPORT;}

	#define FRAMESIZE 2 // number of bytes
	#define SYNCCOUNTER 9 // minimum number of bits which should have equal duration

	#define VOID 0
	#define START 1

	// state machine 1 state definitons
	#define M1_READY   0
	#define M1_PINLOW  1	// pin low state from machine 1
	#define M1_PINHIGH 2	// pin high state from machine 1

	typedef uint8_t state_t;
	typedef uint8_t command_t;

	enum bitState { SENDERBITREADY, BITREADY,BITSTART,BITEND };
	extern uint8_t BitValue;

	#define BITOK 0
	#define BITTIMEOUT 1
	#define TOSHORT 2

	extern uint8_t BitError;

	state_t BrEstimationStateMachine(command_t command);
	enum bitState highBitReceived_S(void);

	#define BYTEREADY 0
	#define WAITFORSTARTBIT 1
	#define RECEIVEBITS 2

	extern uint8_t ReceiverData;
	uint8_t receiveByte_S(void);

	#define FRAMEOK 0
	#define FRAMETIMEOUT 1

	#define FRAMEREADY          0
	#define BITRATEESTIMATION   1
	#define RECEIVEBYTES        2
	#define DATARECEIVED        3

  extern volatile uint8_t FrameData[FRAMESIZE];
  extern uint16_t DecoderData;  
  extern uint8_t FrameError;

  void decoderBegin();
  uint8_t isDataAvailable();
	uint8_t receiveFrame_S(void);

#endif // __DECODERSTATEMACHINE__

#ifdef __cplusplus
}
#endif
