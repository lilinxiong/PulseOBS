#include <obs-module.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *get_heart_rate_source_name(void *);
void *heart_rate_source_create(obs_data_t *settings, obs_source_t *source);
void heart_rate_source_destroy(void *data);
void heart_rate_source_tick(void *data, float seconds);
void heart_rate_source_render(void *data, gs_effect_t *effect);
uint32_t heart_rate_source_get_width(void *data);
uint32_t heart_rate_source_get_height(void *data);
obs_properties_t *heart_rate_source_properties(void *data);

#ifdef __cplusplus
}
#endif
