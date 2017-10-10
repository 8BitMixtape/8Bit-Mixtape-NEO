#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __INTEGER_SIN__
  #define __INTEGER_SIN__

  #include "stdint.h"
  #include <stdint.h>
  #include <avr/pgmspace.h>

  extern const int8_t PROGMEM  sintab[];
  int8_t sin_int8(uint8_t index);

  #endif // __INTEGER_SIN__

#ifdef __cplusplus
}
#endif
