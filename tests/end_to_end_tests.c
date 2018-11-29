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
		fp, &nvram, &ansible_handler,
		buf, sizeof(buf),
		tokens, sizeof(tokens) / sizeof(jsmntok_t));
	fclose(fp);

	ASSERT_EQ(PRESET_READ_OK, rd_result);

	ASSERT_MEM_EQ(&nvram, &rd_nvram, sizeof(nvram_data_t));

	PASS();
}

SUITE(end_to_end_suite) {
	// RUN_TEST(write_read_compare);
}