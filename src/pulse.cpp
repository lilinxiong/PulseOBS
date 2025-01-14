#include <obs-module.h>
#include <obs.h>
#include <obs-frontend-api.h>
#include <obs-source.h>
#include <obs-data.h>
#include <graphics/graphics.h>
#include <util/platform.h>
#include "heart_rate_widget.hpp"
#include <vector>
#include "plugin-support.h"
#include "heart_rate_source.h"

const char *get_heart_rate_source_name(void *)
{
	return "Heart Rate Monitor";
}

struct heart_rate_source {
	obs_source_t *source;
};

// Create function
void *heart_rate_source_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(settings);

	obs_log(LOG_INFO, "--------------Start of CREATE!!!!!!!!!");
	// const char* selected_device = obs_data_get_string(settings, "heart_rate_device");

	// obs_log(LOG_INFO, "--------------Start of DEVICE SELECTION!!!!!!!!!");
	// // Check if a device was selected
	// if (selected_device == nullptr || strlen(selected_device) == 0) {
	//     blog(LOG_ERROR, "No webcam device selected!");
	//     return nullptr;
	// }

	// obs_log(LOG_INFO, "--------------Start of VIDEO SOURCE!!!!!!!!!");
	// // Create video source using the selected device
	// obs_source_t* video_source = obs_source_create("video_capture", selected_device, nullptr, nullptr);

	// if (!video_source) {
	//     blog(LOG_ERROR, "Failed to create video capture source!");
	//     return nullptr;
	// }

	// Store the video source data in the plugin's private data structure
	obs_log(LOG_INFO, "--------------Begin BAZLLOC!!!!!!!!!");
	struct heart_rate_source *hrs = (struct heart_rate_source *)bzalloc(
		sizeof(struct heart_rate_source));
	hrs->source = source;
	obs_log(LOG_INFO, "--------------END OF CREATE!!!!!");
	return hrs;
}

// Destroy function
void heart_rate_source_destroy(void *data)
{
	obs_log(LOG_INFO, "--------------Start of DESTROY!!!!!!!!!");
	bfree(data);
}

// Tick function
void heart_rate_source_tick(void *data, float seconds)
{
	UNUSED_PARAMETER(data);
	obs_log(LOG_INFO, "--------------Start of TICK!!!!!!!!!");
	// struct heart_rate_source * hrs= (struct heart_rate_source *)data;
	UNUSED_PARAMETER(seconds);
	// Logic to update per frame
}

int calculateHeartRate(uint8_t *data[], uint32_t width, uint32_t height)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	// Logic to calculate heart rate
	return 100;
}

// Render function
void heart_rate_source_render(void *data, gs_effect_t *effect)
{
	obs_log(LOG_INFO, "--------------Start of RENDER!!!!!!!!!");
	struct heart_rate_source *hrs = (struct heart_rate_source *)data;
	UNUSED_PARAMETER(effect);

	struct obs_source_frame *frame = obs_source_get_frame(hrs->source);
	if (frame) {
		// Extract RGB data and process it for heart rate
		// int heartRate = calculateHeartRate(frame->data, frame->width, frame->height);

		obs_source_release_frame(hrs->source, frame);

		// if (heartRateWidget) {
		//   heartRateWidget->updateHeartRate(heartRate);
		// }
	}
}

// Get width function
uint32_t heart_rate_source_get_width(void *data)
{
	UNUSED_PARAMETER(data);
	obs_log(LOG_INFO, "--------------Start of WIDTH!!!!!!!!!");
	// struct heart_rate_source *hrs = (struct heart_rate_source *)data;
	// obs_log(LOG_INFO, "END OF WIDTH!!!!! %s", obs_source_get_width(hrs->source));
	// return obs_source_get_width(hrs->source);
	return 100;
}

// Get height function
uint32_t heart_rate_source_get_height(void *data)
{
	UNUSED_PARAMETER(data);
	obs_log(LOG_INFO, "--------------Start of HEIGHT!!!!!!!!!");
	// struct heart_rate_source *hrs = (struct heart_rate_source *)data;
	// return obs_source_get_height(hrs->source);
	return 100;
}

bool store_device_references(void *param, obs_source_t *source)
{
	auto *device_references =
		static_cast<std::vector<obs_source_t *> *>(param);

	obs_source_t *s = obs_source_get_ref(source);
	device_references->push_back(s);

	return true;
}

obs_properties_t *heart_rate_source_properties(void *data)
{
	UNUSED_PARAMETER(data);
	obs_log(LOG_INFO, "PROPERTIES");
	obs_properties_t *props = obs_properties_create();

	// Add properties
	obs_properties_add_text(props, "heart_rate_device", "Webcam Device",
				OBS_TEXT_DEFAULT);

	// Populate available video devices
	obs_property_t *devices_property = obs_properties_add_list(
		props, "heart_rate_device_list", "Choose Webcam",
		OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);

	// Vector to store references
	std::vector<obs_source_t *> device_references;

	obs_enum_sources(store_device_references, &device_references);

	obs_log(LOG_INFO, "GRAB DEVICES");

	for (obs_source_t *source : device_references) {
		const char *source_name = obs_source_get_name(source);
		obs_log(LOG_INFO, "SOURCENAME: %s", source_name);
		obs_property_list_add_string(devices_property, source_name,
					     source_name);
	}

	// // Enumerate available video devices
	// uint32_t device_count = obs_enum_video_devices(nullptr, nullptr);
	// for (uint32_t i = 0; i < device_count; ++i) {
	//     const char* device_name = obs_video_device_get_name(i);
	//     obs_property_list_add_string(devices_property, device_name, device_name);
	// }

	return props;
}

// bool store_device_references(void *param, obs_source_t *source) {
//     auto *device_references = static_cast<std::vector<obs_source_t *> *>(param);

//     obs_source_t * source = obs_source_get_ref(source)
//     device_references->push_back(source);

//     return true;
// }
