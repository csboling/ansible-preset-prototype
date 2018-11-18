﻿#include "deserialize_jsmn.h"

int decode_decimal(const char* s, int len) {
	int ret = 0;
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
	if (hex < '9') {
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
	*dst = hex - 'A';
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

preset_read_result_t handle_object(jsmntok_t tok,
								   nvram_data_t* nvram, child_state_t* s,
								   const char* text, size_t text_len,
								   preset_section_handler_t* handlers, uint8_t handler_ct)
{
	preset_object_state_t* state = (preset_object_state_t*)s->state;
	if (s->fresh) {
		s->fresh = false;
		state->match_state = MATCH_START_OBJECT;
		state->active_handler = NULL;
		state->sections_handled = 0;
	}

	switch (state->match_state) {
	case MATCH_START_OBJECT:
		if (tok.type != JSMN_OBJECT) {
			return PRESET_READ_MALFORMED;
		}
		state->match_state = MATCH_SECTION_NAME;
		return PRESET_READ_INCOMPLETE;
	case MATCH_SECTION_NAME:
		if (tok.type != JSMN_STRING) {
			return PRESET_READ_MALFORMED;
		}
		if (tok.end > 0) {
			for (uint8_t i = state->sections_handled; i < handler_ct; i++) {
				if (strncmp(handlers[i].name, text + tok.start, tok.end - tok.start) == 0) {
					state->active_handler = &handlers[i];
					state->active_handler->child_state.fresh = true;
					state->match_state = MATCH_PARSE_SECTION;
					return PRESET_READ_INCOMPLETE;
				}
			}
		}
		return PRESET_READ_INCOMPLETE;
	case MATCH_PARSE_SECTION:
		switch(state->active_handler->read(tok,
										   nvram, &state->active_handler->child_state,
										   text, text_len)) {
		case PRESET_READ_INCOMPLETE:
			return PRESET_READ_INCOMPLETE;
		case PRESET_READ_OK:
			if (++state->sections_handled < handler_ct) {
				state->match_state = MATCH_SECTION_NAME;
				return PRESET_READ_INCOMPLETE;
			}
			s->fresh = true;
			return PRESET_READ_OK;
		}
	default:
		return PRESET_READ_MALFORMED;
	}
}

preset_read_result_t preset_deserialize(FILE* fp, nvram_data_t* nvram,
										preset_object_state_t* object_state,
										preset_section_handler_t* handlers, uint8_t handler_ct,
										char* textbuf, size_t textbuf_len,
										jsmntok_t* tokbuf, size_t tokbuf_len)
{
	preset_read_result_t result;
	preset_read_state_t state = {
		.text_ct = 0,
		.tok_ct = 0,
		.curr_tok = tokbuf_len,
		.child_state = {
			.fresh = true,
			.state = (void*)object_state,
		},
	};

	jsmn_init(&state.jsmn);
	for (uint8_t i = 0; i < tokbuf_len; i++) {
		tokbuf[i].start = tokbuf[i].end = 0;
	}

	while (state.text_ct >= 0) {
		if (state.curr_tok >= state.tok_ct) {
			int keep_ct = 0;
			if (state.tok_ct > 0) {
				// if the last token was incomplete, copy the text already read and
				// try to parse it again. otherwise copy the text that hasn't been parsed yet.
				jsmntok_t* last_tok = &tokbuf[state.curr_tok - 1];
				if (last_tok->end < 0) {
					keep_ct = textbuf_len - last_tok->start;
				}
				else {
					keep_ct = textbuf_len - last_tok->end;
				}
				strncpy(textbuf,
						textbuf + textbuf_len - keep_ct,
						keep_ct);

				// modify the parser state so it stays within the fixed buffers
				state.jsmn.pos -= textbuf_len - keep_ct;
				state.jsmn.toknext -= state.tok_ct;
				state.jsmn.toksuper -= state.tok_ct;
			}
			state.text_ct = fread(textbuf + keep_ct, 1, textbuf_len - keep_ct, fp);
			if (state.text_ct <= 0) {
				break;
			}
			state.tok_ct = jsmn_parse(&state.jsmn,
									  textbuf, textbuf_len,
									  tokbuf, tokbuf_len);
			switch (state.tok_ct) {
			case JSMN_ERROR_NOMEM:
				state.tok_ct = tokbuf_len;
			case JSMN_ERROR_PART:
				// need to find the last token that was set by the parser
				// the rest are left over in the buffer from last time
				for (state.tok_ct = 1; (size_t)state.tok_ct < tokbuf_len; state.tok_ct++) {
					if (tokbuf[state.tok_ct - 1].end >= tokbuf[state.tok_ct].start) {
						break;
					}
				}
				break;
			default:
				return PRESET_READ_MALFORMED;
			}
			if (state.tok_ct < 0) {
			}
			state.curr_tok = 0;
		}

		result = handle_object(tokbuf[state.curr_tok],
							   nvram, &state.child_state,
							   textbuf, textbuf_len,
							   handlers, handler_ct);
		state.curr_tok++;
		switch (result) {
		case PRESET_READ_INCOMPLETE:
			continue;
		case PRESET_READ_OK:
			return PRESET_READ_OK;
		default:
			return PRESET_READ_MALFORMED;
		}
    }
    if (state.text_ct < 0) {
		return PRESET_READ_MALFORMED;
	}
	return PRESET_READ_OK;
}
