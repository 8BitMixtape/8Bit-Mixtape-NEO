/*
The Center for Alternative Coconut Research presents:
-----------------------------------------------------
  Description:
  8 step sequencer with different waveforms

  Press both bottoms at the same time to start cycle. Number of steps are defined by where you start.
  
  30.05.2017 061 ChrisMicro  | first commit
  20.11.2017 061 dusjagr     | modded colors and envelope control
  13.12.2017 062 dusjagr     | adding syncOut on PB5
  
  This code needs "Reset disabled" set via fusebits
  avrdude -P /dev/ttyACM0 -b 19200 -c avrisp -p t85 -U efuse:w:0xfe:m -U hfuse:w:0x5d:m -U lfuse:w:0xe1:m

=========================================================================================================
 _____  ______ _ _      ___  ____      _                       _   _  _____ _____ 
|  _  | | ___ (_) |     |  \/  (_)    | |                     | \ | ||  ___|  _  |
 \ V /  | |_/ /_| |_    | .  . |___  _| |_ __ _ _ __   ___    |  \| || |__ | | | |
 / _ \  | ___ \ | __|   | |\/| | \ \/ / __/ _` | '_ \ / _ \   | . ` ||  __|| | | |
| |_| | | |_/ / | |_    | |  | | |>  <| || (_| | |_) |  __/   | |\  || |___\ \_/ /
\_____/ \____/|_|\__|   \_|  |_/_/_/\_\\__\__,_| .__/ \___|   \_| \_/\____/ \___/ 
                                               | |                              
     http://8bitmixtape.cc                     |_|                    

  * based on TinyAudioBoot and hex2wav by Chris Haberer, Fredrik Olofsson, Budi Prakosa
    https://github.com/ChrisMicro/AttinySound

=========================================================================================================

  
*/

#include "neolib.h"
#include "neoSoundPittix.h"
#include "Pt1.h"

int syncPin = 5;

typedef struct
{
  uint16_t f_Hz;
  uint8_t waveType;
} sound_t;

sound_t Sounds[8];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* the setup routine runs once when you start the tape or press reset
========================================================================================================================
   _________            _                   
  | setup() |   ___ ___| |_ _ _ _ _ 
  |  o___o  |  |_ -| -_|  _| | | . |
  |__/___\__|  |___|___|_| |___|  _|
                               |_|    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void setup()
{
  neobegin();
  pinMode(syncPin, OUTPUT);
  
  // start sound
  beginNeoPittix();
  setFrequency(440);

  pixels.setPixelColor( 0, COLOR_GREEN );
  pixels.show();
}

uint8_t  LedPosition   = 0;
uint8_t  ColorPosition = 0;

#define POITHYSTERESIS 30 // was 30

int16_t oldPotiLeft;
int16_t oldPotiRight;

uint8_t State = 0;
uint16_t Counter = 1000;

Pt1 lp1(0x100);

uint8_t NumberOfSteps;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* The main loop to put all your code
========================================================================================================================
   _________    _    
  | loop()  |  | |___ ___ ___ 
  |  o___o  |  | | . | . | . | 
  |__/___\__|  |_|___|___|  _| 
                         |_| 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


void loop()
{
  pixels.setPixelColor( LedPosition, COLOR_BLACK );   // clear pixel

  uint8_t b = wasButtonPressed();

  if (b)
  {
    if ( b == BUTTON_LEFT                )   LedPosition--   ;
    if ( b == BUTTON_RIGHT               )   LedPosition++   ;
    if ( b == BUTTON_RIGHT + BUTTON_LEFT )
    {
      ColorPosition++ ;
      if (State == 0)
      {
        State = 1;
        NumberOfSteps = LedPosition;
        pixels.setPixelColor( LedPosition, COLOR_RED );
        pixels.show();
        delay(100);
      }
      else State = 0;
    }

    LedPosition   &= 0b00000111; // prevent out of range
    ColorPosition &= 0b00000011; // prevent out of range

    pixels.setPixelColor( LedPosition, COLOR_GREEN );
    pixels.show();

    setFrequency( Sounds[ LedPosition ].f_Hz );
  }

  int16_t lp = getPoti( POTI_LEFT );        // range 0..1023
  int16_t rp = getPoti( POTI_RIGHT )>>2;    // range 0..255

  if (State == 0)
  { 
    // progMode
    setAmplitude(255); // make sure the sound is on during progMode
    if ( abs(rp - oldPotiRight) > POITHYSTERESIS )
    {
      setWaveformFine(rp);
      oldPotiRight = rp;
      Sounds[ LedPosition ].waveType = rp;
      pixels.setPixelColor( LedPosition, Wheel(rp) );
      pixels.show();
    }

    if ( abs(lp - oldPotiLeft) > POITHYSTERESIS )
    {
      setFrequency(lp>>3);
      oldPotiLeft = lp;
      Sounds[ LedPosition ].f_Hz = lp;
      pixels.setPixelColor( LedPosition, Wheel(rp) );
      pixels.show();
    }
  }

  if (State == 1)
  {   
    // playMode
    lp1.setT((255-rp)*4);           // envelope time
    setAmplitude(lp1.filter(0));
    Counter--;
    if (Counter == 0)
    {
      Counter = 1023 - lp;

      if (Counter > 800) Counter = 800;
      if (Counter < 1) Counter = 1;

      LedPosition++;
      if ( LedPosition > NumberOfSteps) LedPosition = 0;

      pixels.setPixelColor( LedPosition, Wheel(Sounds[ LedPosition ].waveType)); // color of waveType

      // SyncOut per step
      digitalWrite(syncPin,HIGH);
      delay(1);
      digitalWrite(syncPin,LOW);
   
      pixels.show();

      setFrequency( Sounds[ LedPosition ].f_Hz );
      setWaveformFine( Sounds[ LedPosition ].waveType );

      lp1.setOutput(255);
    }

  }

  //delay(0); // no need for extra delay

}



