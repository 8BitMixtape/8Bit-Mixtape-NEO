/*

  Oneliners for the hardware of the 8Bit synthesizer

  8BitMixedTape-SoundProg2085
  Berliner Schule
  Version 0.8 Neo

  With this software you can test if your hardware is working correct.
  The speacker makes a beep sound.
  The value of the potientiometer is displayed as binary value on the leds.
  One potientiometer is displayed in green and the other one is displayed in blue.
  You can press the buttons. They are displayed in red. If both buttons are pressed,
  it will make a sound.

*************************************************************************************

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

********************************** list of outhors **********************************

dusjagr

*************************************************************************************

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
                                            

*************************************************************************************
 */

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

#define NUMPIXELS      1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// fast pin access
#define AUDIOPIN (1<<SPEAKERPIN)
#define PINLOW (PORTB&=~AUDIOPIN)
#define PINHIGH (PORTB|=AUDIOPIN)

// define variables for one-liners
int snd = 0; 
long t = 0; 
int bright = 255;
int delta_T = 2;
uint16_t p1 = 2;
int buttonState1 = 1; 
int lastButtonState1 = 1;
int buttonState2 = 1;
int lastButtonState2 = 1;
int count = 0;
int outputStyle = 0;
int visuals = 0;

void setup() {
  
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1); //what is this?
#endif
  
  TCCR0B = TCCR0B & B11111000 | B00000001; //no timer pre-scaler, fast PWM
  
  pixels.begin(); // This initializes the NeoPixel library.
  pinMode(SPEAKERPIN, OUTPUT);
  pixels.setBrightness(255);
  //rainbowCycle(3,3);
  delay (50);
}

/*
  uint8_t getButton()

  return value: 1 left button pressed  2 right button pressed  3 both buttons pressed
  ADC values: no button:512, left:341, right:248, both:200
*/

uint8_t getButton()
{
  uint8_t button = 0;
  if (analogRead(BUTTONS_ADC) < 450) button = 1;
  if (analogRead(BUTTONS_ADC) < 100) button = 2;
  if (analogRead(BUTTONS_ADC) < 50) button = 3;

  return button;
}

// scaled the adc of the 8BitMixTape voltage divider
// to a return value in range: 0..1023
uint16_t analogReadScaled(uint8_t channel)
{
  uint16_t value = analogRead(channel);
  if (value > 511) value = 511;
  return value * 2;
}

void setColorAllPixel(uint32_t color)
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    pixels.setPixelColor(n, 0); // off
  }
}

void loop() {

  uint8_t x = getButton();

  if (x == 1) {
    buttonState1 = LOW; 
  }
  else {buttonState1 = HIGH;}
  
  if (buttonState1 != lastButtonState1 && buttonState1 == HIGH) {
    // if the state has changed, increment the counter
      count++;
      pixels.setPixelColor(0, Wheel(count+7<<5));
      pixels.show();
      t = 0; 
      if (count >= 8) {
        count = 0;
      } 
    }
    lastButtonState1 = buttonState1;

  // check if potentiometer is all the way down
  uint16_t valuePoti = analogRead(POTI_RIGHT);
  p1 = 1023 - valuePoti>>5;
  
  if (valuePoti < 30) {
    buttonState2 = LOW;
    if (outputStyle == 1) {
      pixels.setBrightness(255);
      pixels.setPixelColor(0, Wheel(count+7<<5));
      pixels.show();
      delay(10000);
      }
    if (outputStyle == 0) {
      pixels.setBrightness(255);
      rainbowCycle(100,2);
      pixels.setPixelColor(0, Wheel(count+7<<5));
      pixels.show();
      delay(10000);
      }
    }
  else {
    buttonState2 = HIGH;
    }
    
  if (buttonState2 != lastButtonState2 && buttonState2 == LOW) {

  }
  if (buttonState2 != lastButtonState2 && buttonState2 == HIGH) {
    pixels.setPixelColor(0, Wheel(count+7<<5));
    pixels.setBrightness(255);
    pixels.show();
    delay(20000);
    outputStyle++; 
    visuals++;
    if (outputStyle > 1) {
        outputStyle = 0;
      }
    if (visuals > 1) {
        visuals = 0;
      }  
  }
  lastButtonState2 = buttonState2;
  
  switch(count) {
     case 1: // a classic
       snd = (t|5) * ((t>>p1|t>>11)&p1&t>>3);
       
       break;          
     case 3: // a classic
       snd = (t*(t>>8|t>>4))>>(t>>p1);
       
       break;
     case 2: // a classic
       snd = t*t/p1;
       break;
     case 4: // a classic
       snd = (t|3) * ((t>>1|t>>6)&p1&t>>3);
       break;       
     case 0: // a classic
       snd = t*((t>>12|t>>8)&63&t>>(p1>>3));
       break;      
      case 5: // a classic
       snd = t * ((t>>p1|t>>3)&17&t>>9);
       break; 
     case 6: // //dubStep
       snd = t>>3&1?t>>4:-t>>(p1>>3) ; //dubStep
       break;        
     case 7: // Tribute to Klaus
       snd = ((((t/3)&(t>>11))|((t/5)&(t>>p1)))&7)*30; 
       break;
    }

//play the sounds
   if (outputStyle == 0) {
    digitalWrite (SPEAKERPIN, snd);
   }
   if (outputStyle == 1) {
    analogWrite (SPEAKERPIN, snd);
   }
   
//make some visuals
  if (visuals == 0) {           // Single Color
    if (t % 1  == 0) {
      pixels.setPixelColor(0, Wheel((count+7)<<5));
      pixels.setBrightness((snd+30) & 255);
      pixels.show();
    }
  }
  
  if (visuals == 1) {
    if (t % 1  == 0) {
      pixels.setPixelColor(0, Wheel(snd % 255));
      pixels.setBrightness((snd+30) & 255);
      pixels.show();
    }
  }  

//wait... this defines the speed. could be read while press & hold with poti
  delayMicroseconds(100);
  t++;
  
}

void rainbowCycle(uint8_t wait, uint8_t rounds) {
  uint16_t i, j;

  for (j = 0; j < 256 * rounds; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
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
