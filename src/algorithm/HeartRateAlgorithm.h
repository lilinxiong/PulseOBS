#ifndef HEART_RATE_ALGO_H
#define HEART_RATE_ALGO_H

#include <cmath>
#include <iostream>
#include <vector>
#include <obs.h>
#include <Eigen/Dense>
#include <vector>
#include <fstream>
#include <cmath>
#include <complex>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include "heart_rate_source.h"

class MovingAvg {
private:
	double fps = 60.0;
	double update_time = 2.0; // Time between heart rate updates in secs
	int maxBufSize = 60;
	double prev_hr = 0.0;
	std::vector<std::vector<double>> frame_data = {}; // Store the average RGB values for each frame

	std::vector<std::vector<double>> moving_average(const std::vector<std::vector<double>> &rgb, int n = 3);

	std::vector<double_t> average_keyed(std::vector<std::vector<std::tuple<double, double, double>>> rgb,
					    std::vector<std::vector<bool>> skinkey = {});

	std::vector<std::vector<double>> magnify_colour_ma(const std::vector<std::vector<double>> &rgb,
							   double delta = 50, int n_bg_ma = 60, int n_smooth_ma = 3);

	double Welch_cpu_heart_rate(const std::vector<std::vector<double>> &bvps, int num_data_points);

public:
	double calculateHeartRate(struct input_BGRA_data *BGRA_data, std::vector<struct vec4> &face_coordinates);
};

#endif
