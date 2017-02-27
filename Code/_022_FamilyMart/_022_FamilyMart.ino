/*
  Family Mart Chime
  Arpeggio
  from http://fonte.me/arduino/#Arpeggio

  Plays arpeggios of chosen chords

  Requires an 8-ohm speaker on digital pin 8

  Copyright (c) 2012 Jeremy Fonte
  This code is released under the MIT license
  http://opensource.org/licenses/MIT
  code addapted to NEO: ChrisMicro
  code adapted to play other tune: dusjagr

  
  
*/
#include "pitches.h"

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

  pinMode(SPEAKERPIN, OUTPUT);

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
  if (analogRead(BUTTONS_ADC) < 450) button = 1;
  if (analogRead(BUTTONS_ADC) < 300) button = 2;
  if (analogRead(BUTTONS_ADC) < 220) button = 3;

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
    pixels.setPixelColor(n, color);
  }
}

void playMart(int notes[]);

// available chord to arpeggiate
//C major
int c[] = {
  NOTE_B4, NOTE_G4, NOTE_D4, NOTE_G4, NOTE_A4, NOTE_D5, PAUSE, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_D4, NOTE_G4 
};

int pauseBetweenNotes = 80;
int noteDuration_ms = 150;

void playMart(int notes[])
{

  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote <= 11; thisNote++) {

    displayBinrayValue(notes[thisNote],pixels.Color(50,0,50));
    pixels.show(); // This sends the updated pixel color to the hardware.
    playSound( notes[thisNote], noteDuration_ms);
    setColorAllPixel(0); // pixels off
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    delay(pauseBetweenNotes);
        
  }
  
}
// analog read scaled for the 8BitMixTape voltage divider
uint16_t analogReadScaled(uint8_t channel)
{
  uint16_t value = analogRead(channel);
  if (value > 511) value = 511;
  return value * 2;
}

void loop()
{
  uint16_t speedPoti;
  //uint16_t speedPoti = analogReadScaled(POTI_LEFT);
  speedPoti = analogReadScaled(POTI_RIGHT)>>2;
  pauseBetweenNotes=speedPoti*110/100;
  noteDuration_ms = speedPoti;

  //list any chord sequence here by calling playArp(chord)

  playMart(c);

// wait for button
    while( getButton()==0);

}
