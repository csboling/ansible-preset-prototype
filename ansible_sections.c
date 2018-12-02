#include "ansible_sections.h"

#define sizeof_field(s, m) (sizeof((((s*)0)->m)))

load_object_state_t ansible_root_object_state;
load_object_state_t ansible_section_object_state;
load_buffer_state_t ansible_load_buffer_state;
load_object_state_t ansible_app_object_state[4];
load_array_state_t ansible_load_array_state[4];

const char* connected_t_options[] = {
	"conNONE",
	"conARC",
	"conGRID",
	"conMIDI",
	"conFLASH",
};
const char* ansible_mode_options[] = {
	"mArcLevels",
	"mArcCycles",
	"mGridKria",
	"mGridMP",
	"mMidiStandard",
	"mMidiArp",
	"mTT",
};

/////////
// meta + shared

preset_section_handler_t ansible_meta_handlers[] = {
	{
		.name = "firmware",
		.read = match_string,
		.write = save_string,
		.params = &((match_string_params_t) {
			.to_match = ANSIBLE_FIRMWARE_NAME,
		}),
	},
	{
		.name = "version",
		.read = match_string,
		.write = save_string,
		.params = &((match_string_params_t) {
			.to_match = ANSIBLE_VERSION,
		}),
	},
	{
		.name = "i2c_addr",
		.read = load_scalar,
		.write = save_number,
		.params = &((load_scalar_params_t){
			.dst_offset = offsetof(nvram_data_t, state.i2c_addr),
			.dst_size = sizeof_field(nvram_data_t, state.i2c_addr),
		}),
	},
	{
		.name = "connected",
		.read = load_enum,
		.write = save_enum,
		.params = &((load_enum_params_t) {
			.option_ct = sizeof(connected_t_options) / sizeof(connected_t_options[0]),
			.options = connected_t_options,
			.dst_offset = offsetof(nvram_data_t, state.connected),
		}),
	},
	{
		.name = "arc_mode",
		.read = load_enum,
		.write = save_enum,
		.params = &((load_enum_params_t) {
			.option_ct = sizeof(ansible_mode_options) / sizeof(ansible_mode_options[0]),
			.options = ansible_mode_options,
			.dst_offset = offsetof(nvram_data_t, state.arc_mode),
		}),
	},
	{
		.name = "grid_mode",
		.read = load_enum,
		.write = save_enum,
		.params = &((load_enum_params_t) {
			.option_ct = sizeof(ansible_mode_options) / sizeof(ansible_mode_options[0]),
			.options = ansible_mode_options,
			.dst_offset = offsetof(nvram_data_t, state.grid_mode),
		}),
	},
	{
		.name = "midi_mode",
		.read = load_enum,
		.write = save_enum,
		.params = &((load_enum_params_t) {
			.option_ct = sizeof(ansible_mode_options) / sizeof(ansible_mode_options[0]),
			.options = ansible_mode_options,
			.dst_offset = offsetof(nvram_data_t, state.midi_mode),
		}),
	},
	{
		.name = "none_mode",
		.read = load_enum,
		.write = save_enum,
		.params = &((load_enum_params_t) {
			.option_ct = sizeof(ansible_mode_options) / sizeof(ansible_mode_options[0]),
			.options = ansible_mode_options,
			.dst_offset = offsetof(nvram_data_t, state.none_mode),
		}),
	},
};

preset_section_handler_t ansible_shared_handlers[] = {
	{
		.name = "scales",
		.read = load_array,
		.write = save_array,
		.fresh = true,
		.state = &ansible_load_array_state,
		.params = &((load_array_params_t) {
			.array_len = sizeof_field(nvram_data_t, scale) / sizeof_field(nvram_data_t, scale[0]),
			.item_size = sizeof_field(nvram_data_t, scale[0]),
			.item_handler = &((preset_section_handler_t) {
				.read = load_buffer,
				.write = save_buffer,
				.fresh = true,
				.state = &ansible_load_buffer_state,
				.params = &((load_buffer_params_t) {
					.dst_size = sizeof_field(nvram_data_t, scale[0]),
					.dst_offset = offsetof(nvram_data_t, scale),
				}),
			})
		})
	},
};

/////////
// apps

preset_section_handler_t kria_handlers[] = {
	{
		.name = "clock_period",
			.read = load_scalar,
			.write = save_number,
			.params = &((load_scalar_params_t) {
			.dst_size = sizeof_field(nvram_data_t, kria_state.clock_period),
				.dst_offset = offsetof(nvram_data_t, kria_state.clock_period),
		}),
	},
	{
		.name = "curr_preset",
		.read = load_scalar,
		.write = save_number,
		.params = &((load_scalar_params_t) {
			.dst_size = sizeof_field(nvram_data_t, kria_state.preset),
			.dst_offset = offsetof(nvram_data_t, kria_state.preset),
		}),
	},
	{
		.name = "note_sync",
		.read = load_scalar,
		.write = save_bool,
		.params = &((load_scalar_params_t) {
			.dst_size = sizeof_field(nvram_data_t, kria_state.note_sync),
			.dst_offset = offsetof(nvram_data_t, kria_state.note_sync),
		}),
	},
	{
		.name = "loop_sync",
		.read = load_scalar,
		.write = save_number,
		.params = &((load_scalar_params_t) {
			.dst_size = sizeof_field(nvram_data_t, kria_state.loop_sync),
			.dst_offset = offsetof(nvram_data_t, kria_state.loop_sync),
		}),
	},
	{
		.name = "cue_div",
		.read = load_scalar,
		.write = save_number,
		.params = &((load_scalar_params_t) {
			.dst_size = sizeof_field(nvram_data_t, kria_state.cue_div),
			.dst_offset = offsetof(nvram_data_t, kria_state.cue_div),
		}),
	},
	{
		.name = "cue_steps",
		.read = load_scalar,
		.write = save_number,
		.params = &((load_scalar_params_t) {
			.dst_size = sizeof_field(nvram_data_t, kria_state.cue_steps),
			.dst_offset = offsetof(nvram_data_t, kria_state.cue_steps),
		}),
	},
	{
		.name = "meta",
		.read = load_scalar,
		.write = save_number,
		.params = &((load_scalar_params_t) {
			.dst_size = sizeof_field(nvram_data_t, kria_state.meta),
			.dst_offset = offsetof(nvram_data_t, kria_state.meta),
		}),
	},
	{
		.name = "presets",
		.read = load_object,
		.write = save_object,
		.state = &ansible_app_object_state[1],
		.params = &((load_object_params_t) {
			.handler_ct = 9,
			.handlers = ((preset_section_handler_t[]) {
				{
					.name = "patterns",
					.read = load_array,
					.write = save_array,
					.fresh = true,
					.state = &ansible_load_array_state[1],
					.params = &((load_array_params_t) {
						.array_len = KRIA_NUM_PATTERNS,
						.item_size = sizeof(kria_pattern),
						.item_handler = &((preset_section_handler_t) {
							.read = load_object,
							.write = save_object,
							.fresh = true,
							.state = &ansible_app_object_state[2],
							.params = &((load_object_params_t) {
								.handler_ct = 2,
								.handlers = ((preset_section_handler_t[]) {
									{
										.name = "tracks",
										.read = load_array,
										.write = save_array,
										.fresh = true,
										.state = &ansible_load_array_state[2],
										.params = &((load_array_params_t) {
											.array_len = 4,
											.item_size = sizeof(kria_track),
											.item_handler = &((preset_section_handler_t) {
												.read = load_object,
												.write = save_object,
												.fresh = true,
												.state = &ansible_app_object_state[3],
												.params = &((load_object_params_t) {
													.handler_ct = 14,
													.handlers = ((preset_section_handler_t[]) {
														{
															.name = "tr",
															.read = load_buffer,
															.write = save_buffer,
															.fresh = true,
															.state = &ansible_load_buffer_state,
															.params = &((load_buffer_params_t) {
																.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].tr),
																.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].tr),
															}),
														},
														{
															.name = "oct",
															.read = load_buffer,
															.write = save_buffer,
															.fresh = true,
															.state = &ansible_load_buffer_state,
															.params = &((load_buffer_params_t) {
																.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].oct),
																.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].oct),
															}),
														},
														{
															.name = "note",
															.read = load_buffer,
															.write = save_buffer,
															.fresh = true,
															.state = &ansible_load_buffer_state,
															.params = &((load_buffer_params_t) {
																.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].note),
																.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].note),
															}),
														},
														{
															.name = "dur",
															.read = load_buffer,
															.write = save_buffer,
															.fresh = true,
															.state = &ansible_load_buffer_state,
															.params = &((load_buffer_params_t) {
																.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].dur),
																.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].dur),
															}),
														},
														{
															.name = "rpt",
															.read = load_buffer,
															.write = save_buffer,
															.fresh = true,
															.state = &ansible_load_buffer_state,
															.params = &((load_buffer_params_t) {
																.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].rpt),
																.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].rpt),
															}),
														},
														{
															.name = "alt_note",
															.read = load_buffer,
															.write = save_buffer,
															.fresh = true,
															.state = &ansible_load_buffer_state,
															.params = &((load_buffer_params_t) {
																.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].alt_note),
																.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].alt_note),
															}),
														},
														{
															.name = "glide",
															.read = load_buffer,
															.write = save_buffer,
															.fresh = true,
															.state = &ansible_load_buffer_state,
															.params = &((load_buffer_params_t) {
																.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].glide),
																.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].glide),
															}),
														},
														{
															.name = "p",
															.read = load_array,
															.write = save_array,
															.fresh = true,
															.state = &ansible_load_array_state[3],
															.params = &((load_array_params_t) {
																.array_len = KRIA_NUM_PARAMS,
																.item_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].p[0]),
																.item_handler = &((preset_section_handler_t) {
																	.read = load_buffer,
																	.write = save_buffer,
																	.fresh = true,
																	.state = &ansible_load_buffer_state,
																	.params = &((load_buffer_params_t) {
																		.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].p[0]),
																		.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].p[0]),
																	}),
																}),
															}),
														},
														{
															.name = "dur_mul",
															.read = load_scalar,
															.write = save_number,
															.params = &((load_scalar_params_t) {
																.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].dur_mul),
																.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].dur_mul),
															}),
														},
														{
															.name = "lstart",
															.read = load_buffer,
															.write = save_buffer,
															.fresh = true,
															.state = &ansible_load_buffer_state,
															.params = &((load_buffer_params_t) {
																.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].lstart),
																.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].lstart),
															}),
														},
														{
															.name = "lend",
															.read = load_buffer,
															.write = save_buffer,
															.fresh = true,
															.state = &ansible_load_buffer_state,
															.params = &((load_buffer_params_t) {
																.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].lend),
																.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].lend),
															}),
														},
														{
															.name = "lswap",
															.read = load_buffer,
															.write = save_buffer,
															.fresh = true,
															.state = &ansible_load_buffer_state,
															.params = &((load_buffer_params_t) {
																.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].lswap),
																.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].lswap),
															}),
														},
														{
															.name = "tmul",
															.read = load_buffer,
															.write = save_buffer,
															.fresh = true,
															.state = &ansible_load_buffer_state,
															.params = &((load_buffer_params_t) {
																.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].t[0].tmul),
																.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].t[0].tmul),
															}),
														},
													}),
												}),
											}),
										}),
									},
									{
										.name = "scale",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0].scale),
											.dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0].scale),
										}),
									},
								}),
							}),
						}),
					}),
				},
				{
					.name = "curr_pattern",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].pattern),
						.dst_offset = offsetof(nvram_data_t, kria_state.k[0].pattern),
					}),
				},
				{
					.name = "meta_pat",
					.read = load_buffer,
					.write = save_buffer,
					.fresh = true,
					.state = &ansible_load_buffer_state,
					.params = &((load_buffer_params_t) {
						.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].meta_pat),
						.dst_offset = offsetof(nvram_data_t, kria_state.k[0].meta_pat),
					}),
				},
				{
					.name = "meta_steps",
					.read = load_buffer,
					.write = save_buffer,
					.fresh = true,
					.state = &ansible_load_buffer_state,
					.params = &((load_buffer_params_t) {
						.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].meta_steps),
						.dst_offset = offsetof(nvram_data_t, kria_state.k[0].meta_steps),
					}),
				},
				{
					.name = "meta_start",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].meta_start),
						.dst_offset = offsetof(nvram_data_t, kria_state.k[0].meta_start),
					}),
				},
				{
					.name = "meta_end",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].meta_end),
						.dst_offset = offsetof(nvram_data_t, kria_state.k[0].meta_end),
					}),
				},
				{
					.name = "meta_len",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].meta_len),
						.dst_offset = offsetof(nvram_data_t, kria_state.k[0].meta_len),
					}),
				},
				{
					.name = "meta_lswap",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].meta_lswap),
						.dst_offset = offsetof(nvram_data_t, kria_state.k[0].meta_lswap),
					}),
				},
				{
					.name = "glyph",
					.read = load_buffer,
					.write = save_buffer,
					.fresh = true,
					.state = &ansible_load_buffer_state,
					.params = &((load_buffer_params_t) {
						.dst_size = sizeof_field(nvram_data_t, kria_state.k[0].glyph),
						.dst_offset = offsetof(nvram_data_t, kria_state.k[0].glyph),
					}),
				},
			}),
		}),
	},
};

preset_section_handler_t ansible_app_handlers[ANSIBLE_APP_COUNT] = {
	{
		.name = "kria",
		.read = load_object,
		.write = save_object,
		.fresh = true,
		.state = &ansible_app_object_state[0],
		.params = &((load_object_params_t) {
			.handler_ct = 8,
			.handlers = kria_handlers,
		}),
	},

	{
		.name = "cycles",
		.read = load_object,
		.write = save_object,
		.fresh = true,
		.state = &ansible_app_object_state[0],
		.params = &((load_object_params_t) {
			.handler_ct = 2,
			.handlers = ((preset_section_handler_t[]) {
				{
					.name = "curr_preset",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.signed_val = false,
						.dst_offset = offsetof(nvram_data_t, cycles_state.preset),
						.dst_size = sizeof_field(nvram_data_t, cycles_state.preset),
					}),
				},
				{
					.name = "presets",
					.read = load_array,
					.write = save_array,
					.fresh = true,
					.state = &ansible_load_array_state,
					.params = &((load_array_params_t) {
						.array_len = sizeof_field(nvram_data_t, cycles_state.c) / sizeof_field(nvram_data_t, cycles_state.c[0]),
						.item_size = sizeof_field(nvram_data_t, cycles_state.c[0]),
						.item_handler = &((preset_section_handler_t) {
							.read = load_object,
							.write = save_object,
							.fresh = true,
							.state = &ansible_app_object_state[1],
							.params = &((load_object_params_t) {
								.handler_ct = 9,
								.handlers = (preset_section_handler_t[]) {
									{
										.name = "pos",
										.read = load_buffer,
										.write = save_buffer,
										.fresh = true,
										.state = &ansible_load_buffer_state,
										.params = &((load_buffer_params_t) {
											.dst_size = sizeof_field(nvram_data_t, cycles_state.c[0].pos),
											.dst_offset = offsetof(nvram_data_t, cycles_state.c[0].pos),
										}),
									},
									{
										.name = "speed",
										.read = load_buffer,
										.write = save_buffer,
										.fresh = true,
										.state = &ansible_load_buffer_state,
										.params = &((load_buffer_params_t) {
											.dst_size = sizeof_field(nvram_data_t, cycles_state.c[0].speed),
											.dst_offset = offsetof(nvram_data_t, cycles_state.c[0].speed),
										}),
									},
									{
										.name = "mult",
										.read = load_buffer,
										.write = save_buffer,
										.fresh = true,
										.state = &ansible_load_buffer_state,
										.params = &((load_buffer_params_t) {
											.dst_size = sizeof_field(nvram_data_t, cycles_state.c[0].mult),
											.dst_offset = offsetof(nvram_data_t, cycles_state.c[0].mult),
										}),
									},
									{
										.name = "range",
										.read = load_buffer,
										.write = save_buffer,
										.fresh = true,
										.state = &ansible_load_buffer_state,
										.params = &((load_buffer_params_t) {
											.dst_size = sizeof_field(nvram_data_t, cycles_state.c[0].range),
											.dst_offset = offsetof(nvram_data_t, cycles_state.c[0].range),
										}),
									},
									{
										.name = "div",
										.read = load_buffer,
										.write = save_buffer,
										.fresh = true,
										.state = &ansible_load_buffer_state,
										.params = &((load_buffer_params_t) {
											.dst_size = sizeof_field(nvram_data_t, cycles_state.c[0].div),
											.dst_offset = offsetof(nvram_data_t, cycles_state.c[0].div),
										}),
									},
									{
										.name = "mode",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.signed_val = false,
											.dst_offset = offsetof(nvram_data_t, cycles_state.c[0].mode),
											.dst_size = sizeof_field(nvram_data_t, cycles_state.c[0].mode),
										}),
									},
									{
										.name = "shape",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.signed_val = false,
											.dst_offset = offsetof(nvram_data_t, cycles_state.c[0].shape),
											.dst_size = sizeof_field(nvram_data_t, cycles_state.c[0].shape),
										}),
									},
									{
										.name = "friction",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.signed_val = false,
											.dst_offset = offsetof(nvram_data_t, cycles_state.c[0].friction),
											.dst_size = sizeof_field(nvram_data_t, cycles_state.c[0].friction),
										}),
									},
									{
										.name = "force",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.signed_val = false,
											.dst_offset = offsetof(nvram_data_t, cycles_state.c[0].force),
											.dst_size = sizeof_field(nvram_data_t, cycles_state.c[0].force),
										}),
									},
								},
							}),
						}),
					}),
				},
			}),
		}),
	},

	{
		.name = "midi_standard",
		.read = load_object,
		.write = save_object,
		.fresh = true,
		.state = &ansible_app_object_state[0],
		.params = &((load_object_params_t) {
			.handler_ct = 5,
			.handlers = ((preset_section_handler_t[]) {
				{
					.name = "clock_period",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.signed_val = false,
						.dst_offset = offsetof(nvram_data_t, midi_standard_state.clock_period),
						.dst_size = sizeof_field(nvram_data_t, midi_standard_state.clock_period),
					}),
				},
				{
					.name = "voicing",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.signed_val = false,
						.dst_offset = offsetof(nvram_data_t, midi_standard_state.voicing),
						.dst_size = sizeof_field(nvram_data_t, midi_standard_state.voicing),
					}),
				},
				{
					.name = "fixed",
					.read = load_object,
					.write = save_object,
					.fresh = true,
					.state = &ansible_app_object_state[1],
					.params = &((load_object_params_t) {
						.handler_ct = 2,
						.handlers = ((preset_section_handler_t[]) {
							{
								.name = "notes",
								.read = load_buffer,
								.write = save_buffer,
								.fresh = true,
								.state = &ansible_load_buffer_state,
								.params = &((load_buffer_params_t) {
									.dst_size = sizeof_field(nvram_data_t, midi_standard_state.fixed.notes),
									.dst_offset = offsetof(nvram_data_t, midi_standard_state.fixed.notes),
								}),
							},
							{
								.name = "cc",
								.read = load_buffer,
								.write = save_buffer,
								.fresh = true,
								.state = &ansible_load_buffer_state,
								.params = &((load_buffer_params_t) {
									.dst_size = sizeof_field(nvram_data_t, midi_standard_state.fixed.cc),
									.dst_offset = offsetof(nvram_data_t, midi_standard_state.fixed.cc),
								}),
							},
						}),
					}),
				},
				{
					.name = "shift",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.signed_val = true,
						.dst_offset = offsetof(nvram_data_t, midi_standard_state.shift),
						.dst_size = sizeof_field(nvram_data_t, midi_standard_state.shift),
					}),
				},
				{
					.name = "slew",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.signed_val = true,
						.dst_offset = offsetof(nvram_data_t, midi_standard_state.slew),
						.dst_size = sizeof_field(nvram_data_t, midi_standard_state.slew),
					}),
				},
			}),
		}),
	},

	{
		.name = "midi_arp",
		.read = load_object,
		.write = save_object,
		.fresh = true,
		.state = &ansible_app_object_state[0],
		.params = &((load_object_params_t) {
			.handler_ct = 4,
			.handlers = ((preset_section_handler_t[]) {
				{
					.name = "clock_period",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.signed_val = false,
						.dst_offset = offsetof(nvram_data_t, midi_arp_state.clock_period),
						.dst_size = sizeof_field(nvram_data_t, midi_arp_state.clock_period),
					}),
				},
				{
					.name = "style",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.signed_val = false,
						.dst_offset = offsetof(nvram_data_t, midi_arp_state.style),
						.dst_size = sizeof_field(nvram_data_t, midi_arp_state.style),
					}),
				},
				{
					.name = "hold",
					.read = load_scalar,
					.write = save_bool,
					.params = &((load_scalar_params_t) {
						.dst_offset = offsetof(nvram_data_t, midi_arp_state.hold),
						.dst_size = sizeof_field(nvram_data_t, midi_arp_state.hold),
					}),
				},
				{
					.name = "players",
					.read = load_array,
					.write = save_array,
					.fresh = true,
					.state = &ansible_load_array_state,
					.params = &((load_array_params_t) {
						.array_len = sizeof_field(nvram_data_t, midi_arp_state.p) / sizeof_field(nvram_data_t, midi_arp_state.p[0]),
						.item_size = sizeof_field(nvram_data_t, midi_arp_state.p[0]),
						.item_handler = &((preset_section_handler_t) {
							.read = load_object,
							.write = save_object,
							.fresh = true,
							.state = &ansible_app_object_state[1],
							.params = &((load_object_params_t) {
								.handler_ct = 8,
								.handlers = (preset_section_handler_t[]) {
									{
										.name = "fill",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.signed_val = false,
											.dst_offset = offsetof(nvram_data_t, midi_arp_state.p[0].fill),
											.dst_size = sizeof_field(nvram_data_t, midi_arp_state.p[0].fill),
										}),
									},
									{
										.name = "division",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.signed_val = false,
											.dst_offset = offsetof(nvram_data_t, midi_arp_state.p[0].division),
											.dst_size = sizeof_field(nvram_data_t, midi_arp_state.p[0].division),
										}),
									},
									{
										.name = "rotation",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.signed_val = false,
											.dst_offset = offsetof(nvram_data_t, midi_arp_state.p[0].rotation),
											.dst_size = sizeof_field(nvram_data_t, midi_arp_state.p[0].rotation),
										}),
									},
									{
										.name = "gate",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.signed_val = false,
											.dst_offset = offsetof(nvram_data_t, midi_arp_state.p[0].gate),
											.dst_size = sizeof_field(nvram_data_t, midi_arp_state.p[0].gate),
										}),
									},
									{
										.name = "steps",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.signed_val = false,
											.dst_offset = offsetof(nvram_data_t, midi_arp_state.p[0].steps),
											.dst_size = sizeof_field(nvram_data_t, midi_arp_state.p[0].steps),
										}),
									},
									{
										.name = "offset",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.signed_val = false,
											.dst_offset = offsetof(nvram_data_t, midi_arp_state.p[0].offset),
											.dst_size = sizeof_field(nvram_data_t, midi_arp_state.p[0].offset),
										}),
									},
									{
										.name = "slew",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.signed_val = true,
											.dst_offset = offsetof(nvram_data_t, midi_arp_state.p[0].slew),
											.dst_size = sizeof_field(nvram_data_t, midi_arp_state.p[0].slew),
										}),
									},
									{
										.name = "shift",
										.read = load_scalar,
										.write = save_number,
										.params = &((load_scalar_params_t) {
											.signed_val = true,
											.dst_offset = offsetof(nvram_data_t, midi_arp_state.p[0].shift),
											.dst_size = sizeof_field(nvram_data_t, midi_arp_state.p[0].shift),
										}),
									},
								},
							}),
						}),
					}),
				},
			}),
		}),
	},

	{
		.name = "tt",
		.read = load_object,
		.write = save_object,
		.fresh = true,
		.state = &ansible_app_object_state[0],
		.params = &((load_object_params_t) {
			.handler_ct = 3,
			.handlers = ((preset_section_handler_t[]) {
				{
					.name = "clock_period",
					.read = load_scalar,
					.write = save_number,
					.params = &((load_scalar_params_t) {
						.signed_val = false,
						.dst_offset = offsetof(nvram_data_t, tt_state.clock_period),
						.dst_size = sizeof_field(nvram_data_t, tt_state.clock_period),
					}),
				},
				{
					.name = "tr_time",
					.read = load_buffer,
					.write = save_buffer,
					.fresh = true,
					.state = &ansible_load_buffer_state,
					.params = &((load_buffer_params_t) {
						.dst_size = sizeof_field(nvram_data_t, tt_state.tr_time),
						.dst_offset = offsetof(nvram_data_t, tt_state.tr_time),
					}),
				},
				{
					.name = "cv_slew",
					.read = load_buffer,
					.write = save_buffer,
					.fresh = true,
					.state = &ansible_load_buffer_state,
					.params = &((load_buffer_params_t) {
						.dst_size = sizeof_field(nvram_data_t, tt_state.cv_slew),
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
	.write = save_object,
	.fresh = true,
	.state = &ansible_root_object_state,
	.params = &((load_object_params_t) {
		.handler_ct = 3,
		.handlers = ((preset_section_handler_t[]) {
			{
				.name = "meta",
				.read = load_object,
				.write = save_object,
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
				.write = save_object,
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
				.write = save_object,
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
