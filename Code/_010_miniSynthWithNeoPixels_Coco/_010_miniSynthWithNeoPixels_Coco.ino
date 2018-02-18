/*************************************************************************************

  Experimental Synth

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


  20.2.2017  chris, version 1.5    for ATTINY85
  27.2.2017  chris, version 2.0    extended version

  C. -H-A-B-E-R-E-R- alias chris

  Please maintain the list of authors.

*************************************************************************************/

#include "Pt1.h"
#include "Oscillator.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// hardware description / pin connections
#define SPEAKERPIN      1
#define NEOPIXELPIN     0

#define POTI_LEFT      A1
#define POTI_RIGHT     A2
#define BUTTONS_ADC    A3

#define NUMPIXELS      8

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

const int8_t PROGMEM  sintab[] = {
  0 , 3 , 6 , 9 , 12 , 15 , 18 , 21 , 24 , 27 , 30 , 33 , 36 , 39 , 42 , 45 , 48 , 51 , 54 , 57 , 59 , 62 , 65 , 67 , 70 , 73 , 75 , 78 , 80 , 82 , 85 , 87 ,
  89 , 91 , 94 , 96 , 98 , 100 , 102 , 103 , 105 , 107 , 108 , 110 , 112 , 113 , 114 , 116 , 117 , 118 , 119 , 120 , 121 , 122 , 123 , 123 , 124 , 125 , 125 , 126 , 126 , 126 , 126 , 126 ,
  127 , 126 , 126 , 126 , 126 , 126 , 125 , 125 , 124 , 123 , 123 , 122 , 121 , 120 , 119 , 118 , 117 , 116 , 114 , 113 , 112 , 110 , 108 , 107 , 105 , 103 , 102 , 100 , 98 , 96 , 94 , 91 ,
  89 , 87 , 85 , 82 , 80 , 78 , 75 , 73 , 70 , 67 , 65 , 62 , 59 , 57 , 54 , 51 , 48 , 45 , 42 , 39 , 36 , 33 , 30 , 27 , 24 , 21 , 18 , 15 , 12 , 9 , 6 , 3 ,
  0 , -3 , -6 , -9 , -12 , -15 , -18 , -21 , -24 , -27 , -30 , -33 , -36 , -39 , -42 , -45 , -48 , -51 , -54 , -57 , -59 , -62 , -65 , -67 , -70 , -73 , -75 , -78 , -80 , -82 , -85 , -87 ,
  -89 , -91 , -94 , -96 , -98 , -100 , -102 , -103 , -105 , -107 , -108 , -110 , -112 , -113 , -114 , -116 , -117 , -118 , -119 , -120 , -121 , -122 , -123 , -123 , -124 , -125 , -125 , -126 , -126 , -126 , -126 , -126 ,
  -127 , -126 , -126 , -126 , -126 , -126 , -125 , -125 , -124 , -123 , -123 , -122 , -121 , -120 , -119 , -118 , -117 , -116 , -114 , -113 , -112 , -110 , -108 , -107 , -105 , -103 , -102 , -100 , -98 , -96 , -94 , -91 ,
  -89 , -87 , -85 , -82 , -80 , -78 , -75 , -73 , -70 , -67 , -65 , -62 , -59 , -57 , -54 , -51 , -48 , -45 , -42 , -39 , -36 , -33 , -30 , -27 , -24 , -21 , -18 , -15 , -12 , -9 , -6 , -3
};

int8_t sin_int8(uint8_t index)
{
  return pgm_read_byte( &sintab[index] );
}

#define F_CPU 16000000L

volatile uint16_t Freq_coefficient = 1;
volatile uint8_t  Amplitude = 255;

void setFrequency(uint16_t frequency_Hz)
{
  uint16_t temp = (uint32_t) F_CPU / 256 * frequency_Hz / 256 / 128;
  cli();
  Freq_coefficient = temp;
  sei();
}

void setAmplitude(uint8_t amp)
{
  cli();
  Amplitude = amp;
  sei();
}


#define SINUS 0
#define TRIANGLE 1
#define SAWTOOTH 2
#define RECTANGLE 3
#define NOISE 4

uint8_t wavetype = SINUS;
uint8_t RectanglePwmValue = 40;
SIGNAL(TIMER1_COMPA_vect)
{
  static uint8_t x;
  static uint16_t phase1, oldphase;
  static int8_t k, flag = 0;
  static int16_t temp, temp2;
  static uint16_t noise = 0xAA;

  if (flag)
  {
    //Phase VCO
    oldphase = phase1;
    phase1 = phase1 + Freq_coefficient;

    if (wavetype == SINUS)
    {
      temp  = (int8_t) pgm_read_byte ( &sintab[ phase1 >> 8 ] ) ;
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
    pixels.setPixelColor(n, 0); // off
  }
}

void setup ()
{

  cli();

  // Set up Timer/Counter1 31250Hz
  TCCR1 &= ~(7 << CS10); // make sure bits are empty
  TCCR1 |= 1 << CS11;    // set prescaler

  // pulse width modulator A enable
  TCCR1 &= ~(1 << COM1A0);       // make sure register is empty
  TCCR1 |=   1 << COM1A1 ;       // PWM mode: clear the OC1A output line on compare match

  TCCR1 |= 1 << PWM1A;

  TIMSK |= 1 << OCIE1A;          // Enable compare match interrupt for timer 1

  OCR1A = 128;                   // initialize PWM to 50% mean value

  DDRB |= 1 << PB1; // PWM pin as output

  sei();
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif

  pixels.begin(); // This initializes the NeoPixel library.
  setFrequency(440);


}

// analog read scaled for the 8BitMixTape voltage divider
uint16_t analogReadScaled(uint8_t channel)
{
  uint16_t value = analogRead(channel);
  if (value > 511) value = 511;
  return value * 2;
}

#define BUTTONS_ADC    A3

/*
  uint8_t getButton()

  return value:
  1: left button pressed
  2: right button pressed
  3: both buttons pressed
  ADC values
  no button:512, left:341, right:248, both:200
*/
uint8_t getButton()
{
  uint8_t button = 0;
  if (analogRead(BUTTONS_ADC) < 480) button = 1;
  if (analogRead(BUTTONS_ADC) < 280) button = 2;
  if (analogRead(BUTTONS_ADC) < 220) button = 3;

  return button;
}

#define LoopTime_ms 5
float AmplitudeModulationFrequency_Hz = 3;
float FrequencyModulationFrequency_Hz = 0.1;

#define POTI_LEFT      A1
#define POTI_RIGHT     A2

uint16_t CycleTimer1_ms = 3000;

Pt1 lp1(0x200);
Pt1 lp2(0x4000);
Oscillator lfo1( LoopTime_ms );
Oscillator lfo2( LoopTime_ms );


void loop()
{
  static uint16_t startTime_ms;

  static int16_t timer1;
  static uint16_t maxFrequency = 3000;

  startTime_ms = millis();
  timer1 = CycleTimer1_ms;

  //lfo1.setFrequency_Hz(10);
  lfo1.setMinMax(10, maxFrequency);
  //lfo1.setWaveform(RECTANGLE);
  //lfo1.setPwmValue(20);

  //lp1.setOutput(255);

  lfo2.setMinMax(0, 255);
  uint8_t selector = 1;
  setColorAllPixel(0); // pixels off

  displayBinrayValue( 0xF, pixels.Color(100, 0, 100));
  pixels.setPixelColor(selector - 1, pixels.Color(0, 100, 0));
  pixels.show(); // This sends the updated pixel color to the hardware.

  uint16_t  mainEnvelope = 255;
  uint8_t singleShot_flag=0;
  while (1)
  {

    // wait for loop cycle
    while ( (uint16_t) millis() - startTime_ms < LoopTime_ms );
    startTime_ms = millis();

    timer1 -= LoopTime_ms;
    if (timer1 <= 0)
    {
      timer1 = CycleTimer1_ms;
      //lp1.setOutput(255);
    }
    volatile static uint16_t v1, v2, v3, v4;
    v3 = analogReadScaled(POTI_RIGHT);
    v4 = analogReadScaled(POTI_LEFT);

    lfo1.setFrequency_mHz(v3 * 60);

    uint16_t lfo2_Hz;
    uint16_t lfo2ModulationMax_Hz;
    uint16_t lfo1Wafeform;
    uint8_t  mainOscillatorWaveType;



    if (selector == 1)
    {
      lfo2_Hz = v4 * 60;
      lfo2.setFrequency_mHz(lfo2_Hz);
    }

    if (selector == 2)
    {
      mainOscillatorWaveType = v4 >> 7;
      if(mainOscillatorWaveType>NOISE)mainOscillatorWaveType=NOISE; 
      wavetype = mainOscillatorWaveType;
    }

    if (selector == 3)
    {
      lfo2ModulationMax_Hz = v4 * 3 + 20;
      lfo1.setMinMax(10, lfo2ModulationMax_Hz);
    }

    if (selector == 4)
    {
      lfo1Wafeform = v4;
      lfo1.setWaveformFine(v4);
    }

    v1 = lfo1.calcNewValue();
    v2 = lfo2.calcNewValue();

    setFrequency(v1);
    setAmplitude((v2 * lp1.filter(mainEnvelope)) >> 8);

    if (getButton() == 1)
    {
      singleShot_flag=true;
      setColorAllPixel(0); // pixels off

      //displayBinrayValue( 0xF, pixels.Color(0, 0, 100));
      pixels.setPixelColor(selector - 1, pixels.Color(0, 100, 0));
      pixels.show(); // This sends the updated pixel color to the hardware.

      mainEnvelope = 0;
      lp1.setOutput(255);

    }

    if (getButton() == 2)
    {
      mainEnvelope = 255;
      if(!singleShot_flag)      selector++;
      singleShot_flag=false;
      if (selector > 4) selector = 1;

      setColorAllPixel(0); // pixels off

      displayBinrayValue( 0xF, pixels.Color(0, 0, 100));
      pixels.setPixelColor(selector - 1, pixels.Color(0, 100, 0));
      //displayBinrayValue( selector, pixels.Color(0, 100, 0));
      pixels.show(); // This sends the updated pixel color to the hardware.

      while (getButton() != 0);

    }


  }
}



