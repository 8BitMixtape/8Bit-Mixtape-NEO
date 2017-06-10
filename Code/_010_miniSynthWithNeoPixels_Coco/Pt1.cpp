/*
 Pt1.cpp
 
 one pole low pass filter
 
 
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

#include "Pt1.h"


Pt1::Pt1(uint16_t t)
{
	T_param=t;
	K_param=0x100;

	y_old=0;
}

void Pt1::setT(uint16_t t)
{
	T_param=t;	
}

void Pt1::setOutput(int16_t y)
{
	y_old=y*0x10000;
}

int16_t Pt1::filter(int16_t input)
{
	int32_t x;
	int32_t temp;

	x=input;

	int16_t y;

	// low pass filter
	y=y_old>>16;
	temp=y_old+T_param*(x-y);
	y_old=temp;	
	
	y=y_old>>16;
	
	return ((int32_t)y*K_param)>>8;

}


