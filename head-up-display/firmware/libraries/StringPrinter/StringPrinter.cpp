#include "StringPrinter.h"

StringPrinter::StringPrinter(uint16_t sz)
{
    this->cache_size = sz;
    this->cache = (uint8_t*)malloc(sz);
    this->cache_idx = 0;
}

void StringPrinter::reset()
{
    this->cache_idx = 0;
    this->cache[0] = 0;
}

char* StringPrinter::c_str()
{
    return (char*)this->cache;
}

size_t StringPrinter::write(uint8_t c)
{
    if (this->cache_idx < this->cache_size - 3)
    {
        this->cache[this->cache_idx] = c;
        this->cache[this->cache_idx + 1] = 0;
        this->cache_idx += 1;
        return 1;
    }
    return 0;
}

size_t StringPrinter::write(const uint8_t* buffer, size_t size)
{
    int i;
    if (this->cache_idx + size >= this->cache_size - 3)
    {
        size -= (this->cache_idx + size) - (this->cache_size - 3);
    }
    for (i = 0; i < size; i++)
    {
        this->cache[this->cache_idx] = buffer[i];
        this->cache_idx += 1;
    }
    this->cache[this->cache_idx] = 0;
    return i;
}
