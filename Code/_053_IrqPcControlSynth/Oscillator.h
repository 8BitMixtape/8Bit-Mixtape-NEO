/*
 * Oscilator.h
 *
 * Created: 20.02.2017 06:55:02
 */ 


#ifndef OSCILATOR_H_
#define OSCILATOR_H_

#include "stdint.h"

// waveType
#define SINUS     0
#define TRIANGLE  1
#define SAWTOOTH  2
#define RECTANGLE 3
#define NOISE     4

class Oscillator
{
	private:

		uint16_t envelope;
		uint16_t freq_const;
		uint8_t  wavetype;
		uint8_t  RectanglePwmValue;
		uint16_t samplingTime_ms;
		int16_t  outputValueMinimum;
		uint16_t phase1, oldphase;
	
	public:
		Oscillator(uint16_t updateRate_ms);
		//virtual ~Oscillator();

		int16_t calcNewValue    (void);
		void    setWaveform     (uint16_t waveType);
		void    setWaveformFine (uint16_t wert);
		void    setFrequency_mHz(uint16_t frequency_mHz);
		void    setFrequency_Hz (uint16_t frequency_Hz);
		void    setPwmValue     (uint8_t value);
		void    setMinMax       (int16_t minimum, int16_t maximum);
	
};


#endif /* OSCILATOR_H_ */
