#include <obs-module.h>
#include <graphics/graphics.h>
#include <media-io/video_frame.h>
#include <util/platform.h>
#include "pulse.hpp"
#include "heart_rate_widget.hpp"

static void *heart_rate_source_create(obs_data_t *settings, obs_source_t *source);
static void heart_rate_source_destroy(void *data);
static void heart_rate_source_tick(void *data, float seconds);
static void heart_rate_source_render(void *data, gs_effect_t *effect);

extern HeartRateWidget *heartRateWidget = nullptr;

obs_source_info *heart_rate_source_info  = {
  .id = "heart_rate_source",
  .type = OBS_SOURCE_TYPE_INPUT,
  .output_flags = OBS_SOURCE_VIDEO,
  .get_name = []() {
    return "Heart Rate Monitor";
  },
  .create = heart_rate_source_create,
  .destroy = heart_rate_source_destroy,
  .video_tick = heart_rate_source_tick,
  .video_render = heart_rate_source_render,
}

// Structure to hold source data
struct heart_rate_source {
    obs_source_t *source;
    // Add fields for any necessary state or data
};

// Create function
static void *heart_rate_source_create(obs_data_t *settings, obs_source_t *source) {
    struct heart_rate_source *hrs = (struct heart_rate_source *)bzalloc(sizeof(struct heart_rate_source));
    hrs->source = source;
    return hrs;
}

// Destroy function
static void heart_rate_source_destroy(void *data) {
    bfree(data);
}

// Tick function
static void heart_rate_source_tick(void *data, float seconds) {
    struct heart_rate_source *hrs = (struct heart_rate_source *)data;
    UNUSED_PARAMETER(seconds);
    // Logic to update per frame
}

int calculateHeartRate(uint8_t *data, uint32_t width, uint32_t height) {
    // Logic to calculate heart rate
    return 100;
}

// Render function
static void heart_rate_source_render(void *data, gs_effect_t *effect) {
    struct heart_rate_source *hrs = (struct heart_rate_source *)data;
    UNUSED_PARAMETER(effect);

    struct obs_source_frame *frame = obs_source_get_frame(hrs->source);
    if (frame) {
        // Extract RGB data and process it for heart rate
        int heartRate = calculateHeartRate(frame->data, frame->width, frame->height);

        obs_source_release_frame(hrs->source, frame);

        if (heartRateWidget) {
          heartRateWidget->updateHeartRate(heartRate);
        }
    }
}
