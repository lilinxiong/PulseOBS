#include <obs-module.h>
#include <obs.h>
#include <obs-frontend-api.h>
#include <obs-source.h>
#include <obs-data.h>
#include <graphics/graphics.h>
#include <util/platform.h>
#include <vector>
#include "plugin-support.h"
#include "heart_rate_source.h"

const char *get_heart_rate_source_name(void *)
{
	return "Heart Rate Monitor";
}

struct heart_rate_source {
	obs_source_t *source;
	gs_texrender_t *
		texrender; // buffer in GPU where rendering operations are performed
	gs_stagesurf_t *
		stagesurface; // facilitates transferring rendered textures from the GPU to CPU
};

static void processBGRAData(uint8_t *data, uint32_t width, uint32_t height,
			    uint32_t linesize)
{
	uint64_t sumB = 0, sumG = 0, sumR = 0, sumA = 0;
	uint32_t pixel_count = width * height;

	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			uint8_t B = data[y * linesize + x * 4 + 0];
			uint8_t G = data[y * linesize + x * 4 + 1];
			uint8_t R = data[y * linesize + x * 4 + 2];
			uint8_t A = data[y * linesize + x * 4 + 3];
			sumB += B;
			sumG += G;
			sumR += R;
			sumA += A;
		}
	}

	double averageB = static_cast<double>(sumB) / pixel_count;
	double averageG = static_cast<double>(sumG) / pixel_count;
	double averageR = static_cast<double>(sumR) / pixel_count;
	double averageA = static_cast<double>(sumA) / pixel_count;

	obs_log(LOG_INFO, "Average B: %f, G: %f, R: %f, A: %f\n", averageB,
		averageG, averageR, averageA);
}

static bool getBGRAFromStageSurface(struct heart_rate_source *hrs,
				    uint32_t &width, uint32_t &height)
{
	obs_log(LOG_INFO, "--------BGRASTART!!!!!!!!");

	// Check if the source is enabled
	if (!obs_source_enabled(hrs->source)) {
		return false;
	}

	obs_log(LOG_INFO, "--------RETRIEVING TARGET SOURCE!!!!!!!!");

	// Retrieve the target source of the filter
	obs_source_t *target = obs_filter_get_target(hrs->source);
	if (!target) {
		return false;
	}

	obs_log(LOG_INFO,
		"--------RETRIEVING BASE DIMENSIONS OF TARGET SOURCE!!!!!!!!");

	// Retrieve the base dimensions of the target source
	width = obs_source_get_base_width(target);
	height = obs_source_get_base_height(target);
	if (width == 0 || height == 0) {
		return false;
	}

	obs_log(LOG_INFO, "--------RESETTING THE TEXTURE RENDERER!!!!!!!!");

	// Resets the texture renderer and begins rendering with the specified width and height
	gs_texrender_reset(hrs->texrender);
	obs_log(LOG_INFO, "width: %d", width);
	obs_log(LOG_INFO, "height: %d", height);
	if (!hrs->texrender) {
		obs_log(LOG_INFO, "texrender is null");
	}
	if (!gs_texrender_begin(hrs->texrender, width, height)) {
		obs_log(LOG_INFO, "--------FAILED TO START RENDERING!!!!!!!!");
		return false;
	}

	obs_log(LOG_INFO,
		"--------FINISHED RESETTING THE TEXTURE RENDERER!!!!!!!!");

	// Clear up and set up rendering
	// - Clears the rendering surface with a zeroed background
	// - Sets up the orthographic projection matrix
	// - Pushes the blend state and sets the blend function
	// - Renders the video of the target source
	// - Pops the blend state and ends the texture rendering

	// Declare a vec4 structure to hold the background colour
	struct vec4 background; // two component vector struct (x, y, z, w)

	// Set the background colour to zero (black)
	vec4_zero(&background); // zeroes a vector

	// Clear the rendering surface from the previous frame processing, with the specified background colour. The GS_CLEAR_COLOR flag indicates that the colour buffer should be cleared. This sets the colour to &background colour
	gs_clear(GS_CLEAR_COLOR, &background, 0.0f,
		 0); // Clears color/depth/stencil buffers

	// Sets up an orthographic projection matrix. This matrix defines a 2D rendering space where objects are rendered without perspective distortion
	// Parameters:
	// - 0.0f: The left coordinate of the projection
	// static_cast<float>(width): The rigCan youht coordinate of the projection, set to the width of the target source
	// 0.0f: The bottom coordinate of the projection.
	// static_cast<float>(height): The top coordinate of the projection, set to the height of the target source
	// -100.0f: The near clipping plane. The near clipping plane is the closest plane to the camera. Objects closer to the camera than this plane are clipped (not rendered). It helps to avoid rendering artifacts and improves depth precision by discarding objects that are too close to the camera
	// 100.0f: The far clipping plane. The far clipping plane is the farthest plane from the camera. Objects farther from the camera than this plane are clipped (not rendered).It helps to limit the rendering distance and manage depth buffer precision by discarding objects that are too far away
	gs_ortho(0.0f, static_cast<float>(width), 0.0f,
		 static_cast<float>(height), -100.0f, 100.0f);

	// This function saves the current blend state onto a stack. The blend state includes settings that control how colors from different sources are combined during rendering. By pushing the current blend state, you can make temporary changes to the blend settings and later restore the original settings by popping the blend state from the stack
	gs_blend_state_push();

	// This function sets the blend function for rendering. The blend function determines how the source (the new pixels being drawn) and the destination (the existing pixels in the framebuffer) colors are combined
	// Parameters:
	// - GS_BLEND_ONE: The source color is used as-is. This means the source color is multiplied by 1
	// - GS_BLEND_ZERO: The destination color is ignored. This means the destination color is multiplied by 0
	// Effect: The combination of GS_BLEND_ONE and GS_BLEND_ZERO results in the source color completely replacing the destination color. This is equivalent to disabling blending, where the new pixels overwrite the existing pixels
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);

	// This function renders the video frame of the target source onto the current rendering surface
	obs_source_video_render(target);

	// This function restores the previous blend state from the stack. The blend state that was saved by gs_blend_state_push is now restored. By popping the blend state, you undo the temporary changes made to the blend settings, ensuring that subsequent rendering operations use the original blend settings
	gs_blend_state_pop();

	// This function ends the texture rendering process. It finalizes the rendering operations and makes the rendered texture available for further processing. This function completes the rendering process, ensuring that the rendered texture is properly finalised and can be used for subsequent operations, such as extracting pixel data or further processing
	gs_texrender_end(hrs->texrender);

	// Retrieve the old existing stage surface
	if (hrs->stagesurface) {
		uint32_t stagesurf_width =
			gs_stagesurface_get_width(hrs->stagesurface);
		uint32_t stagesurf_height =
			gs_stagesurface_get_height(hrs->stagesurface);
		// If it still matches the new width and height, reuse it
		if (stagesurf_width != width || stagesurf_height != height) {
			// Destroy the old stage surface
			gs_stagesurface_destroy(hrs->stagesurface);
			hrs->stagesurface = nullptr;
		}
	}

	// Create a new stage surface if necessary
	if (!hrs->stagesurface) {
		hrs->stagesurface =
			gs_stagesurface_create(width, height, GS_BGRA);
	}

	// Use gs_stage_texture to stage the texture from the texture renderer (hrs->texrender) to the stage surface (hrs->stagesurface). This operation transfers the rendered texture to the stage surface for further processing
	gs_stage_texture(hrs->stagesurface,
			 gs_texrender_get_texture(hrs->texrender));

	// Use gs_stagesurface_map to map the stage surface and retrieve the video data and line size. The video_data pointer will point to the BGRA data, and linesize will indicate the number of bytes per line
	uint8_t *video_data; // A pointer to the memory location where the BGRA data will be accessible
	uint32_t linesize; // The number of bytes per line (or row) of the image data
	// The gs_stagesurface_map function creates a mapping between the GPU memory and the CPU memory. This allows the CPU to access the pixel data directly from the GPU memory
	if (!gs_stagesurface_map(hrs->stagesurface, &video_data, &linesize)) {
		return false;
	}

	{
		// TODO: CV MAT, need a lock
	}

	// // Call our algorithm :)
	// algorithm(video_data, width, height, linesize);
	obs_log(LOG_INFO, "--------START PROCESS BGRA DATA!!!!!!");
	processBGRAData(video_data, width, height, linesize);
	obs_log(LOG_INFO, "--------END PROCESS BGRA DATA!!!!!!");

	// Use gs_stagesurface_unmap to unmap the stage surface, releasing the mapped memory.
	gs_stagesurface_unmap(hrs->stagesurface);
	return true;
}

// Create function
void *heart_rate_source_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(settings);

	obs_log(LOG_INFO, "--------------Start of CREATE!!!!!!!!!");

	struct heart_rate_source *hrs = (struct heart_rate_source *)bzalloc(
		sizeof(struct heart_rate_source));

	hrs->source = source;
	hrs->texrender = gs_texrender_create(GS_BGRA, GS_ZS_NONE);

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
	UNUSED_PARAMETER(effect);

	obs_log(LOG_INFO, "--------------Start of RENDER!!!!!!!!!");
	struct heart_rate_source *hrs =
		reinterpret_cast<struct heart_rate_source *>(data);

	if (!hrs->source) {
		obs_log(LOG_INFO, "--------NO SOURCE!!!!!!!!");
		return;
	}

	uint32_t width, height;
	getBGRAFromStageSurface(hrs, width, height);

	obs_source_skip_video_filter(hrs->source);
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

	return props;
}
