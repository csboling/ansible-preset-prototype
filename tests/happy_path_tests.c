﻿#include "happy_path_tests.h"

#include "greatest/greatest.h"

#include "deserialize_jsmn.h"
#include "ansible_sections.h"

char buf[128];
jsmntok_t tokens[8];
nvram_data_t nvram;
preset_object_state_t object_state;

TEST full_read_ok() {
	ansible_init_sections();

	FILE* fp = fopen("../ansible-presets.json", "r");
	preset_read_result_t result = preset_deserialize(fp,
							&nvram, &object_state,
							ansible_sections, ANSIBLE_SECTION_CT,
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

	ansible_init_sections();

	FILE* fp = fopen("data.tmp", "w");
	fwrite(shared, 1, sizeof(shared), fp);
	fclose(fp);

	fp = fopen("data.tmp", "r");
	preset_read_result_t result = preset_deserialize(fp,
		&nvram, &object_state,
		ansible_shared_handlers, 1,
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

SUITE(happy_path_suite) {
  RUN_TEST(full_read_ok);
  RUN_TEST(shared_read_ok);
}
