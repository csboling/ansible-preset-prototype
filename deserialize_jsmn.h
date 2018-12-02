#pragma once

#include <stdio.h>
#include <string.h>

#include "jsmn/jsmn.h"
#include "nvram.h"

typedef enum {
  PRESET_READ_OK,
  PRESET_READ_INCOMPLETE,
  PRESET_READ_MALFORMED,
} preset_read_result_t;

typedef enum {
  PRESET_WRITE_OK,
  PRESET_WRITE_FAILURE,
} preset_write_result_t;

typedef enum
{
  PRESET_READ_START,
  PRESET_READ_SECTIONS,
  PRESET_READ_DONE,
} preset_read_phase_t;

typedef void(*write_buffer_fn)(const uint8_t* buf, size_t len);

typedef preset_read_result_t(*preset_reader_cb)(
	jsmntok_t token,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len,
	size_t dst_offset, unsigned int depth);
typedef preset_write_result_t(*preset_writer_cb)(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset);

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
	unsigned int curr_tok;
} preset_read_state_t;

char* encode_decimal_unsigned(uint32_t val);
char* encode_decimal_signed(int32_t val);
int32_t decode_decimal(const char* s, int len);
int decode_hexbuf(char* dst, const char* src, size_t len);
char encode_nybble(uint8_t val);

preset_read_result_t load_object(
	jsmntok_t tok,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len,
	size_t dst_offset, unsigned int depth);
preset_write_result_t save_object(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset);
typedef enum {
	MATCH_START_OBJECT,
	SKIP_SECTION,
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
	unsigned int depth;
} load_object_state_t;

preset_read_result_t load_array(
	jsmntok_t tok,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len,
	size_t dst_offset, unsigned int depth);
preset_write_result_t save_array(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset);
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
	const char* text, size_t text_len, size_t dst_offset, unsigned int depth);
preset_write_result_t save_number(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset);
preset_write_result_t save_bool(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset);
preset_write_result_t save_string(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset);
typedef struct {
	size_t dst_offset;
	uint8_t dst_size;
	bool signed_val;
} load_scalar_params_t;

preset_read_result_t match_string(jsmntok_t tok,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len, size_t dst_offset, unsigned int depth);
typedef struct {
	const char* to_match;
} match_string_params_t;

preset_read_result_t load_enum(jsmntok_t tok,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len, size_t dst_offset, unsigned int depth);
preset_write_result_t save_enum(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset);
typedef struct {
	uint8_t option_ct;
	const char** options;
	size_t dst_offset;
} load_enum_params_t;

preset_read_result_t load_buffer(jsmntok_t tok,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len, size_t dst_offset, unsigned int depth);
preset_write_result_t save_buffer(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset);
typedef struct {
	size_t dst_size;
	size_t dst_offset;
} load_buffer_params_t;
typedef struct {
	size_t buf_pos;
} load_buffer_state_t;

preset_read_result_t preset_deserialize(FILE* fp, nvram_data_t* nvram,
										preset_section_handler_t* handler, 
										char* textbuf, size_t textbuf_len, 
 										jsmntok_t* tokbuf, size_t tokbuf_len); 

preset_write_result_t preset_serialize(FILE* fp, nvram_data_t* nvram, preset_section_handler_t* handler);