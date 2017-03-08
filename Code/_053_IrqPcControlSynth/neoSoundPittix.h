

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __NEO_SOUND_PITTIX__
  #define __NEO_SOUND_PITTIX__

  extern volatile uint8_t SoftwareTimer_uint8;  
  
  #define SINUS 0
  #define TRIANGLE 1
  #define SAWTOOTH 2
  #define RECTANGLE 3
  #define NOISE 4
  
  void beginNeoPittix ();
  void setFrequency(uint16_t frequency_Hz);
  void setAmplitude(uint8_t amp);
  void setWaveform(uint8_t waveType);
  void setWaveformFine(uint8_t wert);

  #endif // __NEO_SOUND_PITTIX__

#ifdef __cplusplus
}
#endif



