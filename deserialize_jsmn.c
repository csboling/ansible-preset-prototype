#include <stdlib.h>

#include "deserialize_jsmn.h"


char* encode_decimal_unsigned(uint32_t val) {
	static char decimal_encoding_buf[12] = { 0 };
	uint8_t i = 10;
	if (val == 0) {
		decimal_encoding_buf[i--] = '0';
	}
	else {
		for (; val && i; --i, val /= 10) {
			decimal_encoding_buf[i] = (val % 10) + '0';
		}
	}
	return &decimal_encoding_buf[i + 1];
}

char* encode_decimal_signed(int32_t val) {
	char* ret;
	if (val < 0) {
		ret = encode_decimal_unsigned(-val);
		*--ret = '-';
	} else {
		ret = encode_decimal_unsigned(val);
	}
	return ret;
}

int32_t decode_decimal(const char* s, int len) {
	int32_t ret = 0;
	// also handle bool
	if (s[0] == 't') {
		return 1;
	}
	if (s[0] == 'f') {
		return 0;
	}
	bool negative = s[0] == '-';
	for (int i = negative ? 1 : 0; i < len; i++) {
		ret = ret * 10 + (s[i] - '0');
	}
	return negative ? -ret : ret;
}

int decode_nybble(uint8_t* dst, char hex) {
	if (hex < '0') {
		return -1;
	}
	if (hex <= '9') {
		*dst = hex - '0';
		return 0;
	}
	if (hex < 'A' || hex > 'f') {
		return -1;
	}
	if (hex >= 'a') {
		hex -= ('a' - 'A');
	}
	if (hex > 'F') {
		return -1;
	}
	*dst = hex - 'A' + 0xA;
	return 0;
}

int decode_hexbuf(char* dst, const char* src, size_t len) {
	uint8_t upper, lower;
	for (size_t i = 0; i < len; i += 2) {
		if (decode_nybble(&upper, src[i]) < 0) {
			return -1;
		}
		if (decode_nybble(&lower, src[i + 1]) < 0) {
			return -1;
		}
		dst[i / 2] = (upper << 4) | lower;
	}
	return 0;
}

char encode_nybble(uint8_t value) {
	if (value > 0x9) {
		return value - 0xA + 'A';
	}
	return value + '0';
}

preset_read_result_t load_object(jsmntok_t tok,
								   nvram_data_t* nvram, void* handler_def,
								   const char* text, size_t text_len, size_t dst_offset, unsigned int depth)
{
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	load_object_params_t* params = (load_object_params_t*)handler->params;
	load_object_state_t* state = (load_object_state_t*)handler->state;
	if (handler->fresh) {
		handler->fresh = false;
		state->object_state = MATCH_START_OBJECT;
		state->active_handler = NULL;
		state->sections_handled = 0;
		state->depth = 0;
	}

	switch (state->object_state) {
	case MATCH_START_OBJECT:
		if (tok.type != JSMN_OBJECT) {
			return PRESET_READ_MALFORMED;
		}
		state->object_state = MATCH_SECTION_NAME;
		return PRESET_READ_INCOMPLETE;
	case SKIP_SECTION:
		if (tok.depth != state->depth || tok.type != JSMN_STRING) {
			return PRESET_READ_INCOMPLETE;
		}
		state->object_state = MATCH_SECTION_NAME;
		// fallthrough
	case MATCH_SECTION_NAME:
		if (tok.type != JSMN_STRING) {
			return PRESET_READ_MALFORMED;
		}
		if (tok.end > 0) {
			for (uint8_t i = 0; i < params->handler_ct; i++) {
				if (strncmp(params->handlers[i].name, text + tok.start, tok.end - tok.start) == 0) {
					state->active_handler = &params->handlers[i];
					state->active_handler->fresh = true;
					state->object_state = MATCH_PARSE_SECTION;
					return PRESET_READ_INCOMPLETE;
				}
			}
			state->object_state = SKIP_SECTION;
			state->depth = tok.depth;
		}
		return PRESET_READ_INCOMPLETE;
	case MATCH_PARSE_SECTION:
		switch(state->active_handler->read(tok,
										   nvram, state->active_handler,
										   text, text_len, dst_offset, depth)) {
		case PRESET_READ_INCOMPLETE:
			return PRESET_READ_INCOMPLETE;
		case PRESET_READ_OK:
			if (++state->sections_handled < params->handler_ct) {
				state->object_state = MATCH_SECTION_NAME;
				return PRESET_READ_INCOMPLETE;
			}
			handler->fresh = true;
			return PRESET_READ_OK;
		default:
			return PRESET_READ_MALFORMED;
		}
	default:
		return PRESET_READ_MALFORMED;
	}
}

preset_write_result_t save_object(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset) {
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	load_object_params_t* params = (load_object_params_t*)handler->params;

	write("{", 1);
	for (uint8_t i = 0; i < params->handler_ct; i++) {
		write("\"", 1);
		write(params->handlers[i].name, strlen(params->handlers[i].name));
		write("\": ", 3);
		params->handlers[i].write(write, nvram, &params->handlers[i], src_offset);
		if (i + 1 < params->handler_ct) {
			write(", ", 2);
		}
	}
	write("}", 1);
	return PRESET_WRITE_OK;
}

preset_read_result_t load_scalar(jsmntok_t tok,
								 nvram_data_t* nvram, void* handler_def,
								 const char* text, size_t text_len, size_t dst_offset, unsigned int depth) {
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	load_scalar_params_t* params = (load_scalar_params_t*)handler->params;
	if (tok.type != JSMN_PRIMITIVE) {
		return PRESET_READ_MALFORMED;
	}
	if (tok.end < 0) {
		return PRESET_READ_INCOMPLETE;
	}
	void* dst = (char*)nvram + params->dst_offset + dst_offset;
	int val = decode_decimal(text + tok.start, tok.end - tok.start);
	switch (params->dst_size) {
	case sizeof(uint8_t):
		*(uint8_t*)dst = val;
		break;
	case sizeof(uint16_t):
		*(uint16_t*)dst = val;
		break;
	case sizeof(uint32_t):
		*(uint32_t*)dst = val;
		break;
	default:
		return PRESET_READ_MALFORMED;
	}
	return PRESET_READ_OK;
}

preset_write_result_t save_number(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset) {
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	load_scalar_params_t* params = (load_scalar_params_t*)handler->params;
	void* src = (uint8_t*)nvram + src_offset + params->dst_offset;
	char* dec;

	if (params->signed_val) {
		switch (params->dst_size) {
		case 4:
			dec = encode_decimal_signed(*(int32_t*)src);
			break;
		case 2:
			dec = encode_decimal_signed(*(int16_t*)src);
			break;
		case 1:
			dec = encode_decimal_signed(*(int8_t*)src);
			break;
		default:
			return PRESET_WRITE_FAILURE;
		}
	} else {
		switch (params->dst_size) {
		case 4:
			dec = encode_decimal_unsigned(*(uint32_t*)src);
			break;
		case 2:
			dec = encode_decimal_unsigned(*(uint16_t*)src);
			break;
		case 1:
			dec = encode_decimal_unsigned(*(uint8_t*)src);
			break;
		default:
			return PRESET_WRITE_FAILURE;
		}
	}
	write(dec, strlen(dec));
	return PRESET_WRITE_OK;
}

preset_write_result_t save_bool(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset) {
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	load_scalar_params_t* params = (load_scalar_params_t*)handler->params;

	bool val = *((uint8_t*)nvram + src_offset + params->dst_offset);

	if (val) {
		write("true", 4);
	}
	else {
		write("false", 5);
	}
	return PRESET_WRITE_OK;
}

preset_write_result_t save_string(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset) {
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	match_string_params_t* params = (match_string_params_t*)handler->params;

	write("\"", 1);
	write(params->to_match, strlen(params->to_match));
	write("\"", 1);
	return PRESET_WRITE_OK;
}

preset_read_result_t match_string(jsmntok_t tok,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len, size_t dst_offset, unsigned int depth) {
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	match_string_params_t* params = (match_string_params_t*)handler->params;
	if (tok.type != JSMN_STRING) {
		return PRESET_READ_MALFORMED;
	}
	if (tok.end < 0) {
		return PRESET_READ_INCOMPLETE;
	}
	if (strncmp(params->to_match, text + tok.start, tok.end - tok.start) != 0) {
		return PRESET_READ_MALFORMED;
	}
	return PRESET_READ_OK;
}
preset_read_result_t load_enum(jsmntok_t tok,
	nvram_data_t* nvram, void* handler_def,
	const char* text, size_t text_len, size_t dst_offset, unsigned int depth) {
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	load_enum_params_t* params = (load_enum_params_t*)handler->params;

	int* dst = (int*)((uint8_t*)nvram + params->dst_offset + dst_offset);

	if (tok.end < 0) {
		return PRESET_READ_INCOMPLETE;
	}
	if (tok.type == JSMN_PRIMITIVE) {
		*dst = decode_decimal(text + tok.start, tok.end - tok.start);
		return PRESET_READ_OK;
	}
	if (tok.type != JSMN_STRING) {
		return PRESET_READ_MALFORMED;
	}

	for (uint8_t i = 0; i < params->option_ct; i++) {
		if (strncmp(params->options[i], text + tok.start, tok.end - tok.start) == 0) {
			*dst = i;
			break;
		}
	}
	return PRESET_READ_OK;
}

preset_write_result_t save_enum(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset) {
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	load_enum_params_t* params = (load_enum_params_t*)handler->params;

	int* src = (int*)((uint8_t*)nvram + src_offset + params->dst_offset);
	if (*src < 0 || *src >= params->option_ct) {
		write("null", 4);
		return PRESET_WRITE_OK;
	}
	write("\"", 1);
	write(params->options[*src], strlen(params->options[*src]));
	write("\"", 1);
	return PRESET_WRITE_OK;
}

preset_read_result_t load_array(jsmntok_t tok,
								nvram_data_t* nvram, void* handler_def,
								const char* text, size_t text_len, size_t dst_offset, unsigned int depth) {
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	load_array_params_t* params = (load_array_params_t*)handler->params;
	load_array_state_t* state = (load_array_state_t*)handler->state;
	if (handler->fresh) {
		handler->fresh = false;
		state->array_state = ARRAY_MATCH_START;
		state->array_ct = 0;
	}

	switch (state->array_state) {
	case ARRAY_MATCH_START:
		if (tok.type != JSMN_ARRAY) {
			return PRESET_READ_MALFORMED;
		}
		state->array_state = ARRAY_MATCH_ITEMS;
		return PRESET_READ_INCOMPLETE;
	case ARRAY_MATCH_ITEMS:
		switch (params->item_handler->read(tok,
			nvram, params->item_handler,
			text, text_len, dst_offset + state->array_ct * params->item_size, depth))
		{
		case PRESET_READ_MALFORMED:
			return PRESET_READ_MALFORMED;
		case PRESET_READ_INCOMPLETE:
			return PRESET_READ_INCOMPLETE;
		case PRESET_READ_OK:
			if (++state->array_ct < params->array_len) {
				return PRESET_READ_INCOMPLETE;
			}
			handler->fresh = true;
			return PRESET_READ_OK;
		default:
			return PRESET_READ_MALFORMED;
		}
	default:
		return PRESET_READ_MALFORMED;
	}
}

preset_write_result_t save_array(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset) {
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	load_array_params_t* params = (load_array_params_t*)handler->params;

	write("[", 1);
	for (size_t i = 0; i < params->array_len; i++) {
		params->item_handler->write(write, nvram, params->item_handler, src_offset + params->item_size * i);
		if (i + 1 < params->array_len) {
			write(", ", 2);
		}
	}
	write("]", 1);
	return PRESET_WRITE_OK;
}

preset_read_result_t load_buffer(jsmntok_t tok,
								 nvram_data_t* nvram, void* handler_def,
								 const char* text, size_t text_len, size_t dst_offset, unsigned int depth) {
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	load_buffer_params_t* params = (load_buffer_params_t*)handler->params;
	load_buffer_state_t* state = (load_buffer_state_t*)handler->state;
	if (handler->fresh) {
		handler->fresh = false;
		state->buf_pos = 0;
	}
	if (tok.type != JSMN_STRING) {
		return PRESET_READ_MALFORMED;
	}
	size_t start = tok.start > 0 ? tok.start : 0;
	size_t len = (tok.end > 0 ? tok.end : text_len) - start;
	if (len % 2 != 0) {
		// length needs to be even so we always decode full bytes
		return PRESET_READ_INCOMPLETE;
	}
	if (state->buf_pos + len != params->dst_size * 2) {
		return PRESET_READ_MALFORMED;
	}
	uint8_t* dst = (uint8_t*)nvram + params->dst_offset + dst_offset;
	if (decode_hexbuf(dst + state->buf_pos,
		text + start, len) < 0) {
		return PRESET_READ_MALFORMED;
	}
	if (tok.end > 0) {
		state->buf_pos = 0;
		handler->fresh = true;
		return PRESET_READ_OK;
	}
	state->buf_pos += len;
	return PRESET_READ_INCOMPLETE;
}

preset_write_result_t save_buffer(
	write_buffer_fn write,
	nvram_data_t* nvram, void* handler_def,
	size_t src_offset) {
	char nybble;
	preset_section_handler_t* handler = (preset_section_handler_t*)handler_def;
	load_buffer_params_t* params = (load_buffer_params_t*)handler->params;

	write("\"", 1);
	uint8_t* src = (uint8_t*)nvram + src_offset + params->dst_offset;
	for (size_t i = 0; i < params->dst_size; i++) {
		nybble = encode_nybble((src[i] & 0xF0) >> 4);
		write(&nybble, 1);
		nybble = encode_nybble(src[i] & 0x0F);
		write(&nybble, 1);
	}
	write("\"", 1);
	return PRESET_WRITE_OK;
}

preset_read_state_t deserialize_state;

preset_read_result_t preset_deserialize(FILE* fp, nvram_data_t* nvram,
										preset_section_handler_t* handler,
										char* textbuf, size_t textbuf_len,
										jsmntok_t* tokbuf, size_t tokbuf_len)
{
	preset_read_result_t result;
	int keep_ct = 0;
	jsmntok_t* last_tok;
	size_t bytes_read = 0;

	deserialize_state.text_ct = textbuf_len;
	deserialize_state.curr_tok = tokbuf_len;
	jsmn_init(&deserialize_state.jsmn);

	for (uint8_t i = 0; i < tokbuf_len; i++) {
		tokbuf[i].start = tokbuf[i].end = 0;
	}

	while (deserialize_state.text_ct >= 0) {
		if (deserialize_state.jsmn.pos > textbuf_len) {
			return PRESET_READ_MALFORMED;
		}
		if (deserialize_state.curr_tok >= deserialize_state.jsmn.toknext) {
			keep_ct = 0;
			if (deserialize_state.jsmn.toknext > 0) {
				// if the last token was incomplete, copy the text already read and
				// try to parse it again. otherwise copy the text that hasn't been parsed yet.
				last_tok = &tokbuf[deserialize_state.jsmn.toknext - 1];
				if (last_tok->end < 0) {
					keep_ct = deserialize_state.text_ct - last_tok->start;
				} else {
					keep_ct = deserialize_state.text_ct - last_tok->end;
				}
				strncpy(textbuf,
						textbuf + deserialize_state.text_ct - keep_ct,
						keep_ct);

				// rewind the parser state so it stays within the fixed buffers
				deserialize_state.jsmn.pos -= deserialize_state.text_ct - keep_ct;
				deserialize_state.jsmn.toknext = 0;
				if (deserialize_state.jsmn.toksuper >= 0) {
					deserialize_state.jsmn.toksuper -= deserialize_state.jsmn.toknext;
				}
			}
			bytes_read = fread(textbuf + keep_ct, 1, textbuf_len - keep_ct, fp);
			if (bytes_read < 0) {
				return PRESET_READ_MALFORMED;
			}
			deserialize_state.text_ct = keep_ct + bytes_read;
			int err = jsmn_parse(&deserialize_state.jsmn,
									  textbuf, deserialize_state.text_ct,
									  tokbuf, tokbuf_len);
			if (err < 0 && err != JSMN_ERROR_NOMEM && err != JSMN_ERROR_PART) {
				return PRESET_READ_MALFORMED;
			}
			deserialize_state.curr_tok = 0;
		}

		result = handler->read(tokbuf[deserialize_state.curr_tok],
			nvram, handler,
			textbuf, textbuf_len, 0, deserialize_state.jsmn.depth);
		deserialize_state.curr_tok++;
		switch (result) {
		case PRESET_READ_INCOMPLETE:
			continue;
		case PRESET_READ_OK:
			return PRESET_READ_OK;
		default:
			return PRESET_READ_MALFORMED;
		}
    }
	return PRESET_READ_MALFORMED;
}

static FILE* active_fp;

void write_to_fp(const uint8_t* buf, size_t len) {
	fwrite(buf, 1, len, active_fp);
}

preset_write_result_t preset_serialize(FILE* fp, nvram_data_t* nvram, preset_section_handler_t* handler) {
	active_fp = fp;
	preset_write_result_t ret = handler->write(write_to_fp, nvram, handler, 0);
	return ret;
}