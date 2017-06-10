/* 
=========================================================================================================
 _____  ______ _ _      ___  ____      _                       _   _  _____ _____ 
|  _  | | ___ (_) |     |  \/  (_)    | |                     | \ | ||  ___|  _  |
 \ V /  | |_/ /_| |_    | .  . |___  _| |_ __ _ _ __   ___    |  \| || |__ | | | |
 / _ \  | ___ \ | __|   | |\/| | \ \/ / __/ _` | '_ \ / _ \   | . ` ||  __|| | | |
| |_| | | |_/ / | |_    | |  | | |>  <| || (_| | |_) |  __/   | |\  || |___\ \_/ /
\_____/ \____/|_|\__|   \_|  |_/_/_/\_\\__\__,_| .__/ \___|   \_| \_/\____/ \___/ 
                                             | |                              
    https://8bitmixtape.github.io/           |_|                    

  * based on TinyAudioBoot and hex2wav by Chris Haberer, Fredrik Olofsson, Budi Prakosa
    https://github.com/ChrisMicro/AttinySound
  * needs the _8Bit Mixtape library
    https://github.com/8BitMixtape/_8Bit-Mixtape-NEO-Lib
    
=========================================================================================================
The Center for Alternative Coconut Research presents:
-----------------------------------------------------
  Simple Blink of all NEO-Pixels
  Description:

*********************************************************************************************************
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
********************************************* list of outhors *******************************************
  v0.1  14.03.2017 -H-A-B-E-R-E-R-  first simple blink example for 8Bit Mixtape NEO using neolib.h
  v0.2  15.03.2017 -D-U-S-J-A-G-R-  adding some style and make them Pink
  v0.3  07.06.2017 -D-U-S-J-A-G-R-  Improving the lib description and NeoCoco

  It is mandatory to keep the list of authors in this code.
  Please add your name if you improve/extend something
     _          _             
   _| |_ _ ___ |_|___ ___ ___ 
  | . | | |_ -|| | .'| . |  _|
  |___|___|___|| |__,|_  |_|  
             |___|   |___|    
    
****************************************************************************************************** */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Start of the code
======================================================================================================================*/

#include "neolib.h"

uint8_t  ColorPosition = 0;
uint32_t Colors[] = {COLOR_PINK, COLOR_GREEN, COLOR_RED, COLOR_BLUE, COLOR_WHITE,COLOR_ORANGE, COLOR_TURQUOISE, COLOR_OCHRE};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* the setup routine runs once when you start the tape or press reset
========================================================================================================================
   _________            _                   
  | setup() |   ___ ___| |_ _ _ _ _    -> add this: neobegin(); to get these functions
  |  o___o  |  |_ -| -_|  _| | | . |      
  |__/___\__|  |___|___|_| |___|  _|      getButton(); or wasButtonPressed();
                               |_|        getPoti(uint8_t channel);
                                           
#define POTI_LEFT A2
#define BUTTON_LEFT 1
#define BUTTON_RIGHT 2

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void setup() 
{
  neobegin();
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

  uint16_t lp = getPoti( POTI_LEFT );   // range 0..1023
  uint8_t b = wasButtonPressed();
  
  if( b == BUTTON_LEFT                )     ;
  if( b == BUTTON_RIGHT               )     ;  
  if( b == BUTTON_RIGHT + BUTTON_LEFT )   ColorPosition++ ;
  ColorPosition &= 0b00000111; // prevent out of range
  
  setColorAllPixel(Colors[ColorPosition]);  // all pixels set to pink using the Wheel() function to choose a color from the spectrum 0-255
  pixels.show();                      // This sends the updated pixel color to the hardware.
  delay(lp);                        // wait for a while

  setColorAllPixel(0);                // turn all pixels off  
  pixels.show();                      // This sends the updated pixel color to the hardware.
  delay(lp);                        // wait for a while

}
