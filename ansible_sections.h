#pragma once

#include "deserialize_jsmn.h"

#define ANSIBLE_SECTION_CT 2
#define ANSIBLE_FIRMWARE_NAME "ansible"
#define ANSIBLE_VERSION "1.6.1-dev"

preset_section_handler_t ansible_sections[ANSIBLE_SECTION_CT];
preset_section_handler_t ansible_shared_handlers[];
preset_section_handler_t ansible_meta_handlers[];
preset_section_handler_t ansible_tt_handlers[];

preset_read_result_t ansible_read_meta_section(jsmntok_t token, 
											   nvram_data_t* nvram, child_state_t* s, void* handler_def,
											   const char* text, size_t text_len, size_t dst_offset);
preset_read_result_t ansible_read_shared_section(jsmntok_t token,
												 nvram_data_t* nvram, child_state_t* s, void* handler_def,
												 const char* text, size_t text_len, size_t dst_offset);
