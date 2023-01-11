#ifndef _MULTIDESTPRINTER_H_
#define _MULTIDESTPRINTER_H_

#include <Arduino.h>

#define MDESTPRINTER_DEST_CNT 4
#define MDESTPRINTER_CACHE_SIZE 1024

class MultiDestPrinter : public Print
{
    public:
        MultiDestPrinter();
        Print* destinations[MDESTPRINTER_DEST_CNT] = { NULL };
        size_t write(uint8_t c);
        size_t write(const uint8_t *buffer, size_t size);
        size_t write_to_all(const uint8_t *buffer, size_t size);
    protected:
        uint8_t cache[MDESTPRINTER_CACHE_SIZE];
        uint16_t cache_idx;
};

#endif
