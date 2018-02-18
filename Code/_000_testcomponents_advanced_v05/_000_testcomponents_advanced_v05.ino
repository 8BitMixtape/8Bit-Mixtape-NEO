/*
  Testprogram for the hardware of the 8Bit synthesizer
  8BitMixedTape-SoundProg2085
  Berliner Schule
  Version 0.8 Neo
  With this software you can test if your hardware is working correct.
  The speacker makes a beep sound.
  The value of the potientiometers is displayed bars from left and from right.
  One potientiometer is displayed in pink and the other one is displayed in green.
  You can press the buttons. They are displayed in blue and yellow in the middle. If both buttons are pressed,
  it will make a sound and the middle leds are red. use the potentiometers to change the sound.
  find the secret special visual!!
*************************************************************************************
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
********************************** list of outhors **********************************
  v0.1  15.2.2017 C. -H-A-B-E-R-E-R-  initial version
  v0.2  21.2.2017 C. -D-U-S-J-A-G-R-  improved the style
  v0.3  22.2.2017 C. -H-A-B-E-R-E-R-  merged and error crash due to adc scaling removed
  v0.4  22.2.2017 C. -D-U-S-J-A-G-R-  added better visuals and controlling speed and brightness of rainbow
  v0.5  07.6.2017    -D-U-S-J-A-G-R-  Mod for NeoCoco
  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something
   2017 ChrisMicro
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
  // IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across ???
*************************************************************************************
*/

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

#define NUMPIXELS      2

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// hardware calibration
#define Vbutton_left   380
#define Vbutton_right  300
#define Vbutton_both   240
#define Vcc            37 // 3.7 V for LiPo
#define Vdiv           26 // measure max Voltage on Analog In

// fast pin access
#define AUDIOPIN (1<<SPEAKERPIN)
#define PINLOW (PORTB&=~AUDIOPIN)
#define PINHIGH (PORTB|=AUDIOPIN)

void playSound(long freq_Hz, long duration_ms)
{
  uint16_t n;
  uint32_t delayTime_us;
  uint32_t counts;

  delayTime_us = 1000000UL / freq_Hz / 2;
  counts = duration_ms * 1000 / delayTime_us;

  for (n = 0; n < counts; n++)
  {
    PINHIGH;
    delayMicroseconds(delayTime_us);
    PINLOW;
    delayMicroseconds(delayTime_us);
  }
}

void setup()
{
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setBrightness(80);

  pinMode(SPEAKERPIN, OUTPUT);
  //analogReference( INTERNAL2V56 );
  playSound(1000, 100); // beep
  rainbowCycle(4, 2);
}


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
  if (analogRead(BUTTONS_ADC) < Vbutton_left) button = 1;
  if (analogRead(BUTTONS_ADC) < Vbutton_right) button = 2;
  if (analogRead(BUTTONS_ADC) < Vbutton_both) button = 3;

  return button;
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

void setColorLeftPixel(uint16_t numberPix, uint8_t colorR, uint8_t colorG, uint8_t colorB)
{
  uint8_t n;

  for (n = 0; n < numberPix; n++)
  {
    pixels.setPixelColor(n, pixels.Color(colorR, colorG, colorB));
  }

}

void setColorRightPixel(uint16_t numberPix, uint8_t colorR, uint8_t colorG, uint8_t colorB)
{
  uint8_t n;
  for (n = NUMPIXELS; n >= NUMPIXELS - numberPix; n--)
  {
    pixels.setPixelColor(n, pixels.Color(colorR, colorG, colorB)); // off
  }
}

// scaled the adc of the 8BitMixTape voltage divider
// to a return value in range: 0..1023
uint16_t analogReadScaled(uint8_t channel)
{
  uint16_t value = analogRead(channel);
  value = value * Vcc / Vdiv;
  if (value > 1023) value = 1023;
  return value;
}

void loop()
{
  pixels.setBrightness(255);
  uint16_t p1 = 1023 - analogReadScaled(POTI_LEFT);
  uint16_t p2 = analogReadScaled(POTI_RIGHT);

  setColorAllPixel(0); // pixels off

  setColorLeftPixel ((p2 >> 9), 50, 10, 30);
  setColorRightPixel((p1 >> 9), 0, 60, 0);

  uint8_t x = getButton();
  if (x == 1) pixels.setPixelColor(1, pixels.Color(0, 0, 80));
  if (x == 2) pixels.setPixelColor(0, pixels.Color(40, 40, 0));

  if (x == 3) {
    pixels.setPixelColor(1, pixels.Color(50, 0, 0));
    pixels.setPixelColor(0, pixels.Color(50, 0, 0));
    pixels.show(); // This sends the updated pixel color to the hardware.
    playSound(p1 + 20, (p2 / 5) + 10);
  }

  if (x == 3 && p1 < 80) {
    pixels.setBrightness(80);
    rainbowCycle(10, 10);
  }

  pixels.show(); // This sends the updated pixel color to the hardware.
}

void rainbowCycle(uint8_t wait, uint8_t rounds) {
  uint16_t i, j;

  for (j = 0; j < 256 * rounds; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    uint16_t brightosiech = analogReadScaled(POTI_RIGHT)>>2;
    uint16_t speedosiech = analogReadScaled(POTI_LEFT)>>7;
    pixels.setBrightness(brightosiech+5);
    pixels.show();
    delay(5);
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


