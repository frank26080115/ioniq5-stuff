#include "AsyncAdc.h"

#include <Arduino.h>

#include "esp32-hal-adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
#include "esp_attr.h"
#include "esp_intr.h"
#include "soc/rtc_io_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"

// code copied from https://github.com/espressif/arduino-esp32/blob/eb46978a8d96253309148c71cc10707f2721be5e/cores/esp32/esp32-hal-adc.c

bool IRAM_ATTR adcAttachPin(uint8_t pin){

    int8_t channel = digitalPinToAnalogChannel(pin);
    if(channel < 0){
        return false;//not adc pin
    }

    int8_t pad = digitalPinToTouchChannel(pin);
    if(pad >= 0){
        uint32_t touch = READ_PERI_REG(SENS_SAR_TOUCH_ENABLE_REG);
        if(touch & (1 << pad)){
            touch &= ~((1 << (pad + SENS_TOUCH_PAD_OUTEN2_S))
                    | (1 << (pad + SENS_TOUCH_PAD_OUTEN1_S))
                    | (1 << (pad + SENS_TOUCH_PAD_WORKEN_S)));
            WRITE_PERI_REG(SENS_SAR_TOUCH_ENABLE_REG, touch);
        }
    } else if(pin == 25){
        CLEAR_PERI_REG_MASK(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_XPD_DAC | RTC_IO_PDAC1_DAC_XPD_FORCE);//stop dac1
    } else if(pin == 26){
        CLEAR_PERI_REG_MASK(RTC_IO_PAD_DAC2_REG, RTC_IO_PDAC2_XPD_DAC | RTC_IO_PDAC2_DAC_XPD_FORCE);//stop dac2
    }

    pinMode(pin, ANALOG);

    __analogInit();
    return true;
}

bool IRAM_ATTR adcStart(uint8_t pin){

    int8_t channel = digitalPinToAnalogChannel(pin);
    if(channel < 0){
        return false;//not adc pin
    }

    if(channel > 9){
        channel -= 10;
        CLEAR_PERI_REG_MASK(SENS_SAR_MEAS_START2_REG, SENS_MEAS2_START_SAR_M);
        SET_PERI_REG_BITS(SENS_SAR_MEAS_START2_REG, SENS_SAR2_EN_PAD, (1 << channel), SENS_SAR2_EN_PAD_S);
        SET_PERI_REG_MASK(SENS_SAR_MEAS_START2_REG, SENS_MEAS2_START_SAR_M);
    } else {
        CLEAR_PERI_REG_MASK(SENS_SAR_MEAS_START1_REG, SENS_MEAS1_START_SAR_M);
        SET_PERI_REG_BITS(SENS_SAR_MEAS_START1_REG, SENS_SAR1_EN_PAD, (1 << channel), SENS_SAR1_EN_PAD_S);
        SET_PERI_REG_MASK(SENS_SAR_MEAS_START1_REG, SENS_MEAS1_START_SAR_M);
    }
    return true;
}

bool IRAM_ATTR adcBusy(uint8_t pin){

    int8_t channel = digitalPinToAnalogChannel(pin);
    if(channel < 0){
        return false;//not adc pin
    }

    if(channel > 7){
        return (GET_PERI_REG_MASK(SENS_SAR_MEAS_START2_REG, SENS_MEAS2_DONE_SAR) == 0);
    }
    return (GET_PERI_REG_MASK(SENS_SAR_MEAS_START1_REG, SENS_MEAS1_DONE_SAR) == 0);
}

uint16_t IRAM_ATTR adcEnd(uint8_t pin)
{

    uint16_t value = 0;
    int8_t channel = digitalPinToAnalogChannel(pin);
    if(channel < 0){
        return 0;//not adc pin
    }
    if(channel > 7){
        while (GET_PERI_REG_MASK(SENS_SAR_MEAS_START2_REG, SENS_MEAS2_DONE_SAR) == 0); //wait for conversion
        value = GET_PERI_REG_BITS2(SENS_SAR_MEAS_START2_REG, SENS_MEAS2_DATA_SAR, SENS_MEAS2_DATA_SAR_S);
    } else {
        while (GET_PERI_REG_MASK(SENS_SAR_MEAS_START1_REG, SENS_MEAS1_DONE_SAR) == 0); //wait for conversion
        value = GET_PERI_REG_BITS2(SENS_SAR_MEAS_START1_REG, SENS_MEAS1_DATA_SAR, SENS_MEAS1_DATA_SAR_S);
    }
    return value;
}
