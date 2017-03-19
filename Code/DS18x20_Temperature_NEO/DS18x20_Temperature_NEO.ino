#include <OneWire.h>
#include <Adafruit_NeoPixel.h>

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library


#define ONEWIREPIN   PB4
#define NEOPIXELPIN    0
#define NUMPIXELS      8

OneWire  ds(ONEWIREPIN);  // on pin 10 (a 4.7K resistor is necessary)

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Specific functions of the NEO-Pixel Library
 _____ _____ _____        _         _ 
|   | |   __|     |   ___|_|_ _ ___| |
| | | |   __|  |  |  | . | |_'_| -_| |
|_|___|_____|_____|  |  _|_|_,_|___|_|
                     |_|              
========================================================================================================================
   _________    
  | NEO(x)  |   void setWhiteAllPixel(uint32_t color)                   -> Sets all the pixels to the white level
  |  o___o  |   void displayBinrayValue(uint16_t value, uint32_t color) -> displays binary number
  |__/___\__|   uint32_t Wheel(byte WheelPos)                           -> Input a value 0 to 255 to get a color value.   
                                                                        The colours are a transition r - g - b - back to r. 
                                
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void setWhiteAllPixel(uint32_t color)
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    pixels.setPixelColor(n, color, color, color);
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

void displayBinrayValue(uint16_t value, uint32_t color)
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    if (value & (1 << n)) pixels.setPixelColor(n, color);
    //else pixels.setPixelColor(n,0); // off
  }
}

void rainbowCycle(uint8_t wait, uint8_t rounds, uint8_t rainbowPixels) {
  uint16_t i, j;

  for (j = 0; j < 256 * rounds; j++) { 
    for (i = 0; i < rainbowPixels; i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / rainbowPixels) + j) & 255));
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

  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  int16_t raw;
  int16_t showPixel = 0;
  int blinkeSpeed = 3;
  int updateSpeed = 1000;
  int lowTemp = 20;
  int maxTemp = 50;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* the setup routine runs once when you start the tape or press reset
========================================================================================================================
   _________            _                   
  | setup() |   ___ ___| |_ _ _ _ _ 
  |  o___o  |  |_ -| -_|  _| | | . |
  |__/___\__|  |___|___|_| |___|  _|
                               |_|    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


void setup(void) {
  //Serial.begin(9600);
  //Serial.println("reboot");

  int brightness = 85;
  
  pixels.begin();
  pixels.setBrightness(brightness);
  
  
  rainbowCycle(5,1,16);
  
  for ( int i = brightness; i > 0; i--) {         
    pixels.setBrightness(i);
    pixels.show();
    delay (5);
  }
  delay(200);
  setColorAllPixel(0);
  pixels.setBrightness(brightness);
  pixels.show(); // Initialize all pixels to 'off'
  delay(100);
  
  
  for ( int i = 0; i < 3; i++) { 
    setColorAllPixel(Wheel(220));       // all pixels set to pink using the Wheel() function to choose a color from the spectrum 0-255
    
    for ( int i = 0; i < brightness; i++) {  
      for (int n = 0; n < NUMPIXELS; n++){       
        pixels.setPixelColor(n,i,0,i);
      }
      pixels.show();
      delay (3);
    }
    for ( int i = brightness; i > 0; i--) {         
      for (int n = 0; n < NUMPIXELS; n++){       
        pixels.setPixelColor(n,i,0,i);
      }
      pixels.show();
      delay (3);
    }
    
  }
  delay(500);
  
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

void loop(void) {

  setColorAllPixel(0);
  for (int n = 1; n <= 7; n++)
  {
    pixels.setPixelColor(n, Wheel(-25+n*25));
  }
  pixels.show();

  if ( !ds.search(addr)) {
    //Serial.println("No more addresses.");
    //Serial.println();
    ds.reset_search();
    
    pixels.setPixelColor(0, Wheel(220));
    pixels.show();
    delay(updateSpeed/blinkeSpeed/4);
    return;
  }
  
  if (OneWire::crc8(addr, 7) != addr[7]) {
      //Serial.println("CRC is not valid!");
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  //delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad
  
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  
  int16_t recalc =  raw / 16;
  showPixel = map(recalc, lowTemp, maxTemp, 7, 1);
  if (showPixel < 1) showPixel = 1;
  if (showPixel > 7) showPixel = 7;
  /*
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" C");
  Serial.print("  Pixel Position = ");
  Serial.println(showPixel);
  */
  for ( int i = 0; i < blinkeSpeed; i++) { 
    pixels.setPixelColor(showPixel, 255,255,255);
    pixels.show();
    delay(updateSpeed/blinkeSpeed/2);
    for (int n = 1; n <= 7; n++) {
      pixels.setPixelColor(n, Wheel(-25+n*25));
    }
    pixels.show();
    delay(updateSpeed/blinkeSpeed/2);
  }
}
