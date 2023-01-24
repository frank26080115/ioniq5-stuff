#ifndef _STRINGPRINTER_H_
#define _STRINGPRINTER_H_

#include <Arduino.h>

class StringPrinter : public Print
{
    public:
        StringPrinter(uint16_t sz);
        size_t write(uint8_t c);
        size_t write(const uint8_t *buffer, size_t size);
        void reset(void);
        char* c_str(void);
    protected:
        uint8_t* cache;
        uint16_t cache_idx;
        uint16_t cache_size;
};

#endif
