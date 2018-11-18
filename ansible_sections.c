#include "ansible_sections.h"
#include "ansible_preset_internal.h"

preset_object_state_t ansible_read_object_state;
preset_bufarray_state_t ansible_bufarray_state;
preset_section_handler_t ansible_sections[ANSIBLE_SECTION_CT] = {
	{
		.name = "meta",
		.read = ansible_read_meta_section,
	},
	{
		.name = "shared",
		.read = ansible_read_shared_section,
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
		.read = ansible_save_i2c_addr,
	},
};
preset_section_handler_t ansible_shared_handlers[] = {
	{
		.name = "scales",
		.read = ansible_save_scales,
	},
};

void ansible_init_sections() {
	ansible_sections[0].child_state.state = &ansible_read_object_state;
	ansible_sections[1].child_state.state = &ansible_read_object_state;

	ansible_shared_handlers[0].child_state.state = &ansible_bufarray_state;
}

/////////
// meta

preset_read_result_t ansible_read_meta_section(jsmntok_t token,
	nvram_data_t* nvram, child_state_t* s,
	const char* text, size_t text_len)
{
	return handle_object(token,
		nvram, s,
		text, text_len,
		ansible_meta_handlers, sizeof(ansible_meta_handlers) / sizeof(preset_section_handler_t));
}

preset_read_result_t ansible_match_firmware_name(jsmntok_t token,
												 nvram_data_t* nvram, child_state_t* s,
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
										   nvram_data_t* nvram, child_state_t* s,
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

preset_read_result_t ansible_save_i2c_addr(jsmntok_t token,
										   nvram_data_t* nvram, child_state_t* s,
										   const char* text, size_t text_len)
{
	if (token.type != JSMN_PRIMITIVE) {
		return PRESET_READ_MALFORMED;
	}
	if (token.end < 0) {
		return PRESET_READ_INCOMPLETE;
	}
	nvram->state.i2c_addr = decode_decimal(text + token.start, token.end - token.start);
	return PRESET_READ_OK;
}

////////
// shared

preset_read_result_t ansible_read_shared_section(jsmntok_t token,
												 nvram_data_t* nvram, child_state_t* s,
												 const char* text, size_t text_len)
{
	return handle_object(token,
		nvram, s,
		text, text_len,
		ansible_shared_handlers, sizeof(ansible_shared_handlers) / sizeof(preset_section_handler_t));
}

preset_read_result_t ansible_save_scales(jsmntok_t token,
										 nvram_data_t* nvram, child_state_t* s,
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
		if (token.start > 0 && token.start % 2 != 0) {
			// need to start on an even offset to read hex encoding
			return PRESET_READ_INCOMPLETE;
		}
		size_t start = token.start > 0 ? token.start : 0;
		size_t len = (token.end > 0 ? token.end : text_len) - start;
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
