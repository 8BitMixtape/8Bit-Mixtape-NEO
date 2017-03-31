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
  8Pixel-OneLiners for the hardware "8Bit Mixtape NEO"
  Description:

  TO BE DONE!!!

*********************************************************************************************************
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

********************************************* list of outhors *******************************************

  v0.x  01.01.1999 -D-U-S-J-A-G-R-  See 8bit mixtape on SGMK wiki
  v0.x  01.01.1999 -H-A-B-E-R-E-R-  various routines to read the 8Bit Mixtape NEO
  v1.0  13.03.2017 -D-U-S-J-A-G-R-  adapted to new schematics 0.95. different resistor values

  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something
   _          _             
 _| |_ _ ___ |_|___ ___ ___ 
| . | | |_ -|| | .'| . |  _|
|___|___|___|| |__,|_  |_|  
           |___|   |___|    

****************************************************************************************************** */

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

// Initialize the NEO pixel library
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Specific functions of the 8Bit Mixtape NEO
========================================================================================================================

   _________    uint8_t   getButton()  
  | 8Bit(x) |   -> 1 left, 2 right  3 both 
  |  o___o  | 
  |__/___\__|   uint16_t  analogReadScaled(uint8_t channel)          
                -> scaled the adc of the voltage divider to a return value: 0..1023
              
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

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
uint16_t analogReadScaled(uint8_t channel)
{
  uint16_t value = analogRead(channel);
  if (value > 511) value = 511;
  return value * 2;
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
  
  pixels.begin(); // This initializes the NeoPixel library.
  pinMode(SPEAKERPIN, OUTPUT);
  //pixels.setBrightness(255);
  //rainbowCycle(3,3);
  //delay (50);
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
  // check left pot
  uint16_t valuePotiLeft = analogReadScaled(POTI_LEFT);
  bright = 30;
  //bright = valuePotiLeft >> 6;
  
  
  uint8_t x = getButton();

  if (x == 1) {
    buttonState1 = LOW; 
  }
  if (x == 2) {
    setColorAllPixel(0);
    pixels.show();
    visuals = 2;
  }
  
  if (x == 0) {
    buttonState1 = HIGH; 
  }
  
  if (buttonState1 != lastButtonState1 && buttonState1 == HIGH) {
    // if the state has changed, increment the counter
      count++;
      pixels.setPixelColor(count, Wheel(count+7<<5));
      pixels.show();
      t = 0; 
      if (count >= 8) {
        count = 0;
      } 
    }
    
  lastButtonState1 = buttonState1;

  // check if potentiometer is all the way down
  uint16_t valuePoti = analogReadScaled(POTI_RIGHT);
  p1 = 1023 - valuePoti>>5;
  
  if (valuePoti < 10) {
    buttonState2 = LOW;
    if (outputStyle == 1) {
      pixels.setBrightness(bright);
      setColorAllPixel(Wheel(count+7<<5));
      pixels.show();
      delay(10000);
      }
    if (outputStyle == 0) {
      pixels.setBrightness(bright);
      rainbowCycle(6,2);
      setColorAllPixel(Wheel(count+7<<5));
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
    setColorAllPixel(Wheel(count+7<<5));
    pixels.setBrightness(bright);
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
      setColorAllPixel(Wheel((count+7)<<5));
      pixels.setPixelColor(count, 0xFFFFFF);
      pixels.setBrightness(((snd+30) & bright));
      pixels.show();
    }
  }
  
  if (visuals == 1) {
    if (t % 1  == 0) {
      setColorAllPixel(Wheel(snd % 255));
      pixels.setPixelColor(count, Wheel(count+7<<5));
      pixels.setBrightness(((snd+30) & bright));
      pixels.show();
    }
  }  

  if (visuals == 2) {
    if (t % 255  == 0) {
    setColorAllPixel(0);
    pixels.setPixelColor(count+1, Wheel(count+7<<5));
    }
    //Confused why 2 pixels are lit...
  }  

//wait... this defines the speed. could be read while press & hold with poti
  delayMicroseconds(valuePotiLeft>>2);
  t++;
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Specific functions of the NEO-Pixel Library
========================================================================================================================

   _________    void rainbowCycle(uint8_t wait, uint8_t rounds) -> makes a Rainbow :-)
  | NEO(x)  |   void setColorAllPixel(uint32_t color)           -> Sets all the pixels to the same color
  |  o___o  |   uint32_t Wheel(byte WheelPos)                   -> Input a value 0 to 255 to get a color value.   
  |__/___\__|                                                      The colours are a transition r - g - b - back to r.
                   
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

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

void setColorAllPixel(uint32_t color)
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    pixels.setPixelColor(n, color); // off
  }
}
