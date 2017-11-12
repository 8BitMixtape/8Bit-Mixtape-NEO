/*
  8 step sequencer with different waveforms

  Press both bottoms at the same time to start cycle.

  30.5.2017 ChrisMicro
*/

#include "neolib.h"
#include "neoSoundPittix.h"
#include "Pt1.h"


typedef struct
{
  uint16_t f_Hz;
  uint8_t waveType;
} sound_t;

sound_t Sounds[8];

void setup()
{
  neobegin();

  // start sound
  beginNeoPittix();
  setFrequency(440);

  pixels.setPixelColor( 0, COLOR_GREEN );
  pixels.show();
}

uint8_t  LedPosition   = 0;
uint8_t  ColorPosition = 0;
//uint32_t Colors[] = { COLOR_GREEN, COLOR_RED, COLOR_BLUE, COLOR_PINK};


#define POITHYSTERESIS 30 // was 30

int16_t oldPotiLeft;
int16_t oldPotiRight;

uint8_t State = 0;
uint16_t Counter = 1000;

Pt1 lp1(0x100);

uint8_t NumberOfSteps;

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

  int16_t lp = getPoti( POTI_LEFT );   // range 0..1023
  int16_t rp = getPoti( POTI_RIGHT )>>2;  // range 0..1023

  if (State == 0)
  {
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
      setFrequency(lp);
      oldPotiLeft = lp;
      Sounds[ LedPosition ].f_Hz = lp;
      pixels.setPixelColor( LedPosition, Wheel(rp) );
      pixels.show();
    }
  }

  if (State == 1)
  {
    lp1.setT((255-rp)*4);
    setAmplitude(lp1.filter(0));
    Counter--;
    if (Counter == 0)
    {

      Counter = 1023 - lp;

      if (Counter > 800) Counter = 800;
      if (Counter < 2) Counter = 2;


      LedPosition++;
      if ( LedPosition > NumberOfSteps) LedPosition = 0;

      pixels.setPixelColor( LedPosition, Wheel(Sounds[ LedPosition ].waveType)); // COLOR_PINK
      pixels.show();

      setFrequency( Sounds[ LedPosition ].f_Hz );
      setWaveformFine( Sounds[ LedPosition ].waveType );

      lp1.setOutput(255);
    }

  }

  delay(0);


}



