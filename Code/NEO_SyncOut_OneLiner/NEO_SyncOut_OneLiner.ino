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
    |       /  \|\.....|/  \       |        (SYNC-IN) <- D5/A0  PB5-|       |- VCC                                
    |       \__/|/_____|\__/       |   SD -> *SD-prog -> D3/A3  PB3-| ATTINY|- PB2  D2/A1 <- POTI_RIGHT
    |   ________________________   |   POTI_LEFT / CV -> D4/A2  PB4-|   85  |- PB1  D1    -> PWM SOUND
    |  |v2.0 hardware by dusjagr|  |                            GND-|       |- PB0  D0    -> NEOPIXELS
    '=============================='                                |_______|

  * based on TinyAudioBoot and hex2wav by Chris Haberer, Fredrik Olofsson, Budi Prakosa
    https://github.com/ChrisMicro/AttinySound
    
=========================================================================================================

  SYNC-IN from Korg Volca, with interrupt on PB5
  - Volca sync-in sends four pulses in a beat, pulses are counter and every fourth triggers play-function
  - Play-function plays hardcoded oneliner sound function with t: 0-1000 
  - Neopixel blinking has an effect with timing, needs more testing
  - tested with Volca Sample
  
  NOTICE: Fuses set to disable reset for sync-in pin: 
  avrdude -P /dev/ttyACM0 -b 19200 -c avrisp -p t85 -U efuse:w:0xfe:m -U hfuse:w:0x5d:m -U lfuse:w:0xe1:m
  http://wiki.8bitmixtape.cc/#/0_3-Sync_to_KORG_and_PO

*********************************************************************************************************
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

********************************************* list of outhors *******************************************

  v0.x  01.01.1999 -D-U-S-J-A-G-R-  See 8bit mixtape on SGMK wiki
  v0.x  01.01.1999 -H-A-B-E-R-E-R-  various routines to read the 8Bit Mixtape NEO
  v1.0  13.03.2017 -D-U-S-J-A-G-R-  adapted to new schematics 0.95. different resistor values
  fork  18.02.2018 -KI-M-I-T-O-B0-  first test on sync-in interrupt, simplified oneliner patch

  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something
   _          _             
 _| |_ _ ___ |_|___ ___ ___ 
| . | | |_ -|| | .'| . |  _|
|___|___|___|| |__,|_  |_|  
           |___|   |___|    

****************************************************************************************************** */

#include <Adafruit_NeoPixel.h>
#include <avr/interrupt.h>

#ifdef __AVR__
#include <avr/power.h>
#endif

// hardware description / pin connections
#define SPEAKERPIN      1
#define NEOPIXELPIN     0
#define POTI_LEFT      A1
#define POTI_RIGHT     A2
#define NUMPIXELS      8

// fast pin access
#define AUDIOPIN (1<<SPEAKERPIN)
#define PINLOW (PORTB&=~AUDIOPIN)
#define PINHIGH (PORTB|=AUDIOPIN)

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// define variables for one-liners
int snd = 0; 
long t = 0;
uint16_t p1 = 2;

// sync-in interrupt trigger counter
volatile int trigger = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Specific functions of the 8Bit Mixtape NEO
========================================================================================================================

   _________    
  | 8Bit(x) |   uint16_t  analogReadScaled(uint8_t channel)  
  |  o___o  |   -> scaled the adc of the voltage divider to a return value: 0..1023
  |__/___\__|           
                       
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

// scaled the adc of the 8BitMixTape voltage divider
// to a return value in range: 0..1023

uint16_t analogReadScaled(uint8_t channel)
{
  uint16_t value = analogRead(channel);
  if (value > 511) value = 511;
  return value * 2;
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
  |  o___o  |   
  |__/___\__|  
               
                
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void setColorAllPixel(uint32_t color)
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    pixels.setPixelColor(n, color);
    pixels.show();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* the setup routine runs once when you start the tape or press reset
========================================================================================================================
                         _    
   _________            | |              
  | setup() |   ___  ___| |_ _   _ _ __  
  |  o___o  |  / __|/ _ \ __| | | | '_ \ 
  |__/___\__|  \__ \  __/ |_| |_| | |_) |
               |___/\___|\__|\__,_| .__/ 
                                  | |    
                                  |_|  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void setup() {
  
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1); //what is this?
#endif
  
TCCR0B = TCCR0B & B11111000 | B00000001; //no timer pre-scaler, fast PWM
pinMode(SPEAKERPIN, OUTPUT);

GIMSK = 0b00100000;    // Sync-in: turns on pin change interrupts
PCMSK = 0b00100000;    // Sync-in: turn on interrupts on pins PB5 (reset disabled fuses)
sei();                 // Sync-in: enables interrupts

pixels.begin(); // This initializes the NeoPixel library.
pixels.setBrightness(40);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* The main loop to put all your code
========================================================================================================================
                _  
   _________   | | 
  | loop()  |  | | ___   ___  _ __     
  |  o___o  |  | |/ _ \ / _ \| '_ \ 
  |__/___\__|  | | (_) | (_) | |_) |
               |_|\___/ \___/| .__/ 
                             | |   
                             |_|            
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
                        
void loop() {
  uint16_t valuePoti = analogReadScaled(POTI_RIGHT);
  p1 = 1023 - valuePoti>>5;

  if (trigger == 4) {     // Sync-in: Play every 4 pulses from Volca
    setColorAllPixel(pixels.Color(50, 50, 50)); // lights on
    play();
  }
}

void play() {
  for (uint16_t t = 0; t < 1000; t ++ ){ 
    uint16_t valuePotiLeft = analogReadScaled(POTI_LEFT);
    //snd = (t*(t>>8|t>>4))>>(t>>p1); // a classic
    //snd = t*t/p1; // a classic
    snd = (t|3) * ((t>>1|t>>6)&p1&t>>3);  // a classic (this works quite nicely with sync-in)
    //snd = t*((t>>12|t>>8)&63&t>>(p1>>3)); // a classic
    //snd = t * ((t>>p1|t>>3)&17&t>>9); // a classic
    //snd = t>>3&1?t>>4:-t>>(p1>>3) ; //dubStep
    //snd = ((((t/3)&(t>>11))|((t/5)&(t>>p1)))&7)*30; // Tribute to Klaus
    delayMicroseconds(valuePotiLeft>>2);
    digitalWrite (SPEAKERPIN, snd);
  }
  setColorAllPixel(pixels.Color(0, 0, 0)); // lights off
}

ISR(PCINT0_vect)
{
    trigger++;            // Sync-in: interrupt, increment volatile variable
    if (trigger >= 5) {   // Sync-in: Count four pulses from Volca to one beat 
      trigger = 1;
    } 
}



