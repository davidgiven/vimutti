#ifndef GLOBALS_H
#define GLOBALS_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

extern void fatal(const char* message, ...);
extern void load_file(const char* filename, uint8_t** data, uint32_t* length);
extern uint32_t crc16(uint16_t poly, uint16_t crc, uint8_t* data, uint32_t length);
extern uint16_t read16(uint8_t* ptr);
extern uint32_t read32(uint8_t* ptr);

extern void cmd_unpackapp(int argc, char* const* argv);
extern void cmd_unpackimg(int argc, char* const* argv);

#endif

