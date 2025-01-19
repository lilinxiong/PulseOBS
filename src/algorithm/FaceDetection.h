#ifndef FACE_DETECT_H
#define FACE_DETECT_H

#include <obs-module.h>
#include <opencv2/imgproc.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <vector>
#include <iostream>
#include <stdexcept>

std::vector<std::vector<bool>> detectFacesAndCreateMask(struct input_BGRA_data *frame);

#endif