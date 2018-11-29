#include "happy_path_tests.h"
#include "test_common.h"

#include "greatest/greatest.h"

#include "deserialize_jsmn.h"
#include "ansible_sections.h"

TEST skips_unknown_sections() {
	const char meta[] = "{"
		"\"firmware\": \"" ANSIBLE_FIRMWARE_NAME "\", "
		"\"unknownSection\": {"
			"\"lots\": [\"of\", \"stuff\"],"
			"\"to\": {"
			  "\"skip\": \"over\", "
			  "\"onetwo\": 345"
			"}"
		"},"
		"\"version\": \"" ANSIBLE_VERSION "\", "
		"\"i2c_addr\": 160"
	"}";
	preset_section_handler_t handler = {
		.read = load_object,
		.write = save_object,
		.fresh = true,
		.state = &object_state,
		.params = &((load_object_params_t) {
			.handlers = ansible_meta_handlers,
			.handler_ct = 3,
		}),
	};
	FILE* fp = write_temp_file("in.tmp", meta, sizeof(meta));
	memset(&nvram, 0, sizeof(nvram_data_t));

	preset_read_result_t result = preset_deserialize(fp,
		&nvram, &handler,
		buf, sizeof(buf),
		tokens, sizeof(tokens) / sizeof(jsmntok_t));
	fclose(fp);

	ASSERT_EQ(result, PRESET_READ_OK);
	ASSERT_EQ(nvram.state.i2c_addr, 160);
	PASS();
}

TEST meta_read_ok() {
	const char meta[] = "{"
		"\"firmware\": \"" ANSIBLE_FIRMWARE_NAME "\", "
		"\"version\": \"" ANSIBLE_VERSION "\", "
		"\"i2c_addr\": 160"
	"}";
	preset_section_handler_t handler = {
		.read = load_object,
		.write = save_object,
		.fresh = true,
		.state = &object_state,
		.params = &((load_object_params_t) {
			.handlers = ansible_meta_handlers,
			.handler_ct = 3,
		}),
	};
	FILE* fp = write_temp_file("in.tmp", meta, sizeof(meta));
	memset(&nvram, 0, sizeof(nvram_data_t));

	preset_read_result_t result = preset_deserialize(fp,
		&nvram, &handler,
		buf, sizeof(buf),
		tokens, sizeof(tokens) / sizeof(jsmntok_t));
	fclose(fp);

	ASSERT_EQ(result, PRESET_READ_OK);
	ASSERT_EQ(nvram.state.i2c_addr, 160);

	fp = fopen("out.tmp", "w");
	preset_write_result_t wr_result = preset_serialize(fp, &nvram, &handler);
	fclose(fp);

	ASSERT_FALSE(!compare_files("in.tmp", "out.tmp"));

	PASS();
}

preset_section_handler_t shared_handler = {
	.read = load_object,
	.write = save_object,
	.fresh = true,
	.state = &object_state,
	.params = &((load_object_params_t) {
		.handlers = ansible_shared_handlers,
		.handler_ct = 1,
	}),
};

TEST shared_read_ok() {
	const char shared[] = "{"
		"\"scales\": ["
			"\"0011223344556677\", "
			"\"1899AABBCCDDEEFF\", "
			"\"2000000000000000\", "
			"\"3000000000000000\", "
			"\"4000000000000000\", "
			"\"5000000000000000\", "
			"\"6000000000000000\", "
			"\"7000000000000000\", "
			"\"8000000000000000\", "
			"\"9000000000000000\", "
			"\"A000000000000000\", "
			"\"B000000000000000\", "
			"\"C000000000000000\", "
			"\"D000000000000000\", "
			"\"E000000000000000\", "
			"\"F000000000000000\""
		"]"
	"}";
	FILE* fp = write_temp_file("in.tmp", shared, sizeof(shared));
	memset(&nvram, 0, sizeof(nvram_data_t));

	preset_read_result_t result = preset_deserialize(fp,
		&nvram, &shared_handler,
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

	fp = fopen("out.tmp", "w");
	preset_write_result_t wr_result = preset_serialize(fp, &nvram, &shared_handler);
	fclose(fp);

	ASSERT_FALSE(!compare_files("in.tmp", "out.tmp"));

	PASS();
}

TEST cycles_read_ok() {
	const char cycles[] = "{"
		"\"curr_preset\": 5, "
		"\"presets\": ["
			"{"
				"\"pos\": \"0001000200030004\", "
				"\"speed\": \"80007FFF7FFE7FFD\", "
				"\"mult\": \"807F7E7D\", "
				"\"range\": \"00010203\", "
				"\"div\": \"04050607\", "
				"\"mode\": 5, "
				"\"shape\": 4, "
				"\"friction\": 3, "
				"\"force\": 1000"
			"}, "
			"{"
				"\"pos\": \"0001000200030004\", "
				"\"speed\": \"80007FFF7FFE7FFD\", "
				"\"mult\": \"807F7E7D\", "
				"\"range\": \"00010203\", "
				"\"div\": \"04050607\", "
				"\"mode\": 5, "
				"\"shape\": 4, "
				"\"friction\": 3, "
				"\"force\": 1000"
			"}, "
			"{"
				"\"pos\": \"0001000200030004\", "
				"\"speed\": \"80007FFF7FFE7FFD\", "
				"\"mult\": \"807F7E7D\", "
				"\"range\": \"00010203\", "
				"\"div\": \"04050607\", "
				"\"mode\": 5, "
				"\"shape\": 4, "
				"\"friction\": 3, "
				"\"force\": 1000"
			"}, "
			"{"
				"\"pos\": \"0001000200030004\", "
				"\"speed\": \"80007FFF7FFE7FFD\", "
				"\"mult\": \"807F7E7D\", "
				"\"range\": \"00010203\", "
				"\"div\": \"04050607\", "
				"\"mode\": 5, "
				"\"shape\": 4, "
				"\"friction\": 3, "
				"\"force\": 1000"
			"}, "
			"{"
				"\"pos\": \"0001000200030004\", "
				"\"speed\": \"80007FFF7FFE7FFD\", "
				"\"mult\": \"807F7E7D\", "
				"\"range\": \"00010203\", "
				"\"div\": \"04050607\", "
				"\"mode\": 5, "
				"\"shape\": 4, "
				"\"friction\": 3, "
				"\"force\": 1000"
			"}, "
			"{"
				"\"pos\": \"0001000200030004\", "
				"\"speed\": \"80007FFF7FFE7FFD\", "
				"\"mult\": \"807F7E7D\", "
				"\"range\": \"00010203\", "
				"\"div\": \"04050607\", "
				"\"mode\": 5, "
				"\"shape\": 4, "
				"\"friction\": 3, "
				"\"force\": 1000"
			"}, "
			"{"
				"\"pos\": \"0001000200030004\", "
				"\"speed\": \"80007FFF7FFE7FFD\", "
				"\"mult\": \"807F7E7D\", "
				"\"range\": \"00010203\", "
				"\"div\": \"04050607\", "
				"\"mode\": 5, "
				"\"shape\": 4, "
				"\"friction\": 3, "
				"\"force\": 1000"
			"}, "
			"{"
				"\"pos\": \"0001000200030004\", "
				"\"speed\": \"80007FFF7FFE7FFD\", "
				"\"mult\": \"807F7E7D\", "
				"\"range\": \"00010203\", "
				"\"div\": \"04050607\", "
				"\"mode\": 5, "
				"\"shape\": 4, "
				"\"friction\": 3, "
				"\"force\": 1000"
			"}"
		"]"
	"}";
	FILE* fp = write_temp_file("in.tmp", cycles, sizeof(cycles));
	preset_section_handler_t* handler = find_app_handler("cycles");
	memset(&nvram, 0, sizeof(nvram_data_t));

	preset_read_result_t result = preset_deserialize(fp,
		&nvram, handler,
		buf, sizeof(buf),
		tokens, sizeof(tokens) / sizeof(jsmntok_t));
	fclose(fp);

	ASSERT_EQ(result, PRESET_READ_OK);

	fp = fopen("out.tmp", "w");
	preset_write_result_t wr_result = preset_serialize(fp, &nvram, handler);
	fclose(fp);

	ASSERT_FALSE(!compare_files("in.tmp", "out.tmp"));

	PASS();
}

TEST midi_standard_read_ok() {
	const char midi_standard[] = "{"
		"\"clock_period\": 12345678, "
		"\"voicing\": 123, "
		"\"fixed\": {"
			"\"notes\": \"00112233\", "
			"\"cc\": \"44556677\""
		"}, "
		"\"shift\": -12345, "
		"\"slew\": 12345"
	"}";
	FILE* fp = write_temp_file("in.tmp", midi_standard, sizeof(midi_standard));
	preset_section_handler_t* handler = find_app_handler("midi_standard");

	preset_read_result_t result = preset_deserialize(fp,
		&nvram, handler,
		buf, sizeof(buf),
		tokens, sizeof(tokens) / sizeof(jsmntok_t));
	fclose(fp);

	ASSERT_EQ(result, PRESET_READ_OK);
	ASSERT_EQ(12345678, nvram.midi_standard_state.clock_period);
	ASSERT_EQ(123, nvram.midi_standard_state.voicing);
	ASSERT_EQ(0x33, nvram.midi_standard_state.fixed.notes[3]);
	ASSERT_EQ(0x55, nvram.midi_standard_state.fixed.cc[1]);
	ASSERT_EQ(-12345, nvram.midi_standard_state.shift);
	ASSERT_EQ(12345, nvram.midi_standard_state.slew);

	fp = fopen("out.tmp", "w");
	preset_write_result_t wr_result = preset_serialize(fp, &nvram, handler);
	fclose(fp);

	ASSERT_FALSE(!compare_files("in.tmp", "out.tmp"));

	PASS();
}

TEST midi_arp_read_ok() {
	const char midi_arp[] = "{"
		"\"clock_period\": 12345678, "
		"\"style\": 210, "
		"\"hold\": true, "
		"\"players\": ["
			"{"
				"\"fill\": 0, "
				"\"division\": 1, "
				"\"rotation\": 2, "
				"\"gate\": 3, "
				"\"steps\": 4, "
				"\"offset\": 5, "
				"\"slew\": -5, "
				"\"shift\": 10"
			"}, "
			"{"
				"\"fill\": 1, "
				"\"division\": 1, "
				"\"rotation\": 2, "
				"\"gate\": 3, "
				"\"steps\": 4, "
				"\"offset\": 5, "
				"\"slew\": -5, "
				"\"shift\": 10"
			"}, "
			"{"
				"\"fill\": 2, "
				"\"division\": 1, "
				"\"rotation\": 2, "
				"\"gate\": 3, "
				"\"steps\": 4, "
				"\"offset\": 5, "
				"\"slew\": -5, "
				"\"shift\": 10"
			"}, "
			"{"
				"\"fill\": 3, "
				"\"division\": 1, "
				"\"rotation\": 2, "
				"\"gate\": 3, "
				"\"steps\": 4, "
				"\"offset\": 5, "
				"\"slew\": -5, "
				"\"shift\": 10"
			"}"
		"]"
	"}";
	FILE* fp = write_temp_file("in.tmp", midi_arp, sizeof(midi_arp));
	preset_section_handler_t* handler = find_app_handler("midi_arp");

	preset_read_result_t result = preset_deserialize(fp,
		&nvram, handler,
		buf, sizeof(buf),
		tokens, sizeof(tokens) / sizeof(jsmntok_t));
	fclose(fp);

	ASSERT_EQ(result, PRESET_READ_OK);
	ASSERT_EQ(true, nvram.midi_arp_state.hold);
	for (int i = 0; i < 4; i++) {
		ASSERT_EQ(i, nvram.midi_arp_state.p[i].fill);
		ASSERT_EQ(-5, nvram.midi_arp_state.p[i].slew);
	}

	fp = fopen("out.tmp", "w");
	preset_write_result_t wr_result = preset_serialize(fp, &nvram, handler);
	fclose(fp);

	ASSERT_FALSE(!compare_files("in.tmp", "out.tmp"));
	PASS();
}

TEST tt_read_ok() {
	const char tt[] = "{"
		"\"clock_period\": 12345678, "
		"\"tr_time\": \"0011223344556677\", "
		"\"cv_slew\": \"8899AABBCCDDEEFF\""
	"}";
	FILE* fp = write_temp_file("in.tmp", tt, sizeof(tt));
	preset_section_handler_t* handler = find_app_handler("tt");

	preset_read_result_t result = preset_deserialize(fp,
		&nvram, handler,
		buf, sizeof(buf),
		tokens, sizeof(tokens) / sizeof(jsmntok_t));
	fclose(fp);

	ASSERT_EQ(result, PRESET_READ_OK);
	ASSERT_EQ(12345678, nvram.tt_state.clock_period);
	
	{
		uint16_t expected[] = {
			0x0011,
			0x2233,
			0x4455,
			0x6677,
		};
		ASSERT_FALSE(!compare_short_big_endian(
			expected,
			nvram.tt_state.tr_time,
			sizeof(expected) / sizeof(uint16_t)));
	}

	{
		uint16_t expected[] = {
			0x8899,
			0xaabb,
			0xccdd,
			0xeeff,
		};
		ASSERT_FALSE(!compare_short_big_endian(
			expected,
			nvram.tt_state.cv_slew,
			sizeof(expected) / sizeof(uint16_t)));
	}


	fp = fopen("out.tmp", "w");
	preset_write_result_t wr_result = preset_serialize(fp, &nvram, handler);
	fclose(fp);

	ASSERT_FALSE(!compare_files("in.tmp", "out.tmp"));
	PASS();
}

SUITE(happy_path_suite) {
	RUN_TEST(skips_unknown_sections);

	RUN_TEST(meta_read_ok);
	RUN_TEST(shared_read_ok);

	RUN_TEST(cycles_read_ok);
	RUN_TEST(midi_standard_read_ok);
	RUN_TEST(midi_arp_read_ok);
	RUN_TEST(tt_read_ok);
}
