#include "ansible_sections.h"

load_object_state_t ansible_root_object_state;
load_object_state_t ansible_section_object_state;
load_object_state_t ansible_app_object_state;
load_buffer_state_t ansible_load_buffer_state;
load_array_state_t ansible_load_array_state;

/////////
// meta + shared

preset_section_handler_t ansible_meta_handlers[] = {
	{
		.name = "firmware",
		.read = match_string,
		.params = &((match_string_params_t) {
			.to_match = ANSIBLE_FIRMWARE_NAME,
		}),
	},
	{
		.name = "version",
		.read = match_string,
		.params = &((match_string_params_t) {
			.to_match = ANSIBLE_VERSION,
		}),
	},
	{
		.name = "i2c_addr",
		.read = load_scalar,
		.params = &((load_scalar_params_t){
			.dst_offset = offsetof(nvram_data_t, state.i2c_addr),
			.dst_size = sizeof(uint8_t),
		}),
	},
};

preset_section_handler_t ansible_shared_handlers[] = {
	{
		.name = "scales",
		.read = load_array,
		.fresh = true,
		.state = &ansible_load_array_state,
		.params = &((load_array_params_t) {
			.array_len = 16,
			.item_size = sizeof(uint8_t[8]),
			.item_handler = &((preset_section_handler_t) {
				.read = load_buffer,
				.fresh = true,
				.state = &ansible_load_buffer_state,
				.params = &((load_buffer_params_t) {
					.buf_len = 8,
					.dst_offset = offsetof(nvram_data_t, scale),
				}),
			})
		})
	},
};

/////////
// apps

preset_section_handler_t ansible_app_handlers[ANSIBLE_APP_COUNT] = {
	{
		.name = "tt",
		.read = load_object,
		.fresh = true,
		.state = &ansible_app_object_state,
		.params = &((load_object_params_t) {
			.handler_ct = 3,
			.handlers = ((preset_section_handler_t[]) {
				{
					.name = "clock_period",
					.read = load_scalar,
					.params = &((load_scalar_params_t) {
						.dst_offset = offsetof(nvram_data_t, tt_state.clock_period),
						.dst_size = sizeof(uint32_t),
					}),
				},
				{
					.name = "tr_time",
					.read = load_buffer,
					.fresh = true,
					.state = &ansible_load_buffer_state,
					.params = &((load_buffer_params_t) {
						.buf_len = 4 * sizeof(uint16_t),
						.dst_offset = offsetof(nvram_data_t, tt_state.tr_time),
					}),
				},
				{
					.name = "cv_slew",
					.read = load_buffer,
					.fresh = true,
					.state = &ansible_load_buffer_state,
					.params = &((load_buffer_params_t) {
						.buf_len = 4 * sizeof(uint16_t),
						.dst_offset = offsetof(nvram_data_t, tt_state.cv_slew),
					}),
				},
			}),
		}),
	},
};

/////////
// document

preset_section_handler_t ansible_handler = {
	.read = load_object,
	.fresh = true,
	.state = &ansible_root_object_state,
	.params = &((load_object_params_t) {
		.handler_ct = 3,
		.handlers = ((preset_section_handler_t[]) {
			{
				.name = "meta",
				.read = load_object,
				.fresh = true,
				.state = &ansible_section_object_state,
				.params = &((load_object_params_t) {
					.handlers = ansible_meta_handlers,
					.handler_ct = sizeof(ansible_meta_handlers) / sizeof(preset_section_handler_t),
				}),
			},
			{
				.name = "shared",
				.read = load_object,
				.fresh = true,
				.state = &ansible_section_object_state,
				.params = &((load_object_params_t) {
					.handlers = ansible_shared_handlers,
					.handler_ct = sizeof(ansible_shared_handlers) / sizeof(preset_section_handler_t),
				}),
			},
			{
				.name = "apps",
				.read = load_object,
				.fresh = true,
				.state = &ansible_section_object_state,
				.params = &((load_object_params_t) {
					.handlers = ansible_app_handlers,
					.handler_ct = sizeof(ansible_app_handlers) / sizeof(preset_section_handler_t),
				}),
			},
		}),
	}),
};