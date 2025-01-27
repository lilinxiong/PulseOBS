#include "FaceDetection.h"
#include <obs-module.h>
#include "plugin-support.h"
#include "HeartRateAlgorithm.h"
#include <fstream>
#include <string>

using namespace std;
using namespace Eigen;

// Calculating the average/mean RGB values of a frame
vector<double_t> MovingAvg::average_keyed(std::vector<std::vector<std::tuple<double, double, double>>> rgb,
					  std::vector<std::vector<bool>> skinkey)
{
	double sumR = 0.0, sumG = 0.0, sumB = 0.0;
	double count = 0;

	// Iterate through the frame pixels using the key
	for (int i = 0; i < static_cast<int>(rgb.size()); ++i) {
		for (int j = 0; j < static_cast<int>(rgb[0].size()); ++j) {
			if (skinkey[i][j]) {
				sumR += get<0>(rgb[i][j]);
				sumG += get<1>(rgb[i][j]);
				sumB += get<2>(rgb[i][j]);
				count++;
			}
		}
	}

	// Calculate averages
	if (count > 0) {
		return {sumR / count, sumG / count, sumB / count};
	} else {
		return {0.0, 0.0, 0.0}; // Handle the case where no pixels matched the key
	}
}

vector<double_t> green(vector<vector<double_t>> framesRGB)
{
	vector<double_t> framesG;

	for (int i = 0; i < static_cast<int>(framesRGB.size()); i++) {
		framesG.push_back(framesRGB[i][1]);
	}

	return framesG;
}

void MovingAvg::updateWindows(double_t frame_avg)
{
	if (windows.empty()) {
		windows.push_back({frame_avg});
		return;
	}

	vector<double_t> last = windows.back();

	if (windows.back().size() == windowSize) {
		vector<double_t> window = vector<double_t>(last.end() - windowStride, last.end());
		window.push_back(frame_avg);
		windows.push_back(window);
	} else {
		windows.pop_back();
		last.push_back(frame_avg);
		windows.push_back(last);
	}
}

double MovingAvg::calculateHeartRate(struct input_BGRA_data *BGRA_data, int preFilter, int ppg, int postFilter)
{ // Assume frame in YUV format: struct obs_source_frame *source
	UNUSED_PARAMETER(BGRA_data);
	UNUSED_PARAMETER(preFilter);
	UNUSED_PARAMETER(postFilter);

	vector<vector<double_t>> framesRGB;
	vector<double_t> ppgSignal;

	switch (ppg) {
	case 0:
		ppgSignal = green(framesRGB);

	default:
		break;
	}

	return 0.0;
}
