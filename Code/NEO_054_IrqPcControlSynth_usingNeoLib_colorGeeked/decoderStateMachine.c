/************************************************************************************

  data decoder for FSK realized as state machine

*************************************************************************************

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

********************************** list of outhors **********************************

  v0.1   x-1.2014 C. -H-A-B-E-R-E-R-  initial version for debug led receiver
  v0.2   6.3.2017 C. -H-A-B-E-R-E-R-  addapted for audio line
  v0.2  21.3.2017 C. -H-A-B-E-R-E-R-  decoderBegin routine added
    
  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something
  2017 ChrisMicro
  
*************************************************************************************
*/

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "neoSoundPittix.h" // needs the timer from there
#include "decoderStateMachine.h"

#define true 1==1
#define false 1!=1

uint8_t BitTimeHigh;
uint8_t BitTimeLow;
uint8_t HighTakesLonger;

volatile uint8_t error=false;

volatile uint8_t dataAvailableFlag = false;
volatile uint16_t InterruptData;
uint16_t DecoderData;

uint8_t isDataAvailable()
{
  uint8_t value;
  cli();
  value = dataAvailableFlag;
  dataAvailableFlag = false;
  DecoderData = InterruptData;
  sei();
  return value;
}

ISR(PCINT0_vect)
{
  //LEDOFF;
  //TOGGLELED;
  if (receiveFrame_S() == DATARECEIVED)
  {
    dataAvailableFlag = true;
    InterruptData = FrameData[1] + (((uint16_t)FrameData[0]) << 8);
    //InterruptData=FrameData[1];

    //TOGGLELED;
  }
  //LEDON;
}

void decoderBegin()
{
  INITAUDIOPORT;

  // pin change interrupts on Attiny85
  // https://thewanderingengineer.com/2014/08/11/pin-change-interrupts-on-attiny85/
  GIMSK = 0b00100000;    // turns on pin change interrupts
  // PCMSK bitmask: none none PCINT5 PCINT4 PCINT3 PCINT2 PCINT1 PCINT0
  PCMSK = 0b00001000;    // turn on interrupts on pins PB5
  sei();                 // enables interrupts
}

/***************************************************************************************

	state_t BrEstimationStateMachine(command_t command)

    Detect preample and measure high and low signal durations with this state machine

	input: command
	1. start the statemachine with (BrEstimationStateMachine(START)
	2. cylic call BrEstimationStateMachine(VOID) until result state is M1_READY

	output: 	state ( M1_READY when preample detected )

	global variables output
				uint8_t BitTimeLow
				uint8_t BitTimeHigh
				uint8_t HighTakesLonger
				
	BitTimeLow and BitTimeHigh should be almost the same if the signal is symmetric.
	Due to some electrical reasons it might be none symmetric,
	then the high and low time differ.			

***************************************************************************************/
state_t BrEstimationStateMachine(command_t command)
{
	static state_t state=M1_READY;
	static uint16_t counter=0,t_old=0;
	uint8_t tolerance;

	switch(state)
	{
		case M1_READY:{

			if(command==START)
			{
 				counter=0;
				// we don't know the pin state but
 				// we assume it is low to start the state machine
				state=M1_PINLOW;						// ======> switch to low state
			}
		}break;

		case M1_PINLOW:{

			if(PINHIGH)
			{
				BitTimeLow=SoftwareTimer_uint8;
				SoftwareTimer_uint8=0; // reset SoftwareTimer_uint8

				state=M1_PINHIGH; 						// ======> switch to high state
			}
		}break;

		case M1_PINHIGH:{

			if(PINLOW)
			{
				BitTimeHigh=SoftwareTimer_uint8;
				SoftwareTimer_uint8=0; // reset SoftwareTimer_uint8

				tolerance=BitTimeHigh>>2; // 1/4 t
				if(tolerance<2) tolerance =2;// only did this for very slow timer with low resolution bit time
				

				// check if t is close to t_old
				if( ( t_old>(BitTimeHigh-tolerance)) && (t_old<(BitTimeHigh+tolerance)) )
				{
					counter ++ ;
				}
				else
				{
					counter=0;
				}

				t_old=BitTimeHigh;

				if (counter>=SYNCCOUNTER)
				{
					state=M1_READY; 					// ======> all sync bits received

					//HighTakesLonger=false;
					//if( BitTimeHigh > BitTimeLow ) HighTakesLonger = true;
					HighTakesLonger=true;
//TOGGLELED;					
				}
				else state = M1_PINLOW;					 // ======> switch to low state
			}
		}break;
		deault:
		{
			error=true;
		}
	}
	return state;
}
/***************************************************************************************

   highBitReceived_S()

   output:  state ( SENDERBITREADY when bit received )

***************************************************************************************/
uint8_t BitValue=0;
uint8_t BitError=BITOK;
#define TIMEOUTCYCLES 10000

enum bitState highBitReceived_S()
{
        static uint8_t p,t,tolerance;

        static enum bitState state;
        static uint16_t timeOutCounter;

        switch(state)
        {
        	case SENDERBITREADY:{
        		timeOutCounter=0;
        		state=BITSTART;  					//==> BITSTATE1
        	}// attention: fall through !
        	case BITSTART:
			{
        		BitValue=0;
        		//BitError=BITOK;

                if(HighTakesLonger)
                {
                	// wait for high
                	if(PINHIGH)
                	{
                        t=SoftwareTimer_uint8;
                        SoftwareTimer_uint8=0; // reset SoftwareTimer_uint8
                        p=PINVALUE;
                		state=BITEND; 	        //==> BITSTATE2
                	}
                }
                else
                {
                    // wait for low
                	if(PINLOW)
                	{
                		t=SoftwareTimer_uint8;
                		SoftwareTimer_uint8=0; // reset SoftwareTimer_uint8
                		p=PINVALUE;
                		state=BITEND;            //==> BITSTATE2
                	}
                }
        	}break;
        	case BITEND:
			{
                if(p!=PINVALUE) // wait for edge
                {
                	t=SoftwareTimer_uint8;
                    SoftwareTimer_uint8=0; // reset SoftwareTimer_uint8
                    p=PINVALUE;

                    if(HighTakesLonger)
                    {
                            tolerance=(BitTimeHigh>>2);
                            //if(t>(BitTimeHigh+tolerance)) BitValue=1;
                            if(t<(BitTimeHigh-tolerance)) BitValue=1;

                            if(t< ( BitTimeHigh>>1) ) BitError=TOSHORT;//SystemOutDec("to short",t);
                    }else
                    {
                            tolerance=(BitTimeLow>>2);
                            //if(t>(BitTimeLow+tolerance)) BitValue=1;
                            if(t<(BitTimeLow-tolerance)) BitValue=1;
                            if(t< ( BitTimeLow>>1) ) BitError=TOSHORT; // SystemOutDec("to short",t);
                    }
					
//if(BitValue==1){LEDON;}
//else {LEDOFF;};
                    state=SENDERBITREADY;                 //==> SENDERBITREADY
                }
        	}break;
        	default: state=SENDERBITREADY;
        }
        timeOutCounter++;
        if( timeOutCounter > TIMEOUTCYCLES)
        {
        	BitError=BITTIMEOUT;
        	state=SENDERBITREADY;
        	//SystemOutDec("----BITTIMEOUT ",t);
        }

        return state;
}

/***************************************************************************************

    receiveByte_S()

    output:  state ( BYTEREADY when byte received )

***************************************************************************************/

uint8_t ReceiverData;

#define NUMBEROFBITS 8

uint8_t receiveByte_S()
{

	static uint8_t state=BYTEREADY;
	static uint8_t bitCounter,dat;

	switch(state)
	{
		case BYTEREADY:
		{
           state=WAITFORSTARTBIT;                          // ==> WAITFORSTARTBIT
           bitCounter=0;
		} // fall through

		case WAITFORSTARTBIT:
		{
			if(highBitReceived_S()==SENDERBITREADY)
			{
			   if(BitValue==1) state=RECEIVEBITS;          // ==>RECEIVEBITS
			}
		}break;

		case RECEIVEBITS:
		{
//LEDON; // DEBUG
			if(highBitReceived_S()==SENDERBITREADY)
			{
				dat=dat<<1;
				dat=dat|BitValue;
				bitCounter++;
				if(bitCounter==NUMBEROFBITS)
				{
					state=BYTEREADY;     // ==> RECEIVERREADY
					ReceiverData=dat;
//LEDOFF;
				}

			}
		}break;
	}
	return state;
}
/***************************************************************************************

    receiveFrame_S()

    output:  state ( FRAMEREADY when frame received )

***************************************************************************************/

volatile uint8_t FrameData[FRAMESIZE];
uint8_t FrameError;

uint8_t receiveFrame_S()
{

	static uint8_t state=FRAMEREADY;
	static uint8_t byteCounter;
	static uint16_t timeOutCounter;

	switch(state)
	{
		case FRAMEREADY:
		{
           state=BITRATEESTIMATION;                          // ==> WAITFORSTARTBIT
           byteCounter=0;
           timeOutCounter=0;
           FrameError=FRAMEOK;
		} // fall through

		case BITRATEESTIMATION:
		{
			if(BrEstimationStateMachine(START)==M1_READY)
			{
				timeOutCounter=0;
				state=RECEIVEBYTES;          // ==>RECEIVEBITS
			}
		}break;

		case RECEIVEBYTES:
		{
			if(receiveByte_S()==BYTEREADY)
			{
				FrameData[byteCounter]=ReceiverData;
				byteCounter++;
				if(byteCounter==FRAMESIZE)
				{
					state=DATARECEIVED;     // ==> RECEIVERREADY
				}
			}
		}break;
		case DATARECEIVED:
		{
			state=FRAMEREADY;     // ==> RECEIVERREADY
		}
	}
    timeOutCounter++;
    if( timeOutCounter > TIMEOUTCYCLES)
    {
    	FrameError=FRAMETIMEOUT;
    	state=FRAMEREADY;;
    }
	return state;
}






