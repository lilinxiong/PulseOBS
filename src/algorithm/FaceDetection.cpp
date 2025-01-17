#include <obs-module.h>
#include <opencv2/imgproc.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <vector>
#include <iostream>
#include <stdexcept>


static cv::CascadeClassifier face_cascade;
static bool cascade_loaded = false;

if (!cascade_loaded) {
    if (!face_cascade.load("haarcascade_frontalface_default.xml")) {
        std::cerr << "Error loading face cascade!" << std::endl;
        return;
    }
    cascade_loaded = true;
}



// Function to detect faces and create a mask
std::vector<std::vector<bool>> detectFacesAndCreateMask(obs_source_frame *source, cv::CascadeClassifier &face_cascade) {
    if (!source) {
        throw std::runtime_error("Invalid OBS frame!");
    }

    // Verify the format (assume NV12 for this example)
    if (source->format != VIDEO_FORMAT_NV12) {
        throw std::runtime_error("Unsupported frame format! Expected NV12.");
    }

    // Retrieve frame dimensions and data
    int width = source->width;
    int height = source->height;

    uint8_t *y_plane = source->data[0]; // Y plane
    uint8_t *uv_plane = source->data[1]; // UV plane (interleaved for NV12)

    // Create OpenCV Mat for YUV
    cv::Mat y_channel(height, width, CV_8UC1, y_plane);
    cv::Mat uv_channel(height / 2, width / 2, CV_8UC2, uv_plane); // UV interleaved

    // Convert YUV (NV12) to BGR
    cv::Mat yuv_frame;
    cv::merge(std::vector<cv::Mat>{y_channel, uv_channel}, yuv_frame);
    cv::Mat bgr_frame;
    cv::cvtColor(yuv_frame, bgr_frame, cv::COLOR_YUV2BGR_NV12);

    // Detect faces
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(bgr_frame, faces, 1.1, 10, 0, cv::Size(30, 30));

    // Initialize 2D boolean mask
    std::vector<std::vector<bool>> face_mask(height, std::vector<bool>(width, false));

    // Mark pixels within detected face regions as true
    for (const auto &face : faces) {
        for (int y = face.y; y < face.y + face.height && y < height; ++y) {
            for (int x = face.x; x < face.x + face.width && x < width; ++x) {
                face_mask[y][x] = true;
            }
        }
    }

    return face_mask;
}



