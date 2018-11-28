#pragma once

#include "deserialize_jsmn.h"

#include "greatest/greatest.h"

char buf[128];
jsmntok_t tokens[8];
nvram_data_t nvram;
load_object_state_t object_state;

preset_section_handler_t* find_app_handler(char* name);

FILE* write_temp_file(const char* name, const char* text, size_t len);

char cmp_buf[2][4096];
bool compare_files(const char* left, const char* right);

bool compare_short_big_endian(uint16_t* big_endian, uint16_t* native_order, size_t len);
