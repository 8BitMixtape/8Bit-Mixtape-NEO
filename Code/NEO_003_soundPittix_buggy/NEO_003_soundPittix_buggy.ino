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
  25.02.2017  dusjagr, version 2.1  minor edits, improve PWM mode and add some NEO-pixels
  13.3.2017 C. dusjagr, version 2.2 adapted to new schematics 0.95. different resistor values
  
  C. -H-A-B-E-R-E-R- alias chris

  Please maintain the list of authors.
  
*************************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// hardware description / pin connections
#define SPEAKERPIN      1
#define NEOPIXELPIN     0

#define POTI_RIGHT     A1
#define POTI_LEFT      A2
#define BUTTONS_ADC    A3

#define NUMPIXELS      8

// hardware calibration
#define Vbutton_left   380
#define Vbutton_right  300
#define Vbutton_both   240
#define Vcc            50 // 3.7 V for LiPo
#define Vdiv           34 // measure max Voltage on Analog In

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

#define INIT_ADC  ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1)

uint16_t adc_read(char input)
{
  // get ADC value
  uint16_t wert, temp = 0;
  uint8_t n;

  // VCC ref and ADCx
  ADMUX = (0 << REFS0) | (0x07 & input);

  // Clear ADIF and start single conversion
  ADCSRA |= (1 << ADIF) | (1 << ADSC);

  // wait for conversion done, ADIF flag active
  while (!(ADCSRA & (1 << ADIF)));

  n = ADCH; // dummy adc read

  for (n = 0; n < 4; n++)
  {
    // Clear ADIF and start single conversion
    ADCSRA |= (1 << ADIF) | (1 << ADSC);

    // wait for conversion done, ADIF flag active
    while (!(ADCSRA & (1 << ADIF)));
    // read out ADCL register
    wert = ADCL;
    // read out ADCH register
    wert += (ADCH << 8);

    temp += wert;
  }

  return (temp >> 2);
}

// sampling frequency
#define FS (37500/2) // @FCPU=9.6Mhz

uint8_t envelope = 255;
uint16_t freq_const = 440 * 65536 / FS;

#define SINUS 0
#define TRIANGLE 1
#define SAWTOOTH 2
#define RECTANGLE 3
#define NOISE 4

uint8_t wavetype = SAWTOOTH;
uint8_t RectanglePwmValue = 40;
uint16_t wert, wave, waveChange;
uint16_t adcwert;
long t = 0;
uint8_t n;
uint8_t bright = 35;
int buttonState1 = 1; 
int lastButtonState1 = 1;
int buttonState2 = 1;
int lastButtonState2 = 1;
int visuals = 1;

int8_t sinwerte[17] = {0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 106, 112, 117, 122, 125, 126, 127 };

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
    phase1 = phase1 + freq_const;

    if (wavetype == SINUS)
    {
      x = phase1 >> 10;

      if (x > 48)
      {
        x = 64 - x;
        k = -sinwerte[x];
      }
      else if (x > 32)
      {
        x = x - 32;
        k = -sinwerte[x];
      }
      else if (x > 16)
      {
        x = 32 - x;
        k = sinwerte[x];
      }
      else
      {
        k = sinwerte[x];
      }
      temp = k;
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
    temp = (temp * envelope);

    OCR1A = (temp >> 8) + 128;

  }
  flag ^= 0x01;
}

void setColorAllPixel(uint32_t color)
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    pixels.setPixelColor(n, color); // off
  }
}

uint8_t getButton()
{
  uint8_t button = 0;
  if (analogRead(BUTTONS_ADC) < Vbutton_left) button = 1;
  if (analogRead(BUTTONS_ADC) < Vbutton_right) button = 2;
  if (analogRead(BUTTONS_ADC) < Vbutton_both) button = 3;

  return button;
}

// scaled the adc of the 8BitMixTape voltage divider
// to a return value in range: 0..1023
// Use Vcc / Vdiv as measured on your hardware
uint16_t analogReadScaled(uint8_t channel)
{
  uint16_t value = analogRead(channel);
  value = value * Vcc / Vdiv;
  if (value > 1023) value = 1023;
  return value;
}

int main(void)
{

  INIT_ADC;
  cli();

  // Set up Timer/Counter1 62kHz.
  TCCR1 &= ~(0x0F << CS10); // make sure bits are empty
  TCCR1 |= 1 << CS11;    // set prescaler 1/8

  // pulse width modulator A enable
  TCCR1 &= ~(1 << COM1A0);       // make sure register is empty
  TCCR1 |=   1 << COM1A1 ;       // PWM mode: clear the OC1A output line on compare match

  TCCR1 |= 1 << PWM1A;           // PWM mode

  TIMSK |= 1 << OCIE1A;          // Enable compare match interrupt for timer 1

  OCR1A = 128;                   // initialize PWM to 50% mean value

  DDRB |= 1 << PB1; // PWM pin as output

  sei();

  // Ports
  PORTB = (1 << PORTB2) | (1 << PORTB3); // Pullup

  #if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1); //what is this?
  #endif
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setBrightness(bright);
  
  while (1)
  {
    uint8_t x = getButton();
    
    if (x == 0) {
      buttonState1 = HIGH;
      buttonState2 = HIGH;
    }
    
    if (x == 2) {
      buttonState2 = LOW;
      buttonState1 = HIGH;
      bright = analogReadScaled(POTI_RIGHT)>>3; 
      pixels.setBrightness(bright);
      
      uint8_t n;
      for (n = 0; n < NUMPIXELS; n++){
        pixels.setPixelColor(n, 255,255,255); // off
      }
      pixels.show(); 
    }
    
    if (buttonState2 != lastButtonState2 && buttonState2 == HIGH) {
      ledGeeking();
      pixels.show();      
    }

    if (x == 1) {
      buttonState1 = LOW;
    }
    
    if (buttonState1 != lastButtonState1 && buttonState1 == HIGH) {
    // if the state has changed, increment the counter
      ledGeeking();
      pixels.show();
      visuals++; 
      if (visuals > 1) {
        visuals = 0;
        for (n = 0; n < NUMPIXELS; n++){
        pixels.setPixelColor(n, 0,0,0); // off
      }
      pixels.show(); 
      } 
    }

  lastButtonState1 = buttonState1;
  lastButtonState2 = buttonState2;
    
    adcwert = analogReadScaled(POTI_LEFT);
    adcwert = adcwert;
    freq_const = adcwert * 2 * 65536 / FS / 2;
    /* LFO switch
        if (PINB & (1 << PINB1))
        {
          //TCCR0B = (1 << CS00); // VCO
          TCCR1 &= ~(0x0F << CS10); // make sure bits are empty
          TCCR1 |= 1 << CS13;    // set prescaler 1/8
        }
        else
        {
          //freq_const=adcwert*65536/FS/4; // LFO ( 1/10 Frequency )
          //TCCR0B = (1 << CS01) ; // 1/8 prescaling,
          TCCR1 &= ~(0x0F << CS10); // make sure bits are empty
          TCCR1 |= 1 << CS13;    // set prescaler
          TCCR1 |= 1 << CS12;    // set prescaler
          freq_const = adcwert;
        }
    */
    //envelope= adc_read(VOLUME_CONTROL)>>2;
    envelope = 255;
    wert = analogReadScaled(POTI_RIGHT)>>2;
    if (wert > 255)wert = 255;

    if(wert<=10) {wave=NOISE;}  
    if(wert>10) {wave=RECTANGLE;RectanglePwmValue=0x02;}
    if(wert>55) {wave=RECTANGLE;RectanglePwmValue=0x04;}
    if(wert>60) {wave=RECTANGLE;RectanglePwmValue=0x06;}
    if(wert>65) {wave=RECTANGLE;RectanglePwmValue=0x09;}
    if(wert>70) {wave=RECTANGLE;RectanglePwmValue=0x12;}
    if(wert>75) {wave=RECTANGLE;RectanglePwmValue=0x15;}
    if(wert>80) {wave=RECTANGLE;RectanglePwmValue=0x20;}
    if(wert>85) {wave=RECTANGLE;RectanglePwmValue=0x26;}
    if(wert>90) {wave=RECTANGLE;RectanglePwmValue=0x32;}      
    if(wert>95) {wave=RECTANGLE;RectanglePwmValue=0x40;}      
    if(wert>100) {wave=RECTANGLE;RectanglePwmValue=0x50;}
    if(wert>105) {wave=RECTANGLE;RectanglePwmValue=0x64;}  
    if(wert>110) {wave=RECTANGLE;RectanglePwmValue=0x80;}  
    if(wert>170) wave=SAWTOOTH; 
    if(wert>210) wave=TRIANGLE;
    if(wert>240) wave=SINUS;

    wavetype = wave;
    
    if (wave != waveChange && wave == SINUS && visuals == 1){
       setColorAllPixel(Wheel(30));
       pixels.show();
      }
    if (wave != waveChange && wave == TRIANGLE && visuals == 1){
       setColorAllPixel(Wheel(50));
       pixels.show();
    }
    if (wave != waveChange && wave == SAWTOOTH && visuals == 1){
       setColorAllPixel(Wheel(130));
       pixels.show();
    }
    if (wave != waveChange && wave == RECTANGLE && visuals == 1){
       setColorAllPixel(Wheel(220));
       pixels.show();
    }
    
    if (wave == NOISE && visuals == 1){
      for (int n = 0; n < ((1023 - adcwert)>>3)+1; n++){
        int siebeSiech = random(255);
        pixels.setPixelColor(random(8), siebeSiech, siebeSiech, siebeSiech);
        //pixels.setPixelColor(random(8), Wheel(random(255))); // off
      }
      pixels.show();
    }
    
    waveChange = wave;
      
  }
}

//
void ledGeeking(){
    if (wave == SINUS){
       setColorAllPixel(Wheel(30));
       pixels.show();
      }
    if (wave == TRIANGLE){
       setColorAllPixel(Wheel(50));
       pixels.show();
    }
    if (wave == SAWTOOTH){
       setColorAllPixel(Wheel(130));
       pixels.show();
    }
    if (wave == RECTANGLE){
       setColorAllPixel(Wheel(220));
       pixels.show();
    }
    
    if (wave == NOISE){
      for (int n = 0; n < ((1023 - adcwert)>>3)+1; n++){
        int siebeSiech = random(255);
        pixels.setPixelColor(random(8), siebeSiech, siebeSiech, siebeSiech);
        //pixels.setPixelColor(random(8), Wheel(random(255))); // off
      }
      pixels.show();
    }
    
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

