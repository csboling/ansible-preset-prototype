#include "ansible_sections.h"
#include "ansible_preset_internal.h"

preset_object_state_t ansible_read_object_state;
preset_bufarray_state_t ansible_bufarray_state;
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
		.read = ansible_save_scales,
		.child_state = {
			.state = &ansible_bufarray_state,
		},
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
	const char* text, size_t text_len)
{
	return handle_object(token,
		nvram, s,
		text, text_len,
		ansible_meta_handlers, sizeof(ansible_meta_handlers) / sizeof(preset_section_handler_t));
}

preset_read_result_t ansible_match_firmware_name(jsmntok_t token,
												 nvram_data_t* nvram, child_state_t* s, void* handler_def,
												 const char* text, size_t text_len)
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
										   const char* text, size_t text_len)
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
												 const char* text, size_t text_len)
{
	return handle_object(token,
		nvram, s,
		text, text_len,
		ansible_shared_handlers, sizeof(ansible_shared_handlers) / sizeof(preset_section_handler_t));
}

preset_read_result_t ansible_save_scales(jsmntok_t token,
										 nvram_data_t* nvram, child_state_t* s, void* handler_def,
										 const char* text, size_t text_len)
{
	preset_bufarray_state_t* state = (preset_bufarray_state_t*)s->state;
	if (s->fresh) {
		s->fresh = false;
		state->array_state = ARRAY_MATCH_START;
		state->array_ct = 0;
		state->buf_pos = 0;
	}

	switch (state->array_state) {
	case ARRAY_MATCH_START:
		if (token.type != JSMN_ARRAY) {
			return PRESET_READ_MALFORMED;
		}
		state->array_state = ARRAY_MATCH_ITEMS;
		return PRESET_READ_INCOMPLETE;
	case ARRAY_MATCH_ITEMS:
		if (token.type != JSMN_STRING) {
			return PRESET_READ_MALFORMED;
		}
		size_t start = token.start > 0 ? token.start : 0;
		size_t len = (token.end > 0 ? token.end : text_len) - start;
		if (len % 2 != 0) {
			// length needs to be even so we always decode full bytes
			return PRESET_READ_INCOMPLETE;
		}
		if (decode_hexbuf((uint8_t*)&nvram->scale[state->array_ct] + state->buf_pos,
						  text + start, len) < 0) {
			return PRESET_READ_MALFORMED;
		}
		if (token.end > 0) {
			state->buf_pos = 0;
			if (++state->array_ct < 16) {
				return PRESET_READ_INCOMPLETE;
			}
			s->fresh = true;
			return PRESET_READ_OK;
		}
		state->buf_pos += len;
		return PRESET_READ_INCOMPLETE;
	default:
		return PRESET_READ_MALFORMED;
	}
}

////////
// apps
