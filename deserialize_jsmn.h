#pragma once

#include <stdio.h>
#include <string.h>

#include "jsmn/jsmn.h"
#include "nvram.h"

typedef enum
{
  PRESET_READ_OK,
  PRESET_READ_INCOMPLETE,
  PRESET_READ_MALFORMED,
} preset_read_result_t;

typedef enum
{
  PRESET_READ_START,
  PRESET_READ_SECTIONS,
  PRESET_READ_DONE,
} preset_read_phase_t;

typedef preset_read_result_t(*preset_reader_cb)(jsmntok_t token,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len, size_t dst_offset);
typedef void(*preset_writer_cb)();

typedef struct
{
	char* name;
	preset_reader_cb read;
	preset_writer_cb write;
	bool fresh;
	void* state;
	void* params;
} preset_section_handler_t;

typedef struct {
	preset_read_phase_t phase;
	jsmn_parser jsmn;

    size_t text_ct;
	int tok_ct;
	int curr_tok;
} preset_read_state_t;

int decode_decimal(const char* s, int len);
int decode_hexbuf(char* dst, const char* src, size_t len);

preset_read_result_t load_object(jsmntok_t tok,
								   nvram_data_t* nvram, void* handler_def,
								   const char* text, size_t text_len, size_t dst_offset);
typedef enum {
	MATCH_START_OBJECT,
	MATCH_SECTION_NAME,
	MATCH_PARSE_SECTION,
} object_state_t;
typedef struct {
	preset_section_handler_t* handlers;
	uint8_t handler_ct;
} load_object_params_t;
typedef struct {
	object_state_t object_state;
	preset_section_handler_t* active_handler;
	uint8_t sections_handled;
} load_object_state_t;

preset_read_result_t load_array(jsmntok_t tok,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len, size_t dst_offset);
typedef enum {
	ARRAY_MATCH_START,
	ARRAY_MATCH_ITEMS,
} array_state_t;
typedef struct {
	size_t array_len;
	size_t item_size;
	preset_section_handler_t* item_handler;
} load_array_params_t;
typedef struct {
	array_state_t array_state;
	size_t array_ct;
} load_array_state_t;

preset_read_result_t load_scalar(jsmntok_t tok,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len, size_t dst_offset);
typedef struct {
	size_t dst_offset;
	uint8_t dst_size;
} load_scalar_params_t;

preset_read_result_t match_string(jsmntok_t tok,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len, size_t dst_offset);
typedef struct {
	const char* to_match;
} match_string_params_t;

preset_read_result_t load_buffer(jsmntok_t tok,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len, size_t dst_offset);
typedef struct {
	size_t buf_len;
	size_t dst_offset;
} load_buffer_params_t;
typedef struct {
	size_t buf_pos;
} load_buffer_state_t;

preset_read_result_t preset_deserialize(FILE* fp, nvram_data_t* nvram,
										preset_section_handler_t* handler, 
										char* textbuf, size_t textbuf_len, 
 										jsmntok_t* tokbuf, size_t tokbuf_len); 
