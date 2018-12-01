#pragma once

#include "deserialize_jsmn.h"

#define ANSIBLE_FIRMWARE_NAME "ansible"
#define ANSIBLE_VERSION "1.6.1-dev"
#define ANSIBLE_APP_COUNT 5

preset_section_handler_t ansible_handler;
preset_section_handler_t ansible_shared_handlers[];
preset_section_handler_t ansible_meta_handlers[];
preset_section_handler_t ansible_app_handlers[ANSIBLE_APP_COUNT];