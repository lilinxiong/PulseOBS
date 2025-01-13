#ifndef PULSE_H
#define PULSE_H

#include <obs-module.h>
#include "heart_rate_widget.hpp"

extern HeartRateWidget *heartRateWidget;

// Structure to hold source data
struct heart_rate_source {
    obs_source_t *source;
};

#endif