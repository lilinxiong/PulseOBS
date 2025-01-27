#include "FaceDetection.h"
#include <obs-module.h>
#include "plugin-support.h"
#include "HeartRateAlgorithm.h"
#include <fstream>
#include <string>

using namespace std;
using namespace Eigen;

// Calculating the average/mean RGB values of a frame
vector<double_t> MovingAvg::averageRGB(vector<vector<vector<uint8_t>>> rgb)
{
	double sumR = 0.0, sumG = 0.0, sumB = 0.0;
	double count = 0;

	// Iterate through the frame pixels using the key
	for (int i = 0; i < static_cast<int>(rgb.size()); ++i) {
		for (int j = 0; j < static_cast<int>(rgb[0].size()); ++j) {
			sumR += rgb[i][j][0];
			sumG += rgb[i][j][1];
			sumB += rgb[i][j][2];
			count++;
		}
	}

	return {sumR / count, sumG / count, sumB / count};
}

vector<double_t> green(vector<vector<double_t>> framesRGB)
{
	vector<double_t> framesG;

	for (int i = 0; i < static_cast<int>(framesRGB.size()); i++) {
		framesG.push_back(framesRGB[i][1]);
	}

	return framesG;
}

void MovingAvg::updateWindows(vector<double_t> frame_avg)
{
	if (windows.empty()) {
		windows.push_back({frame_avg});
		return;
	}

	vector<vector<double_t>> last = windows.back();

	if (static_cast<int>(last.size()) == windowSize) {
		vector<vector<double_t>> window = vector<vector<double_t>>(last.end() - windowStride, last.end());
		window.push_back(frame_avg);
		windows.push_back(window);
	} else {
		windows.pop_back();
		last.push_back(frame_avg);
		windows.push_back(last);
	}

	if (static_cast<int>(windows.size()) > maxNumWindows) {
		windows.erase(windows.begin());
	}
}

vector<vector<vector<uint8_t>>> extractRGB(struct input_BGRA_data *BGRA_data)
{
	uint8_t *data = BGRA_data->data;
	uint32_t width = BGRA_data->width;
	uint32_t height = BGRA_data->height;
	uint32_t linesize = BGRA_data->linesize;

	vector<vector<vector<uint8_t>>> frameRGB(height, vector<vector<uint8_t>>(width));

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			uint8_t B = data[y * linesize + x * 4];
			uint8_t G = data[y * linesize + x * 4 + 1];
			uint8_t R = data[y * linesize + x * 4 + 2];

			frameRGB[x][y] = {R, G, B};
		}
	}

	return frameRGB;
}

double MovingAvg::calculateHeartRate(struct input_BGRA_data *BGRA_data, int preFilter, int ppg, int postFilter)
{ // Assume frame in YUV format: struct obs_source_frame *source
	UNUSED_PARAMETER(preFilter);
	UNUSED_PARAMETER(postFilter);

	// NOTE: This code should be processed for each ROI, so shouldn't be here (didn't have time to)

	vector<vector<vector<uint8_t>>> frameRGB = extractRGB(BGRA_data);
	vector<double_t> avg = averageRGB(frameRGB);
	updateWindows(avg);

	vector<double_t> ppgSignal;

	switch (ppg) {
	case 0:
		ppgSignal = green(windows.back()); // Placeholder

	default:
		break;
	}

	return 0.0;
}
