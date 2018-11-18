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

typedef enum {
	MATCH_START_OBJECT,
	MATCH_SECTION_NAME,
	MATCH_PARSE_SECTION,
} preset_object_phase_t;

typedef struct {
	bool fresh;
	void* state;
} child_state_t;

typedef preset_read_result_t(*preset_reader_cb)(jsmntok_t token,
	nvram_data_t* nvram, child_state_t* s,
	const char* text, size_t text_len);
typedef void(*preset_writer_cb)();

typedef struct
{
	char name[8];
	preset_reader_cb read;
	preset_writer_cb write;
	child_state_t child_state;
} preset_section_handler_t;

typedef struct {
	preset_read_phase_t phase;
	jsmn_parser jsmn;

    size_t text_ct;
	int tok_ct;
	int curr_tok;

	child_state_t child_state;
} preset_read_state_t;

typedef struct {
	preset_object_phase_t match_state;
    preset_section_handler_t* active_handler;
	uint8_t sections_handled;
} preset_object_state_t;

typedef enum {
	ARRAY_MATCH_KEY,
	ARRAY_MATCH_START,
	ARRAY_MATCH_ITEMS,
} array_state_t;

typedef struct {
	array_state_t array_state;
	size_t buf_pos;
	uint8_t array_ct;
} preset_bufarray_state_t;

int decode_decimal(const char* s, int len);
int decode_hexbuf(char* dst, const char* src, size_t len);

preset_read_result_t handle_object(jsmntok_t tok,
								   nvram_data_t* nvram, child_state_t* s,
								   const char* text, size_t text_len,
								   preset_section_handler_t* handlers, uint8_t handler_ct);

preset_read_result_t preset_deserialize(FILE* fp, nvram_data_t* nvram, void* child_state_storage, 
 										preset_section_handler_t* handlers, uint8_t handler_ct, 
										char* textbuf, size_t textbuf_len, 
 										jsmntok_t* tokbuf, size_t tokbuf_len); 
