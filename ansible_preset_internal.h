#pragma once

preset_read_result_t ansible_read_meta_section(jsmntok_t token,
											   nvram_data_t* nvram, child_state_t* s,
											   const char* text, size_t text_len);
preset_read_result_t ansible_match_firmware_name(jsmntok_t token,
												 nvram_data_t* nvram, child_state_t* s,
												 const char* text, size_t text_len);
preset_read_result_t ansible_match_version(jsmntok_t token,
										   nvram_data_t* nvram, child_state_t* s,
										   const char* text, size_t text_len);
preset_read_result_t ansible_save_i2c_addr(jsmntok_t token,
										   nvram_data_t* nvram, child_state_t* s,
										   const char* text, size_t text_len);

preset_read_result_t ansible_read_shared_section(jsmntok_t token,
												 nvram_data_t* nvram, child_state_t* s,
												 const char* text, size_t text_len);
preset_read_result_t ansible_save_scales(jsmntok_t token,
										 nvram_data_t* nvram, child_state_t* s,
										 const char* text, size_t text_len);
