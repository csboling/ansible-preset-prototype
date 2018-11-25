#include "ansible_sections.h"
#include "ansible_preset_internal.h"

preset_object_state_t ansible_read_object_state;
load_buffer_state_t ansible_load_buffer_state;
load_array_state_t ansible_load_array_state;
preset_section_handler_t ansible_sections[ANSIBLE_SECTION_CT] = {
	{
		.name = "meta",
		.read = ansible_read_meta_section,
		.child_state = {
			.state = &ansible_read_object_state,
		},
	},
	{
		.name = "shared",
		.read = ansible_read_shared_section,
		.child_state = {
			.state = &ansible_read_object_state,
		},
	},
};
preset_section_handler_t ansible_meta_handlers[] = {
	{
		.name = "firmware",
		.read = ansible_match_firmware_name,
	},
	{
		.name = "version",
		.read = ansible_match_version,
	},
	{
		.name = "i2c_addr",
		.read = load_scalar,
		.params = &((load_scalar_params_t){
			.dst_offset = offsetof(nvram_data_t, state.i2c_addr),
			.dst_size = sizeof(uint8_t),
		}),
	},
};
preset_section_handler_t ansible_shared_handlers[] = {
	{
		.name = "scales",
		.read = load_array,
		.child_state = {
			.state = &ansible_load_array_state,
		},
		.params = &((load_array_params_t) {
			.array_len = 16,
			.item_size = sizeof(uint8_t[8]),
			.item_handler = &((preset_section_handler_t) {
				.read = load_buffer,
				.child_state = {
					.state = &ansible_load_buffer_state,
				},
				.params = &((load_buffer_params_t) {
					.buf_len = 8,
					.dst_offset = offsetof(nvram_data_t, scale),
				}),
			})
		})
	},
};
preset_section_handler_t ansible_tt_handlers[] = {
	{
		.name = "clock_period",
	},
};

/////////
// meta

preset_read_result_t ansible_read_meta_section(jsmntok_t token,
	nvram_data_t* nvram, child_state_t* s, void* handler_def,
	const char* text, size_t text_len, size_t dst_offset)
{
	return handle_object(token,
		nvram, s,
		text, text_len,
		ansible_meta_handlers, sizeof(ansible_meta_handlers) / sizeof(preset_section_handler_t));
}

preset_read_result_t ansible_match_firmware_name(jsmntok_t token,
												 nvram_data_t* nvram, child_state_t* s, void* handler_def,
												 const char* text, size_t text_len, size_t dst_offset)
{
	if (token.type != JSMN_STRING) {
		return PRESET_READ_MALFORMED;
	}
	if (token.end < 0) {
		return PRESET_READ_INCOMPLETE;
	}
	if (strncmp(ANSIBLE_FIRMWARE_NAME, text + token.start, token.end - token.start) != 0) {
		return PRESET_READ_MALFORMED;
	}
	return PRESET_READ_OK;
}

preset_read_result_t ansible_match_version(jsmntok_t token,
										   nvram_data_t* nvram, child_state_t* s, void* handler_def,
										   const char* text, size_t text_len, size_t dst_offset)
{
	if (token.type != JSMN_STRING) {
		return PRESET_READ_MALFORMED;
	}
	if (token.end < 0) {
		return PRESET_READ_INCOMPLETE;
	}
	if (strncmp(ANSIBLE_VERSION, text + token.start, token.end - token.start) != 0) {
		return PRESET_READ_MALFORMED;
	}
	return PRESET_READ_OK;
}

////////
// shared

preset_read_result_t ansible_read_shared_section(jsmntok_t token,
												 nvram_data_t* nvram, child_state_t* s, void* handler_def,
												 const char* text, size_t text_len, size_t dst_offset)
{
	return handle_object(token,
		nvram, s,
		text, text_len,
		ansible_shared_handlers, sizeof(ansible_shared_handlers) / sizeof(preset_section_handler_t));
}

////////
// apps
