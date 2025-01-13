#include <obs-module.h>
#include <dlib/image_processing.h>
#include <dlib/image_io.h>
#include <dlib/opencv.h>

// Initialize dlib face detector
dlib::frontal_face_detector face_detector = dlib::get_frontal_face_detector();

// This function is called whenever a frame is received from the OBS source
void on_frame_received(obs_source_t *source, obs_video_info *frame_info) {
    // Assuming frame_info->data[0] contains the raw frame data (BGR or RGBA)
    // Convert the frame data to dlib::cv_image format
    dlib::cv_image<dlib::bgr_pixel> img(frame_info->data[0]);

    // Detect faces in the frame
    std::vector<dlib::rectangle> faces = face_detector(img);

    // If a face is detected, crop it
    if (!faces.empty()) {
        // Take the first detected face (you could iterate over faces if there are multiple)
        dlib::rectangle face = faces[0];

        // Crop the face region
        dlib::matrix<dlib::bgr_pixel> face_region;
        dlib::extract_image_chip(img, dlib::get_face_chip_details(face), face_region);

        // Convert the cropped face back to a format OBS can render (e.g., BGR)
        unsigned char* cropped_face_data = face_region.nc_data();

        // Here you could process the cropped face data or use it later for other tasks
        // For now, you can return the cropped face to OBS or save it for later
        obs_source_video_render(source, cropped_face_data);  // Example placeholder function for rendering
    } else {
        // If no face is detected, you can either do nothing or process the frame as-is
        obs_source_video_render(source, frame_info->data[0]);  // Example placeholder function for rendering
    }
}