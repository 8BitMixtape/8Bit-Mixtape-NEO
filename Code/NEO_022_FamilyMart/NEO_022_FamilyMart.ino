/* 
=========================================================================================================
 _____  ______ _ _      ___  ____      _                       _   _  _____ _____ 
|  _  | | ___ (_) |     |  \/  (_)    | |                     | \ | ||  ___|  _  |
 \ V /  | |_/ /_| |_    | .  . |___  _| |_ __ _ _ __   ___    |  \| || |__ | | | |
 / _ \  | ___ \ | __|   | |\/| | \ \/ / __/ _` | '_ \ / _ \   | . ` ||  __|| | | |
| |_| | | |_/ / | |_    | |  | | |>  <| || (_| | |_) |  __/   | |\  || |___\ \_/ /
\_____/ \____/|_|\__|   \_|  |_/_/_/\_\\__\__,_| .__/ \___|   \_| \_/\____/ \___/ 
                                             | |                              
     https://8bitmixtape.github.io/          |_|                    
    .------------------------------.                              ATTINY85 Pins
    |↓↓\ 0  0  0  0  0  0  0  0 /↑↑|                              =============
    |   \___8Bit Mixtape NEO___/   |                                 _______
    |        __  ______  __        |                                |   U   |                                     
    |       /  \|\.....|/  \       |       (SYNC-OUT) <- D5/A0  PB5-|       |- VCC                                
    |       \__/|/_____|\__/       |   SD -> *SD-prog -> D3/A3  PB3-| ATTINY|- PB2  D2/A1 <- POTI_RIGHT
    |   ________________________   |   POTI_LEFT / CV -> D4/A2  PB4-|   85  |- PB1  D1    -> PWM SOUND
    |  |v2.0 hardware by dusjagr|  |                            GND-|       |- PB0  D0    -> NEOPIXELS
    '=============================='                                |_______|

  * based on TinyAudioBoot and hex2wav by Chris Haberer, Fredrik Olofsson, Budi Prakosa
    https://github.com/ChrisMicro/AttinySound
    
=========================================================================================================
The Center for Alternative Coconut Research presents:
-----------------------------------------------------
  Family Mart Chime
  Description:
  Plays arpeggios of chosen chords
  Arpeggio
  from http://fonte.me/arduino/#Arpeggio

*********************************************************************************************************
  This code is released under the MIT license
  http://opensource.org/licenses/MIT

********************************************* list of outhors *******************************************
  Copyright (c) 2012 Jeremy Fonte
  code addapted to NEO: ChrisMicro
  code adapted to play other tune: dusjagr
     _          _             
   _| |_ _ ___ |_|___ ___ ___ 
  | . | | |_ -|| | .'| . |  _|
  |___|___|___|| |__,|_  |_|  
             |___|   |___|    

****************************************************************************************************** */

#include "pitches.h"

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
#define Vcc            37 // 3.7 V for LiPo
#define Vdiv           26 // measure max Voltage on Analog In

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// fast pin access
#define AUDIOPIN (1<<SPEAKERPIN)
#define PINLOW (PORTB&=~AUDIOPIN)
#define PINHIGH (PORTB|=AUDIOPIN)

// variables
uint16_t speedPoti;
uint16_t colorPoti;
byte buttonState1 = LOW; 
byte lastButtonState1 = LOW;
byte buttonCount1 = 0;
byte buttonReset1 = 4;

byte buttonState2 = LOW; 
byte lastButtonState2 = LOW;
byte buttonCount2 = 0;
byte buttonReset2 = 4;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Specific functions of the 8Bit Mixtape NEO
 ___ _____ _ _      _____ _     _                  _____ _____ _____ 
| . | __  |_| |_   |     |_|_ _| |_ ___ ___ ___   |   | |   __|     |
| . | __ -| |  _|  | | | | |_'_|  _| .'| . | -_|  | | | |   __|  |  |
|___|_____|_|_|    |_|_|_|_|_,_|_| |__,|  _|___|  |_|___|_____|_____|
                                       |_|                           
========================================================================================================================
   _________    
  | 8Bit(x) |   uint8_t   getButton()                       -> 1 left, 2 right  3 both 
  |  o___o  |   uint16_t  analogReadScaled(uint8_t channel) -> scale the adc of the voltage divider to a return value: 0..1023     
  |__/___\__|   
                
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

uint8_t getButton()
{
  uint8_t button = 0;
  if (analogRead(BUTTONS_ADC) < Vbutton_left) button = 1;
  if (analogRead(BUTTONS_ADC) < Vbutton_right) button = 2;
  if (analogRead(BUTTONS_ADC) < Vbutton_both) button = 3;

  return button;
}

uint8_t getButtonState()
{
  uint8_t state = 0;
  int x = getButton();
  
  if (x == 0) {
    buttonState1 = LOW; //high means not pressed
    buttonState2 = LOW; 
  }
  if (x == 1) buttonState1 = HIGH; // low means pressed
  if (x == 2) buttonState2 = HIGH;

  if (buttonState1 != lastButtonState1 && buttonState1 == LOW) {
    if (buttonState2 == LOW){
    //do this on press of button 1
       buttonCount1++;
       if (buttonCount1 >= buttonReset1) {
        buttonCount1 = 0;
       }
    }
  }
  
  if (buttonState2 != lastButtonState2 && buttonState2 == LOW) {
    if (buttonState1 == LOW){
    //do this on press of button 2
       buttonCount2++;
       if (buttonCount2 >= buttonReset2) {
        buttonCount2 = 0;
       }
    }
  }

  return state;
}

uint16_t analogReadScaled(uint8_t channel)
{
  uint16_t value = analogRead(channel);
  value = value * Vcc / Vdiv;
  if (value > 1023) value = 1023;
  return value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Specific functions of the 8Bit Mixtape NEO
 _____         _   _                    _    _____               _     
|   __|_ _ ___| |_| |_ ___    ___ ___ _| |  |   __|___ _ _ ___ _| |___ 
|__   | | |   |  _|   |_ -|  | .'|   | . |  |__   | . | | |   | . |_ -|
|_____|_  |_|_|_| |_|_|___|  |__,|_|_|___|  |_____|___|___|_|_|___|___|
      |___|                                                                                    
========================================================================================================================
   _________    
  | 8Bit(x) |   void playSound(long freq_Hz, long duration_ms)
  |  o___o  |   void playMart(int notes[])    -> see below
  |__/___\__|   
               
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void playSound(long freq_Hz, long duration_ms)
{
  uint16_t n;
  uint32_t delayTime_us;
  uint32_t counts;

  delayTime_us = 1000000UL / (freq_Hz*colorPoti>>6);
  counts = duration_ms * 1000 / delayTime_us;

  for (n = 0; n < counts; n++)
  {
    PINHIGH;
    delayMicroseconds(delayTime_us);
    PINLOW;
    delayMicroseconds(delayTime_us);
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
    speedPoti = 1023 - analogReadScaled(POTI_RIGHT)>>2;
    colorPoti = analogReadScaled(POTI_LEFT)>>2;
    pauseBetweenNotes=speedPoti*110/100;
    noteDuration_ms = speedPoti;
    displayBinrayValue(notes[thisNote],Wheel(colorPoti));
    pixels.show(); // This sends the updated pixel color to the hardware.
    
    playSound( notes[thisNote], noteDuration_ms);
    setColorAllPixel(0); // pixels off
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    delay(pauseBetweenNotes);
        
  }
  delay(pauseBetweenNotes*5);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Specific functions of the NEO-Pixel Library
 _____ _____ _____        _         _ 
|   | |   __|     |   ___|_|_ _ ___| |
| | | |   __|  |  |  | . | |_'_| -_| |
|_|___|_____|_____|  |  _|_|_,_|___|_|
                     |_|              
========================================================================================================================
   _________    
  | NEO(x)  |   void setColorAllPixel(uint32_t color)                   -> Sets all the pixels to the same color
  |  o___o  |   void displayBinrayValue(uint16_t value, uint32_t color) -> displays binary number
  |__/___\__|   uint32_t Wheel(byte WheelPos)                           -> Input a value 0 to 255 to get a color value.   
                                                                        The colours are a transition r - g - b - back to r. 
                void rainbowCycle(uint8_t wait, uint8_t rounds)         -> makes a Rainbow :-)  
                
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void setColorAllPixel(uint32_t color)
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    pixels.setPixelColor(n, color);
  }
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
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setBrightness(40);
  pinMode(SPEAKERPIN, OUTPUT);

}

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
  //uint16_t speedPoti = analogReadScaled(POTI_LEFT);
  speedPoti = analogReadScaled(POTI_RIGHT)>>2;
  pauseBetweenNotes=speedPoti*110/100;
  noteDuration_ms = speedPoti;

  //read the buttons
  uint8_t x = getButtonState();
  
  if (buttonCount2 == 1)  {
    playMart(c); //
  }
    
  setColorAllPixel (0);
  pixels.setPixelColor(buttonCount1, Wheel(170));
  pixels.setPixelColor(NUMPIXELS-buttonCount2-1, Wheel(220));
  pixels.show(); // This sends the updated pixel color to the hardware.
    
  lastButtonState1 = buttonState1;
  lastButtonState2 = buttonState2;
  
}

