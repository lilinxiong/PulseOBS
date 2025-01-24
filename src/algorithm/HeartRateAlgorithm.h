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
	std::vector<double_t> average_keyed(
		std::vector<std::vector<std::tuple<double, double, double>>> rgb,
		std::vector<std::vector<bool>> skinkey = {});

public:
	double calculateHeartRate(struct input_BGRA_data *BGRA_data);
};

#endif
