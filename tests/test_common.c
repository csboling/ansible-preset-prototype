#include "test_common.h"

#include <stdlib.h>
// for htons and friends
#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

#include "greatest/greatest.h"

#include "deserialize_jsmn.h"
#include "ansible_sections.h"

preset_section_handler_t* find_handler(preset_section_handler_t* base, char * name) {
	memset(&nvram, 0, sizeof(nvram_data_t));
	load_object_params_t* params = base->params;
	for (int i = 0; i < params->handler_ct; i++) {
		if (strcmp(params->handlers[i].name, name) == 0) {
			return &params->handlers[i];
		}
	}
	return NULL;
}

preset_section_handler_t* find_app_handler(char* name) {
	preset_section_handler_t* apps = find_handler(&ansible_handler, "apps");
	return find_handler(apps, name);
}

FILE* write_temp_file(const char* name, const char* text, size_t len) {
	FILE* fp = fopen(name, "w");
	fwrite(text, 1, len, fp);
	fclose(fp);
	return fopen(name, "r");
}

char cmp_buf[2][4096];
bool compare_files(const char* left, const char* right) {
	FILE* l = fopen(left, "r");
	FILE* r = fopen(right, "r");
	size_t l_ct, r_ct;
	while (true) {
		l_ct = fread(cmp_buf[0], 1, sizeof(cmp_buf), l);
		r_ct = fread(cmp_buf[1], 1, sizeof(cmp_buf), r);
		if (l_ct == 0 && r_ct == 0) {
			return true;
		}
		if (l_ct < 0 || r_ct < 0 || l_ct != r_ct) {
			return false;
		}

		if (strncmp(cmp_buf[0], cmp_buf[1], l_ct) != 0) {
			return false;
		}
	}
}

bool compare_short_big_endian(uint16_t* big_endian, uint16_t* native_order, size_t len) {
  for (size_t i = 0; i < len; i++) {
	uint16_t big = htons(native_order[i]);
    if (big_endian[i] != big) {
		return false;
	}
  }
  return true;
}
