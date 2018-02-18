/*************************************************************************************

  Attiny SoundPitix-VCO

  Basic sound generator for building modular mixed analog/digital
  synthesizers.

  This was originally made for the chTinySizer an little mixed analog/digital
  synthesizer with 4 ATTINY13 an one Atmega168.

  Today ( 2017 ) the sound could be improved alot by new software techncs.
  Therefore this is a more ore less historic ( 10 years ) version.


*************************************************************************************/

/*************************************************************************************

  Hardware Platform: Attiny85

                                      ATTINY85 Pins
                                      =============

                                             _______
                                            |   U   |
  DEBUGLED / SYNC-OUT             reset/PB5-|       |- VCC
  soundprog / buttons ->  D3/A3         PB3-| ATTINY|- PB2    D2/A1  <- POTI1 / CV-IN
  POTI2 / SYNC-IN ->      D4/A2         PB4-|   85  |- PB1    D1     -> PWM SOUND
                                        GND-|       |- PB0    D0     -> NEOPIXELS
                                            |_______|


*************************************************************************************/

/*************************************************************************************
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.


  11.09.2007  chris, version 1.5    for ATTINY13
  13.08.2014  chris, version 1.5.1  copyright message added, code cleaned
  17.02.2017  chris, version 2.0    code reworked vor Attiny85, sound out PB1
  04.03.2017  chris, version 2.1    code encapsulated in separate file

  C. -H-A-B-E-R-E-R- alias chris

  Please maintain the list of authors.

*************************************************************************************/

#include <stdint.h>
#include "integer_sin.h"
#include "neoSoundPittix.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000L

volatile uint16_t Freq_coefficient = 1;
volatile uint8_t  Amplitude = 255;
volatile uint8_t wavetype = SINUS;
volatile uint8_t RectanglePwmValue = 40;

// software timer available for external use
// increased with the sample rate frequency
volatile uint8_t SoftwareTimer_uint8;

void beginNeoPittix ()
{

  cli();

  // Set up Timer/Counter1 31250Hz
  TCCR1 &= ~(7 << CS10); // make sure bits are empty
  TCCR1 |= 1 << CS11;    // set prescaler

  // pulse width modulator A enable
  TCCR1 &= ~(1 << COM1A0);       // make sure register is empty
  TCCR1 |=   1 << COM1A1 ;       // PWM mode: clear the OC1A output line on compare match
  
  //TCCR1 |= 1 << CS12 | 1 << CS10;    // set prescaler
  //PLLCSR &=~ 1<<PCKE; // slow clock
  
  TCCR1 |= 1 << PWM1A;

  TIMSK |= 1 << OCIE1A;          // Enable compare match interrupt for timer 1

  OCR1A = 128;                   // initialize PWM to 50% mean value

  DDRB |= 1 << PB1; // PWM pin as output

  sei();
}
/*
  // sampling frequency
  #define FS (37500/2) // @FCPU=9.6Mhz

  uint16_t freq_const = 440 * 65536 / FS;
*/
void setFrequency(uint16_t frequency_Hz)
{
  uint16_t temp = (uint32_t) F_CPU / 256 * frequency_Hz / 256 / 64;
  cli();
  Freq_coefficient = temp;
  sei();
}

void setAmplitude(uint8_t amp)
{
  Amplitude = amp;
}

void setWaveform(uint8_t waveType)
{
  wavetype = waveType;
}
void setWaveformFine(uint8_t wert)
{
  uint8_t wave;
  
  wert=255-wert;
  
  if (wert <= 60) {
    wave = NOISE;
  }
  if (wert > 60 && wert <= 188)
  {
    wave = RECTANGLE;
    RectanglePwmValue = 315-wert;
  }
  if (wert > 188) wave = SAWTOOTH;
  if (wert > 200) wave = TRIANGLE;
  if (wert > 220) wave = SINUS;

  wavetype = wave;
}

void setPwmValue(uint8_t value)
{
  RectanglePwmValue = -value;
}

SIGNAL(TIMER1_COMPA_vect)
{
  static uint8_t x;
  static uint16_t phase1, oldphase;
  static int8_t flag = 0;
  static int16_t temp, temp2;
  static uint16_t noise = 0xAA;
  
  SoftwareTimer_uint8++;
  if (flag)
  {
    //Phase VCO
    oldphase = phase1;
    phase1 = phase1 + Freq_coefficient;

    if (wavetype == SINUS)
    {
      temp  = (int8_t)  sin_int8( phase1 >> 8 )  ;
    }
    if (wavetype == TRIANGLE)
    {
      x = phase1 >> 8;
      if (x & 0x80) temp = (x ^ 0xFF);
      else temp = x;
      temp = (temp << 1) - 128;
    }

    if (wavetype == RECTANGLE)
    {
      x = phase1 >> 8;
      if (x > RectanglePwmValue)
      {
        temp = 127;
      }
      else temp = -127;
    }
    if (wavetype == SAWTOOTH)
    {
      temp = (phase1 >> 8) - 128;
    }
    if (wavetype == NOISE)
    {
      temp = noise;
      noise = noise << 1;
      temp ^= noise;
      if ( ( temp & 0x4000 ) == 0x4000 )noise |= 1;

      if ((oldphase ^ phase1) & 0x4000)
      {
        temp2 = (noise >> 6) & 0xFF;
      }
      temp = temp2;
    }
  }
  else
  {
    temp = (temp * Amplitude);

    OCR1A = (temp >> 8) + 128;

  }
  flag ^= 0x01;
}


