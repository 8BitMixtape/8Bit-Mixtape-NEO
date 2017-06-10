/************************************************************************************

  PC and Poit controlled synth

*************************************************************************************

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

********************************** list of outhors **********************************

  v0.1   6.3.2017 C. -H-A-B-E-R-E-R-  initial version
  v0.2  21.3.2017 C. -H-A-B-E-R-E-R- poti control added

  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something
  2017 ChrisMicro
*************************************************************************************
*/

#include "neolib.h";
#include "neoSoundPittix.h"
#include "decoderStateMachine.h"
#include "Oscillator.h"

// the command has 5 bits
uint8_t getCommand()
{
  uint8_t value = (DecoderData >> 10) & 0x1f;
  return value;
}

// the value has 10 bits
uint16_t getValue()
{
  uint16_t value = DecoderData & 0x3FF;

  setColorAllPixel(0);
  displayBinaryValue(value, 100);

  pixels.show(); // This sends the updated pixel color to the hardware.

  return value;
}

uint8_t  LedNumber = 0;
uint16_t Red = 0;
uint16_t Green = 0;
uint16_t Blue = 0;
uint16_t Helligkeit = 70;


#define LoopTime_ms 5

Oscillator lfo1( LoopTime_ms );
Oscillator lfo2( LoopTime_ms );

uint8_t  Volume = 255;
int16_t  HFOfrequency = 440;
uint16_t LFO1_Frequency_mHz = 0;
uint16_t LFO2_Frequency_mHz = 0;

void comandInterpreter()
{
  switch (getCommand())
  {
    case 1:
      {
        HFOfrequency = getValue();
        setFrequency(HFOfrequency * 2);
      } break;

    case 2:
      {
        Green = getValue();
        setWaveformFine(Green / 4);
      } break;

    case 3:
      {
        Blue = getValue();
        setAmplitude(Blue );
        Volume = Blue;
      } break;
    case 4:
      {
        LFO1_Frequency_mHz = getValue() * 32;
        lfo1.setFrequency_mHz( LFO1_Frequency_mHz );
      } break;
    case 5:
      {
        lfo1.setWaveformFine(getValue());
      } break;
    case 6: // max lfo amplitude
      {
        lfo1.setMinMax(0, getValue() * 4);
      } break;
    case 7:
      {
        LFO2_Frequency_mHz = getValue() * 32;
        lfo2.setFrequency_mHz( LFO2_Frequency_mHz );
      } break;
    case 8:
      {
        lfo2.setWaveformFine(getValue());
      } break;
    case 9: // max lfo amplitude
      {
        lfo2.setMinMax(0, getValue());
      } break;
  }
}

uint16_t StartTime_ms;

int16_t oldPotiLeft;
int16_t oldPotiRight;

void setup()
{
  neobegin();
  pixels.setBrightness(Helligkeit);
  // show leds
  setColorAllPixel(pixels.Color(Helligkeit, 0, Helligkeit));
  pixels.show(); // This sends the updated pixel color to the hardware.

  // start sound
  beginNeoPittix ();
  setFrequency(440);

  // start audio line command receiver
  decoderBegin();

  lfo1.setMinMax(10, 3000);
  lfo2.setMinMax(1, 255);

  StartTime_ms = millis();
  oldPotiLeft = getPoti(POTI_LEFT);
  oldPotiRight = getPoti(POTI_RIGHT);
}

uint8_t ButtonSelector1 = 0; 
uint8_t ButtonSelector2 = 0; 

void showState()
{
  setColorAllPixel(pixels.Color(Helligkeit, 0, Helligkeit));
  displayBinaryValue( 0b11111000 ,  pixels.Color(0, Helligkeit, Helligkeit)); // display the value as binary on 8 NEO leds
 

  pixels.setPixelColor(ButtonSelector1, COLOR_DARKGREEN);

  pixels.setPixelColor(7 - ButtonSelector2, COLOR_RED);
  pixels.setBrightness(100);
  pixels.show();
}

#define POITHYSTERESIS 30

void loop()
{

  static uint8_t potiControlFlag = false;
  uint8_t b;

  if (isDataAvailable())
  {
    comandInterpreter();
    potiControlFlag = false;
    oldPotiLeft = getPoti(POTI_LEFT);
  }

  int16_t lp = getPoti( POTI_LEFT );   // range 0..1023
  int16_t rp = getPoti( POTI_RIGHT );  // range 0..1023

  // wait for next loop cycle
  while ( (uint16_t) millis() - StartTime_ms < LoopTime_ms );
  StartTime_ms = millis();

  if ( abs(lp - oldPotiLeft) > POITHYSTERESIS *2 || abs(rp - oldPotiRight) > POITHYSTERESIS *2 && !potiControlFlag)
  {
    potiControlFlag = true;
    showState();
  }
  
  b = wasButtonPressed();

  if (b == 1 && potiControlFlag)
  {
    pixels.setPixelColor(ButtonSelector1, COLOR_BLACK);
    ButtonSelector1++;
    if (ButtonSelector1 > 2) ButtonSelector1 = 0;

    oldPotiLeft = getPoti(POTI_LEFT);
    showState();
  }
  
  if (b == 2 && potiControlFlag)
  {
    pixels.setPixelColor(7 - ButtonSelector2, COLOR_BLACK);
    ButtonSelector2++;
    if (ButtonSelector2 > 4) ButtonSelector2 = 0;

    oldPotiRight = getPoti(POTI_RIGHT);
    showState();
  }

  if (potiControlFlag)
  {


    if ( abs(lp - oldPotiLeft) > POITHYSTERESIS )
    {
      switch (ButtonSelector1)
      {
        case 0:
          {
            HFOfrequency = lp;
            setFrequency(HFOfrequency * 2);
          }
          break;
        case 1:
          {
            setWaveformFine(lp / 2);
          }
          break;
        case 2:
          {
            Volume = lp / 4;
          }
          break;
      }
      oldPotiLeft = lp;
    }

    if ( abs(rp - oldPotiRight) > POITHYSTERESIS )
    {
      switch (ButtonSelector2)
      {
        case 0:
          {
            LFO1_Frequency_mHz = rp * 16;
            lfo1.setFrequency_mHz( LFO1_Frequency_mHz );
          } break;
        case 1:
          {
            lfo1.setWaveformFine(rp);
          } break;
        case 2: // max lfo amplitude
          {
            lfo1.setMinMax(0, rp * 4);
          } break;
        case 3:
          {
            LFO2_Frequency_mHz = rp * 32;
            lfo2.setFrequency_mHz( LFO2_Frequency_mHz );
          } break;
        case 4:
          {
            lfo2.setWaveformFine(rp);
          } break;

          oldPotiRight = rp;

      }
    }
  }

  uint16_t v1, v2;

  v1 = lfo1.calcNewValue();
  v2 = lfo2.calcNewValue();

  if (LFO1_Frequency_mHz != 0 )
  {
    int16_t temp;
    temp = HFOfrequency + v1;
    setFrequency(temp);
  } else setFrequency(HFOfrequency);

  if (LFO2_Frequency_mHz != 0  ) setAmplitude((v2 * Volume) >> 8);
  else setAmplitude(Volume);



}

