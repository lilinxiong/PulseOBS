#include "FaceDetection.h"

#include <graphics/matrix4.h>
#include <algorithm>

// Static variables for face detection
static cv::CascadeClassifier face_cascade, mouth_cascade, eye_cascade;
static bool cascade_loaded = false;

static void loadCascade(cv::CascadeClassifier &cascade, const char *module_name, const char *file_name)
{
	char *cascade_path = obs_find_module_file(obs_get_module(module_name), file_name);
	if (!cascade_path) {
		obs_log(LOG_INFO, "Error finding %s file!", file_name);
		throw std::runtime_error("Error finding cascade file!");
	}

	if (!cascade.load(cascade_path)) {
		obs_log(LOG_INFO, "Error loading %s!", file_name);
		throw std::runtime_error("Error loading cascade!");
	}

	bfree(cascade_path);
}

// Ensure the face cascade is loaded once
static void initializeFaceCascade()
{
	if (!cascade_loaded) {
		loadCascade(face_cascade, "pulse-obs", "haarcascade_frontalface_default.xml");
		loadCascade(eye_cascade, "pulse-obs", "haarcascade_eye.xml");
		loadCascade(mouth_cascade, "pulse-obs", "haarcascade_mcs_mouth.xml");
		cascade_loaded = true;
	}
}

static struct vec4 getNormalizedRect(const cv::Rect &region, uint32_t width, uint32_t height)
{
	float norm_min_x = static_cast<float>(region.x) / width;
	float norm_max_x = static_cast<float>(region.x + region.width) / width;
	float norm_min_y = static_cast<float>(region.y) / height;
	float norm_max_y = static_cast<float>(region.y + region.height) / height;

	struct vec4 rect;
	vec4_set(&rect, norm_min_x, norm_max_x, norm_min_y, norm_max_y);
	return rect;
}

// Function to detect faces and create a mask
std::vector<std::vector<bool>> detectFacesAndCreateMask(struct input_BGRA_data *frame,
							std::vector<struct vec4> &face_coordinates)
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

	// Detect eyes and mouth within detected faces
	for (size_t i = 0; i < faces.size(); i++) {
		if (faces.size() < 1) {
			obs_log(LOG_INFO, "No faces detected");
			break;
		}
		face_coordinates.push_back(getNormalizedRect(faces[i], width, height));

		// Define region of interest (ROI) for eyes and mouth
		cv::Mat faceROI = bgr_frame(faces[i]);
		cv::Mat gray_faceROI;
		cv::cvtColor(faceROI, gray_faceROI, cv::COLOR_BGR2GRAY);

		cv::Mat upperFaceROI =
			gray_faceROI(cv::Rect(0, 0, gray_faceROI.cols, gray_faceROI.rows / 2)); // Upper half
		cv::Mat lowerFaceROI = gray_faceROI(
			cv::Rect(0, gray_faceROI.rows / 2, gray_faceROI.cols, faceROI.rows / 2)); // Lower half

		// Detect eyes in the upper half of the face ROI
		std::vector<cv::Rect> eyes;
		eye_cascade.detectMultiScale(upperFaceROI, eyes, 1.1, 10, 0, cv::Size(15, 15));
		// for (size_t j = 0; j < eyes.size(); j++) {
		// 	cv::Rect eye = eyes[j];
		// 	cv::Point eye_center(faces[i].x + eye.x + eye.width / 2, faces[i].y + eye.y + eye.height / 2);
		// 	int radius = cvRound((eye.width + eye.height) * 0.25);
		// 	cv::circle(img, eye_center, radius, cv::Scalar(0, 255, 0), 2);
		// }

		std::vector<vec4> eye_rects;
		for (size_t i = 0; i < std::min(static_cast<size_t>(2), eyes.size()); i++) {
			const auto &eye = eyes[i];
			// Calculate absolute coordinates for the eye
			cv::Rect absolute_eye(eye.x + faces[i].x, eye.y + faces[i].y, eye.width, eye.height);

			// Push absolute eye bounding box as normalized coordinates
			face_coordinates.push_back(getNormalizedRect(absolute_eye, width, height));
		}

		// Detect mouth in the lower half of the face ROI
		std::vector<cv::Rect> mouths;
		mouth_cascade.detectMultiScale(lowerFaceROI, mouths, 1.05, 25, 0, cv::Size(30, 15));
		// for (size_t j = 0; j < mouths.size(); j++) {
		// 	cv::Rect mouth = mouths[j];
		// 	mouth.y += faceROI.rows / 2; // Adjust mouth position relative to the full face ROI
		// 	cv::Point top_left(faces[i].x + mouth.x, faces[i].y + mouth.y);
		// 	cv::Point bottom_right(faces[i].x + mouth.x + mouth.width, faces[i].y + mouth.y + mouth.height);
		// 	cv::rectangle(img, top_left, bottom_right, cv::Scalar(0, 0, 255), 2);
		// }

		std::vector<vec4> mouth_rects;
		for (size_t i = 0; i < std::min(static_cast<size_t>(1), mouths.size()); i++) {
			const auto &mouth = mouths[i];
			// Calculate absolute coordinates for the mouth
			cv::Rect absolute_mouth(mouth.x + faces[i].x, mouth.y + faces[i].y + faceROI.rows / 2,
						mouth.width, mouth.height);

			// Push absolute mouth bounding box as normalized coordinates
			face_coordinates.push_back(getNormalizedRect(absolute_mouth, width, height));
		}
	}

	// Initialize a 2D boolean mask
	std::vector<std::vector<bool>> face_mask(height, std::vector<bool>(width, false));

	// Mark pixels within detected face regions as true
	for (const auto &face : faces) {
		obs_log(LOG_INFO, "FACE FACE FACE");
		for (int y = face.y; y < face.y + face.height && y < static_cast<int>(height); ++y) {
			for (int x = face.x; x < face.x + face.width && x < static_cast<int>(width); ++x) {
				face_mask[y][x] = true;
			}
		}
	}

	return face_mask;
}
