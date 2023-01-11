#ifndef _ASYNCADC_H_
#define _ASYNCADC_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// code copied from https://github.com/espressif/arduino-esp32/blob/eb46978a8d96253309148c71cc10707f2721be5e/cores/esp32/esp32-hal-adc.c

extern bool adcAttachPin(uint8_t pin);// __attribute__ ((weak, alias("__adcAttachPin")));
extern bool adcStart(uint8_t pin);// __attribute__ ((weak, alias("__adcStart")));
extern bool adcBusy(uint8_t pin);// __attribute__ ((weak, alias("__adcBusy")));
extern uint16_t adcEnd(uint8_t pin);// __attribute__ ((weak, alias("__adcEnd")));

#ifdef __cplusplus
}
#endif

#endif
