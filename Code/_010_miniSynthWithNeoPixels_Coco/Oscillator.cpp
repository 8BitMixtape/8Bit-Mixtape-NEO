/*
Oscilator.cpp
 
low frequency oscillator with different waveforms

*************************************************************************************
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
********************************** list of outhors **********************************
v0.1  20.2.2017 C. -H-A-B-E-R-E-R-  initial version

It is mandatory to keep the list of authors in this code.
Please add your name if you improve/extend something
2017 ChrisMicro
*************************************************************************************
*/ 

#include "Oscillator.h"

void Oscillator::setFrequency_mHz(uint16_t frequency_mHz)
{
	freq_const = (uint32_t) frequency_mHz * 65536 / 1000 * samplingTime_ms/1000;
}

void Oscillator::setFrequency_Hz(uint16_t frequency_Hz)
{
	freq_const = (uint32_t) frequency_Hz * 65536 * samplingTime_ms/1000;
}

int8_t sinwerte[17] = {0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 106, 112, 117, 122, 125, 126, 127 };
	
void Oscillator::setWaveform(uint16_t waveType)
{
	wavetype=waveType;
}

void Oscillator::setPwmValue(uint8_t value)
{
	RectanglePwmValue = -value;
}

void Oscillator::setMinMax(int16_t minimum, int16_t maximum)
{
	int16_t temp;
	temp=maximum-minimum;
	outputValueMinimum=minimum+temp/2;
	envelope = temp;
}

Oscillator::Oscillator(uint16_t updateRate_ms)
{
	envelope = 255;
	setWaveform(SINUS);
	samplingTime_ms = updateRate_ms;
	setFrequency_mHz(1000);
	setMinMax(-127,127);
	setPwmValue(25);
}

#define FACTOR 4

// wert: 0..1023

void Oscillator::setWaveformFine(uint16_t wert)
{
	uint8_t wave;
	
	if (wert > 255*FACTOR)wert = 255*FACTOR;


	if (wert <= 60*FACTOR) {
		wave = NOISE;
	}
	if (wert > 60*FACTOR && wert <= 188*FACTOR) 
	{
		wave = RECTANGLE;
		RectanglePwmValue = (wert>>2)-60;
	}
	if (wert > 188*FACTOR) wave = SAWTOOTH;
	if (wert > 200*FACTOR) wave = TRIANGLE;
	if (wert > 220*FACTOR) wave = SINUS;

	wavetype = wave;
}
	
int16_t Oscillator::calcNewValue(void)
{
	uint8_t x;
	int8_t k;
	int16_t temp, temp2;
	static uint16_t noise = 0xAA;

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

	int32_t temp3;
	temp3 = (int32_t)temp * envelope;

	return temp3 / 256  + outputValueMinimum;

}
