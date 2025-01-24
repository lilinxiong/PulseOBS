#include "FaceDetection.h"

// Static variables for face detection
static cv::CascadeClassifier face_cascade;
static bool cascade_loaded = false;

// Ensure the face cascade is loaded once
static void initializeFaceCascade()
{
	if (!cascade_loaded) {
		char *cascade_path =
			obs_find_module_file(obs_get_module("pulse-obs"), "haarcascade_frontalface_default.xml");
		if (!cascade_path) {
			obs_log(LOG_INFO, "Error finding face cascade file!");
			throw std::runtime_error("Error finding face cascade file!");
		}

		if (!face_cascade.load(cascade_path)) {
			obs_log(LOG_INFO, "Error loading face cascade!");
			throw std::runtime_error("Error loading face cascade!");
		}

		bfree(cascade_path);
		cascade_loaded = true;
	}
}

// Function to detect faces and create a mask
std::vector<std::vector<bool>> detectFacesAndCreateMask(struct input_BGRA_data *frame)
{
	if (!frame || !frame->data) {
		throw std::runtime_error("Invalid BGRA frame data!");
	}

	// Initialize the face cascade
	initializeFaceCascade();

	// Extract frame parameters
	uint8_t *data = frame->data;
	uint32_t width = frame->width;
	uint32_t height = frame->height;
	uint32_t linesize = frame->linesize;

	// Create an OpenCV Mat for the BGRA frame
	// `linesize` specifies the number of bytes per row, which can include padding
	cv::Mat bgra_frame(height, linesize / 4, CV_8UC4, data);

	// Crop to remove padding if linesize > width * 4
	cv::Mat cropped_bgra_frame = bgra_frame(cv::Rect(0, 0, width, height));

	// Convert BGRA to BGR (OpenCV processes images in BGR format)
	cv::Mat bgr_frame;
	cv::cvtColor(cropped_bgra_frame, bgr_frame, cv::COLOR_BGRA2BGR);

	// Detect faces
	std::vector<cv::Rect> faces;
	face_cascade.detectMultiScale(bgr_frame, faces, 1.1, 10, 0, cv::Size(30, 30));

	// Initialize a 2D boolean mask
	std::vector<std::vector<bool>> face_mask(height, std::vector<bool>(width, false));

	// Mark pixels within detected face regions as true
	for (const auto &face : faces) {
		for (int y = face.y; y < face.y + face.height && y < static_cast<int>(height); ++y) {
			for (int x = face.x; x < face.x + face.width && x < static_cast<int>(width); ++x) {
				face_mask[y][x] = true;
			}
		}
	}

	return face_mask;
}
