#ifndef _SERIALCMDLINE_H_
#define _SERIALCMDLINE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <Arduino.h>
#include <Stream.h>

#define SIDEBUFF_SIZE 16

typedef void (*cmd_handler_t)(void*, char*, Stream*);

typedef struct
{
    const char cmd_header[64];
    cmd_handler_t handler_callback;
}
cmd_def_t;

class SerialCmdLine
{
    public:
        SerialCmdLine(Stream* stream_obj, cmd_def_t* user_cmd_list, bool local_echo, char* prompt, char* unknown_reply, bool higher_priori, uint32_t buffer_size);
        void print_prompt(void);
        int task(void);
        int side_enter(char c);
        inline void set_echo(bool x) { this->_echo = x; };
    protected:
        Stream* _stream;
        cmd_def_t* _cmd_list;
        bool _echo;
        char* _prompt;
        char* _unknown_reply;
        uint8_t* _buffer;
        uint32_t _buffer_size;
        uint32_t _buff_idx;
        char _prev_char;
        bool _higher_priori;

        uint8_t _sidebuff[SIDEBUFF_SIZE];
        uint8_t _sidebuff_w = 0;
        uint8_t _sidebuff_r = 0;
};

#endif