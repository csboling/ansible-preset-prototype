#include "end_to_end_tests.h"
#include "deserialize_jsmn.h"
#include "ansible_sections.h"

#include "test_common.h"
#include "greatest/greatest.h"

nvram_data_t rd_nvram;

TEST write_read_compare(void) {
	memset(&nvram, 1, sizeof(nvram_data_t));

	FILE* fp = fopen("out.tmp", "w");
	preset_write_result_t wr_result = preset_serialize(fp, &nvram, &ansible_handler);
	fclose(fp);
	
	ASSERT_EQ(PRESET_WRITE_OK, wr_result);

	fp = fopen("out.tmp", "r");
	preset_read_result_t rd_result = preset_deserialize(
		fp, &rd_nvram, &ansible_handler,
		buf, sizeof(buf),
		tokens, sizeof(tokens) / sizeof(jsmntok_t));
	fclose(fp);

	ASSERT_EQ(PRESET_READ_OK, rd_result);

	// we can't just compare the memory regions directly because
	// of padding bytes added to the structs for alignment.
	// fudge it a little, and only test the apps that are
	// tedious to test manually
	ASSERT_MEM_EQ(
		&nvram.kria_state,
		&rd_nvram.kria_state,
		sizeof(kria_state_t) - sizeof(int));
	ASSERT_MEM_EQ(
		&nvram.mp_state,
		&rd_nvram.mp_state,
		sizeof(mp_state_t) - sizeof(int));
	ASSERT_MEM_EQ(
		(char*)&nvram.levels_state + 2,
		(char*)&rd_nvram.levels_state + 2,
		sizeof(levels_state_t) - sizeof(int));

	PASS();
}

SUITE(end_to_end_suite) {
	RUN_TEST(write_read_compare);
}