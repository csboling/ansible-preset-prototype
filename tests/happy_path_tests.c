﻿#include "happy_path_tests.h"

#include "greatest/greatest.h"

#include "deserialize_jsmn.h"
#include "ansible_sections.h"

char buf[128];
jsmntok_t tokens[8];
nvram_data_t nvram;
load_object_state_t object_state;

preset_section_handler_t* find_app_handler(char* name) {
	for (int i = 0; i < ANSIBLE_APP_COUNT; i++) {
		if (strcmp(ansible_app_handlers[i].name, name) == 0) {
			return &ansible_app_handlers[i];
		}
	}
	return NULL;
}

FILE* write_temp_file(const char* text, size_t len) {
	FILE* fp = fopen("data.tmp", "w");
	fwrite(text, 1, len, fp);
	fclose(fp);
	return fopen("data.tmp", "r");
}

TEST meta_read_ok() {
	const char meta[] = "{"
		"\"firmware\": \"" ANSIBLE_FIRMWARE_NAME "\","
		"\"version\": \"" ANSIBLE_VERSION "\","
		"\"i2c_addr\": 160"
	"}";
	FILE* fp = write_temp_file(meta, sizeof(meta));

	preset_read_result_t result = preset_deserialize(fp,
		&nvram, &((preset_section_handler_t) {
			.read = load_object,
			.fresh = true,
			.state = &object_state,
			.params = &((load_object_params_t) {
				.handlers = ansible_meta_handlers,
				.handler_ct = 3,
			}),
		}),
		buf, sizeof(buf),
		tokens, sizeof(tokens) / sizeof(jsmntok_t));
	fclose(fp);

	ASSERT_EQ(result, PRESET_READ_OK);
	ASSERT_EQ(nvram.state.i2c_addr, 160);
	PASS();
}

TEST shared_read_ok() {
	const char shared[] = "{"
		"\"scales\": ["
			"\"0011223344556677\","
			"\"1899aaBBccDDeeFF\","
			"\"2000000000000000\","
			"\"3000000000000000\","
			"\"4000000000000000\","
			"\"5000000000000000\","
			"\"6000000000000000\","
			"\"7000000000000000\","
			"\"8000000000000000\","
			"\"9000000000000000\","
			"\"A000000000000000\","
			"\"b000000000000000\","
			"\"C000000000000000\","
			"\"d000000000000000\","
			"\"E000000000000000\","
			"\"f000000000000000\","
		"]"
	"}";
	FILE* fp = write_temp_file(shared, sizeof(shared));

	preset_read_result_t result = preset_deserialize(fp,
		&nvram, &((preset_section_handler_t) {
			.read = load_object,
			.fresh = true,
			.state = &object_state,
			.params = &((load_object_params_t) {
				.handlers = ansible_shared_handlers,
				.handler_ct = 1, 
			}),
		}),
		buf, sizeof(buf),
		tokens, sizeof(tokens) / sizeof(jsmntok_t));
	fclose(fp);

	ASSERT_EQ(result, PRESET_READ_OK);

	ASSERT_STRN_EQ(
		"\x00\x11\x22\x33\x44\x55\x66\x77",
		nvram.scale[0],
		8);
	ASSERT_STRN_EQ(
		"\x18\x99\xAA\xBB\xCC\xDD\xEE\xFF",
		nvram.scale[1],
		8);
	for (int i = 2; i < 16; i++) {
		ASSERT_EQ(i << 4, nvram.scale[i][0]);
		ASSERT_STRN_EQ(
			"\x00\x00\x00\x00\x00\x00\x00",
			&nvram.scale[i][1],
			7);
	}
	PASS();
}

TEST tt_read_ok() {
	const char tt[] = "{"
		"\"clock_period\": 12345678,"
		"\"tr_time\": \"0011223344556677\","
		"\"cv_slew\": \"8899aabbccddeeff\""
	"}";
	FILE* fp = write_temp_file(tt, sizeof(tt));
	preset_section_handler_t* handler = find_app_handler("tt");

	preset_read_result_t result = preset_deserialize(fp,
		&nvram, handler,
		buf, sizeof(buf),
		tokens, sizeof(tokens) / sizeof(jsmntok_t));
	fclose(fp);

	ASSERT_EQ(result, PRESET_READ_OK);
	ASSERT_EQ(12345678, nvram.tt_state.clock_period);
	PASS();
}

SUITE(happy_path_suite) {
	RUN_TEST(meta_read_ok);
	RUN_TEST(shared_read_ok);
	RUN_TEST(tt_read_ok);
}
