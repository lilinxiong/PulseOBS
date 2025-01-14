#include "heart_rate_source_info.h"

struct obs_source_info heart_rate_source_info = {
	.id = "heart_rate_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = get_heart_rate_source_name,
	.create = heart_rate_source_create,
	.destroy = heart_rate_source_destroy,
	.video_tick = heart_rate_source_tick,
	.video_render = heart_rate_source_render,
	.get_width = heart_rate_source_get_width,
	.get_height = heart_rate_source_get_height,
	.get_properties = heart_rate_source_properties};
