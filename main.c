#include <stdio.h>
#include <string.h>

#include "ansible_sections.h"
#include "deserialize_jsmn.h"

char buf[128];
jsmntok_t tokens[8];
nvram_data_t nvram;
preset_read_result_t result;
preset_object_state_t object_state;

int main()
{
	FILE* fp = fopen("ansible-presets.json", "r");
	result = preset_deserialize(fp,
								&nvram, &object_state,
								ansible_sections, ANSIBLE_SECTION_CT,
								buf, sizeof(buf),
								tokens, sizeof(tokens) / sizeof(jsmntok_t));
	fclose(fp);  
  
	return result;
}

