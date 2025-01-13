#include <cmath>
#include <iostream>
#include <vector>
#include <obs.h>
#include <Eigen/Dense>
#include <vector>
#include <fstream>
#include <cmath>
#include <complex>
#include <fftw3.h>

using namespace std;
using namespace Eigen;

class MovingAvg {
    private:
        int CHROMA_SIMILARITY = 100;
        
        // void chromaKey(uint8_t *y, uint8_t *u, uint8_t *v, skinKey ?) {
            
        // }

        vector<vector<vector<uint8_t>>> frame_to_vector(video_data *frame) {
            
        }
        

        vector<vector<uint8_t>> moving_average(
            int width,
            int height,
            const vector<vector<uint8_t>>& yuv,
            int n = 3
        ) {
            // Convert the input vector to an Eigen matrix
            MatrixXd input(height, width);
            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {
                    input(i, j) = static_cast<double>(yuv[i][j]);
                }
            }

            // Create an output matrix
            MatrixXd output = MatrixXd::Zero(height, width);

            // Apply moving average using a sliding window
            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {
                    // Calculate the row range for the moving average
                    int start_row = max(0, i - n / 2);
                    int end_row = min(height, i + n / 2 + 1);

                    // Calculate the moving average
                    output(i, j) = input.block(start_row, j, end_row - start_row, 1).mean();
                }
            }

            // Convert the output matrix back to a vector of vectors with uint8_t
            vector<vector<uint8_t>> result(height, vector<uint8_t>(width));
            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {
                    result[i][j] = static_cast<uint8_t>(round(output(i, j)));
                }
            }

            return result;

        }

        // TODO: Add skin key
        vector<double_t> average_keyed(std::vector<std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>> yuv) {
            double sumY = 0.0, sumU = 0.0, sumV = 0.0;
            size_t count = 0;

            // Iterate through the frame pixels using the key
            for (const auto& row : yuv) {
                for (const auto& value : row) {
                    sumY += static_cast<double>(get<0>(value));
                    sumU += static_cast<double>(get<1>(value));
                    sumV += static_cast<double>(get<2>(value));
                    count++;
                }
            }

            // Calculate averages
            if (count > 0) {
                return {sumY / count, sumU / count, sumV / count};
            } else {
                return {0.0, 0.0, 0.0}; // Handle the case where no pixels matched the key
            }
        }

        std::tuple<int, int, std::vector<std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>>> processFrame(const obs_source_frame *frame) {
            if (!frame) {
                throw std::invalid_argument("Frame is null");
            }

            // Get frame width, height, and format
            int width = frame->width;
            int height = frame->height;
            video_format format = frame->format;

            // Container for YUV values
            std::vector<std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>> yuvValues(height, std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>(width));

            if (format == VIDEO_FORMAT_I420) {
                // I420 format: Y, U, V planes
                const uint8_t *y_plane = frame->data[0];
                const uint8_t *u_plane = frame->data[1];
                const uint8_t *v_plane = frame->data[2];
                int y_stride = frame->linesize[0];
                int u_stride = frame->linesize[1];
                int v_stride = frame->linesize[2];

                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        uint8_t y_value = y_plane[y * y_stride + x];
                        uint8_t u_value = u_plane[(y / 2) * u_stride + (x / 2)];
                        uint8_t v_value = v_plane[(y / 2) * v_stride + (x / 2)];
                        yuvValues[y][x] = std::make_tuple(y_value, u_value, v_value);
                    }
                }
            } else if (format == VIDEO_FORMAT_NV12) {
                // NV12 format: Y plane, interleaved UV plane
                const uint8_t *y_plane = frame->data[0];
                const uint8_t *uv_plane = frame->data[1];
                int y_stride = frame->linesize[0];
                int uv_stride = frame->linesize[1];

                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        uint8_t y_value = y_plane[y * y_stride + x];
                        int uv_offset = (y / 2) * uv_stride + (x / 2) * 2;
                        uint8_t u_value = uv_plane[uv_offset];
                        uint8_t v_value = uv_plane[uv_offset + 1];
                        yuvValues[y][x] = std::make_tuple(y_value, u_value, v_value);
                    }
                }
            } else {
                throw std::runtime_error("Unsupported video format for YUV extraction");
            }

            return {width, height, yuvValues};
        }

        void doThing(struct obs_source_frame *source) { // Assume frame in YUV format
            auto [width, height, yuvValues] = processFrame(source);
        }

        // Function to magnify color
        std::vector<double> magnify_colour_ma(vector<vector<uint8_t>>& yuv, double delta = 50, int n_bg_ma = 60, int n_smooth_ma = 3) {
            int width = yuv[0].size();
            int height = yuv.size();

            // Return if one of the parameters are 0
            if (width == 0 || height == 0) return {};

            // Step 1: Remove slow-moving background component
            vector<vector<uint8_t>> yuv_bg_ma = moving_average(width, height, yuv, n_bg_ma);
            
            vector<vector<uint8_t>> yuv_detrended(height, vector<uint8_t>(width));
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {                   
                    // Subtract the values
                    yuv_detrended[i][j] = yuv[i][j] - yuv_bg_ma[i][j];
                }
            }


            // Step 2: Smooth the resulting PPG
            vector<vector<uint8_t>> ppg_smoothed = moving_average(width, height, yuv_detrended, n_smooth_ma);

            // Step 3: Remove NaNs (replace with 0)
            // Not necessray as we are working with uint8_t types which cannot be non-numbers

            // Step 4: Normalize to have a max delta of 'delta'
            float max_val = 0.0f;
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    max_val = std::max(max_val, std::abs(ppg_smoothed[i][j]));
                }
            }

            // Now normalize the ppg_smoothed matrix using max delta
            if (max_val > 0) {  // Avoid division by zero
                for (int i = 0; i < height; i++) {
                    for (int j = 0; j < width; j++) {
                        ppg_smoothed[i][j] = delta * ppg_smoothed[i][j] / max_val;
                    }
                }
            }

            return result;
        }

        void csv_append(const std::string& filename, const std::vector<std::vector<double>>& data) {
            std::ofstream file(filename, std::ios::app);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file for writing: " + filename);
            }

            for (const auto& row : data) {
                for (size_t i = 0; i < row.size(); ++i) {
                    file << row[i];
                    if (i < row.size() - 1) {
                        file << ",";
                    }
                }
                file << "\n";
            }

            file.close();
        }
        

        // Welch's method on the CPU to return the most common frequency
        // freq resolution = fps/nfft
        double Welch_cpu_heart_rate(const std::vector<std::vector<double>>& bvps, double fps, int nfft = 8192) {
            int num_estimators = bvps.size();
            int num_frames = bvps[0].size();
            int segment_size = 256; // = time between update * fps
            int overlap = 200;

            // Window function (Hann window)
            std::vector<double> window(segment_size);
            for (int i = 0; i < segment_size; ++i) {
                window[i] = 0.5 * (1 - std::cos(2 * M_PI * i / (segment_size - 1)));
            }

            // Frequencies
            std::vector<double> frequencies(nfft / 2 + 1);
            for (int i = 0; i <= nfft / 2; ++i) {
                frequencies[i] = i * fps / nfft;
            }

            // FFTW plan and buffers
            std::vector<double> segment(segment_size);
            std::vector<std::complex<double>> fft_output(nfft / 2 + 1);
            fftw_plan fft_plan = fftw_plan_dft_r2c_1d(nfft, segment.data(), reinterpret_cast<fftw_complex*>(fft_output.data()), FFTW_ESTIMATE);

            // Initialize a vector to store the average PSD across all estimators
            std::vector<double> avg_psd(frequencies.size(), 0.0);

            for (int estimator = 0; estimator < num_estimators; ++estimator) {
                // Divide signal into overlapping segments
                int num_segments = 0;
                std::vector<double> psd(frequencies.size(), 0.0);
                for (int start = 0; start + segment_size <= bvps[estimator].size(); start += (segment_size - overlap)) {
                    // Extract and window the segment
                    for (int j = 0; j < segment_size; ++j) {
                        segment[j] = bvps[estimator][start + j] * window[j];
                    }

                    // Perform FFT
                    fftw_execute(fft_plan);

                    // Compute power spectrum and accumulate
                    for (size_t k = 0; k < fft_output.size(); ++k) {
                        psd[k] += std::norm(fft_output[k]) / (segment_size * fps);
                    }

                    ++num_segments;
                }

                // Average PSD for this estimator
                for (size_t k = 0; k < psd.size(); ++k) {
                    psd[k] /= num_segments;
                }

                // Accumulate into the overall average PSD
                for (size_t k = 0; k < avg_psd.size(); ++k) {
                    avg_psd[k] += psd[k];
                }
            }

            // Finalize the average PSD across all estimators
            for (size_t k = 0; k < avg_psd.size(); ++k) {
                avg_psd[k] /= num_estimators;
            }

            // Find the frequency with the maximum average PSD
            size_t max_index = 0;
            for (size_t k = 1; k < avg_psd.size(); ++k) {
                if (avg_psd[k] > avg_psd[max_index]) {
                    max_index = k;
                }
            }

            // Clean up FFTW plan
            fftw_destroy_plan(fft_plan);

            // Return the most common frequency
            return frequencies[max_index];
        }

    public:
        void algorithm() {
            std::cout << "Yay" << std::endl;
        }

        

        
};