/************************************************************************************

  PC controlled synth

*************************************************************************************

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

********************************** list of outhors **********************************

  v0.1  6.3.2017 C. -H-A-B-E-R-E-R-  initial version

  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something
  2017 ChrisMicro
*************************************************************************************
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "neoSoundPittix.h"
#include "decoderStateMachine.h"
#include "Oscillator.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// hardware description / pin connections
#define NEOPIXELPIN     0

#define NUMPIXELS      12

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

void displayBinrayValue(uint16_t value, uint32_t color)
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    if (value & (1 << n)) pixels.setPixelColor(n, color);
    //else pixels.setPixelColor(n,0); // off
  }
}

void setColorAllPixel(uint32_t color)
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    pixels.setPixelColor(n, color);
  }
}

void setup()
{
  beginNeoPittix ();
  setFrequency(440);

  //delay(1000);
  INITAUDIOPORT;

  // pin change interrupts on Attiny85
  // https://thewanderingengineer.com/2014/08/11/pin-change-interrupts-on-attiny85/
  GIMSK = 0b00100000;    // turns on pin change interrupts
  // none none PCINT5 PCINT4 PCINT3 PCINT2 PCINT1 PCINT0
  PCMSK = 0b00001000;    // turn on interrupts on pins PB5
  sei();                 // enables interrupts
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif

  pixels.begin(); // This initializes the NeoPixel library.

  setColorAllPixel(pixels.Color(0, 0, 1)); // pixels off

  pixels.show(); // This sends the updated pixel color to the hardware.
}

volatile uint8_t dataAvailableFlag = false;
volatile uint16_t InterruptData;
uint16_t Data;

uint8_t isDataAvailable()
{
  uint8_t value;
  cli();
  value = dataAvailableFlag;
  dataAvailableFlag = false;
  Data = InterruptData;
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


#define POTI_LEFT      A1
#define POTI_RIGHT     A2

// the command has 5 bits
uint8_t getCommand()
{
  uint8_t value = (Data >> 10) & 0x1f;
  /*
    setColorAllPixel(0);
    displayBinrayValue(value, 15);

    pixels.show(); // This sends the updated pixel color to the hardware.
  */
  return value;
}

// the value has 10 bits
uint16_t getValue()
{
  uint16_t value = Data & 0x3FF;

  setColorAllPixel(0);
  displayBinrayValue(value, 15);

  pixels.show(); // This sends the updated pixel color to the hardware.

  return value;
}

uint8_t LedNumber = 0;
uint16_t Red = 0;
uint16_t Green = 0;
uint16_t Blue = 0;

uint8_t SynthSetup = 0;

#define LoopTime_ms 5

Oscillator lfo1( LoopTime_ms );
Oscillator lfo2( LoopTime_ms );

uint8_t Volume=255;
int16_t HFOfrequency=440;

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
        Volume=Blue;
      } break;
    case 4:
      {
        lfo1.setFrequency_mHz(getValue() * 32);
      } break;
    case 5:
      {
        lfo1.setWaveformFine(getValue());
      } break;
    case 6: // max lfo amplitude
      {
        lfo1.setMinMax(0,getValue()*4);
      } break;   
    case 7:
      {
        lfo2.setFrequency_mHz(getValue() * 32);
      } break;
    case 8:
      {
        lfo2.setWaveformFine(getValue());
      } break;
    case 9: // max lfo amplitude
      {
        lfo2.setMinMax(0,getValue());
      } break;
    case 16:
      {
        LedNumber = getValue();
        SynthSetup=LedNumber;
      } break;
  }
}



void loop()
{
  static uint16_t startTime_ms;


  lfo1.setMinMax(10, 3000);
  lfo2.setMinMax(1, 255);
  
  startTime_ms = millis();
  {
    while (1)
    {
      if (isDataAvailable())
      {
        comandInterpreter();
      }

      // wait for loop cycle
      while ( (uint16_t) millis() - startTime_ms < LoopTime_ms );
      startTime_ms = millis();
      
      uint16_t v1,v2;
      v1 = lfo1.calcNewValue();
      v2 = lfo2.calcNewValue();
      if (SynthSetup & 0x01 )
      {
        int16_t temp;
        temp=HFOfrequency+v1;
        setFrequency(temp);
      }
      if (SynthSetup & 0x02 ) setAmplitude((v2*Volume)>>8);
    }
  }
}
/*
  {
    if (isDataAvailable())
    {
    uint16_t value = Data&0x3FF;
    setColorAllPixel(0);
    displayBinrayValue(getCommand(), 15);

    pixels.show(); // This sends the updated pixel color to the hardware.
    setFrequency(value * 10 + 300);
    }
  }*/

/*
  int main(void)
  {
  setup();

    while (1)
    {
    loop();
    }
  }
*/

