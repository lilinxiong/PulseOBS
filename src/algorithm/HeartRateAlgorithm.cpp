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


using namespace std;
using namespace Eigen;

class MovingAvg {
    private:
        int CHROMA_SIMILARITY = 100;
        
        // vector<vector<vector<uint8_t>>> frame_to_vector(video_data *frame) {
            
        // }
        

        vector<vector<double>> moving_average(
            const vector<vector<double>>& yuv,
            int n = 3
        ) {
            size_t width = yuv[0].size();
            size_t height = yuv.size();
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
                    int end_row = min(static_cast<int>(height), i + n / 2 + 1);

                    // Calculate the moving average
                    output(i, j) = input.block(start_row, j, end_row - start_row, 1).mean();
                }
            }

            // Convert the output matrix back to a vector of vectors with uint8_t
            vector<vector<double>> result(height, vector<double>(width));
            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {
                    result[i][j] = output(i, j);
                }
            }

            return result;

        }

        // TODO: Add skin key
        vector<double_t> average_keyed(std::vector<std::vector<std::tuple<double, double, double>>> yuv, std::vector<std::vector<bool>> skinkey = {}) {
            double sumY = 0.0, sumU = 0.0, sumV = 0.0;
            size_t count = 0;

            // Iterate through the frame pixels using the key
            for (int i = 0; i < yuv.size(); ++i) {
                for (int j = 0; j < yuv[0].size(); ++j) {
                    if (skinkey.size() != 0) {
                        if (skinkey[i][j]) {
                            sumY += get<0>(yuv[i][j]);
                            sumU += get<1>(yuv[i][j]);
                            sumV += get<2>(yuv[i][j]);
                            count++;
                        }
                    } else {
                        sumY += get<0>(yuv[i][j]);
                        sumU += get<1>(yuv[i][j]);
                        sumV += get<2>(yuv[i][j]);
                        count++;
                    }
                    
                }
            }

            // Calculate averages
            if (count > 0) {
                return {sumY / count, sumU / count, sumV / count};
            } else {
                return {0.0, 0.0, 0.0}; // Handle the case where no pixels matched the key
            }
        }

        std::vector<std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>> processFrame(const obs_source_frame *frame) {
            if (!frame) {
                throw std::invalid_argument("Frame is null");
            }

            // Get frame width, height, and format
            size_t width = frame->width;
            size_t height = frame->height;
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

            return yuvValues;
        }

        uint8_t generateRandomNumber() {
            return static_cast<uint8_t>(rand() % 256);
        }

        vector<vector<tuple<double, double, double>>> randomFrame() {
            // Define the structure: a vector of 3x3 vectors containing tuples
            std::vector<std::vector<std::tuple<double, double, double>>> frame(3);

            for (int i = 0; i < 3; ++i) {
                std::vector<std::tuple<double, double, double>> matrix;
                for (int j = 0; j < 3; ++j) { // 3x3 matrix has 9 elements
                    // Generate a tuple with three random numbers
                    std::tuple<double, double, double> element = {
                        static_cast<double>(generateRandomNumber()),
                        static_cast<double>(generateRandomNumber()),
                        static_cast<double>(generateRandomNumber())
                    };
                    matrix.push_back(element);
                }
                frame[i] = matrix;
            }

            return frame;
        }

        double doThing() { // Assume frame in YUV format: struct obs_source_frame *source

            // auto [width, height, yuvValues] = processFrame(source);
            srand(static_cast<unsigned>(time(0)));
            vector<vector<double>> yuvValues; 
            for (int frame = 0; frame < 10; frame++) {
                std::vector<std::vector<std::tuple<double, double, double>>> testData = randomFrame();
                yuvValues.push_back(average_keyed(testData));
            }
            
            std::vector<std::vector<double>> ppg_rgb_ma = magnify_colour_ma(yuvValues); 
            
            std::vector<vector<double>> ppg_w_ma;
            for (auto& f: ppg_rgb_ma) {
                std::vector<double> avg; 
                avg.push_back((f[0] + f[1] + f[2]) / 3);
                ppg_w_ma.push_back(avg);
            }
            return Welch_cpu_heart_rate(ppg_w_ma, 60);
        }

        // Function to magnify color
        std::vector<vector<double>> magnify_colour_ma(const vector<vector<double>>& yuv, double delta = 50, int n_bg_ma = 60, int n_smooth_ma = 3) {
            size_t height = yuv.size();
            size_t width = yuv[0].size();

            // Return if one of the parameters are 0
            if (width == 0 || height == 0) return {};

            // Step 1: Remove slow-moving background component
            vector<vector<double>> yuv_bg_ma = moving_average(yuv, n_bg_ma);
            
            vector<vector<double>> yuv_detrended(height, vector<double>(width));
            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {                   
                    // Subtract the values
                    yuv_detrended[i][j] = yuv[i][j] - yuv_bg_ma[i][j];
                }
            }


            // Step 2: Smooth the resulting PPG
            vector<vector<double>> ppg_smoothed = moving_average(yuv_detrended, n_smooth_ma);

            // Step 3: Remove NaNs (replace with 0)
            // Not necessray as we are working with uint8_t types which cannot be non-numbers

            // Step 4: Normalize to have a max delta of 'delta'
            int8_t max_val = 0;
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    // TODO: We are not sure if we are working with unsigend numers correctly, might need to look after conversion
                    max_val = std::max(max_val, static_cast<int8_t>(std::abs(ppg_smoothed[i][j])));
                }
            }

            // Now normalize the ppg_smoothed matrix using max delta
            if (max_val > 0) {  // Avoid division by zero
                for (int i = 0; i < height; i++) {
                    for (int j = 0; j < width; j++) {
                       // TODO: We are not sure if we are working with unsigned numbers correctly, might need to look after conversion
                        ppg_smoothed[i][j] = static_cast<double>(delta * ppg_smoothed[i][j] / max_val);
                    }
                }
            }

            return ppg_smoothed; 
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
        double Welch_cpu_heart_rate(const std::vector<std::vector<double>>& bvps, double fps, int nfft = 16) {
            using Eigen::ArrayXd;

            int num_estimators = static_cast<int>(bvps.size());
            int num_frames = static_cast<int>(bvps[0].size());

            // Define segment size and overlap
            int segment_size = nfft;
            int overlap = nfft / 2; // 50% overlap

            // Hann window
            ArrayXd hann_window(segment_size);
            for (int i = 0; i < segment_size; ++i) {
                hann_window[i] = 0.5 * (1 - std::cos(2 * M_PI * i / (segment_size - 1)));
            }

            // Frequencies
            ArrayXd frequencies = ArrayXd::LinSpaced(nfft / 2 + 1, 0, fps / 2);

            // Initialize average PSD
            ArrayXd avg_psd = ArrayXd::Zero(nfft / 2 + 1);

            // Function to compute DFT manually
            auto computeDFT = [](const ArrayXd& input) -> Eigen::ArrayXcd {
                int N = static_cast<int>(input.size());
                Eigen::ArrayXcd output(N);

                for (int k = 0; k < N; ++k) {
                    std::complex<double> sum(0.0, 0.0);
                    for (int n = 0; n < N; ++n) {
                        double angle = -2.0 * M_PI * k * n / N;
                        sum += input[n] * std::exp(std::complex<double>(0, angle));
                    }
                    output[k] = sum;
                }

                return output;
            };

            // Iterate over all estimators
            for (const auto& bvp : bvps) {
                // Convert signal to Eigen array
                ArrayXd signal = Eigen::Map<const ArrayXd>(bvp.data(), bvp.size());

                // Divide signal into overlapping segments
                int num_segments = 0;
                ArrayXd psd = ArrayXd::Zero(nfft / 2 + 1);

                for (int start = 0; start + segment_size <= num_frames; start += (segment_size - overlap)) {
                    // Extract segment and apply window
                    ArrayXd segment = signal.segment(start, segment_size) * hann_window;

                    // Compute DFT
                    Eigen::ArrayXcd fft_result = computeDFT(segment);

                    // Compute power spectrum
                    for (int k = 0; k <= nfft / 2; ++k) {
                        psd[k] += std::norm(fft_result[k]) / (segment_size * fps);
                    }

                    ++num_segments;
                }

                // Average PSD for this estimator
                if (num_segments > 0) {
                    psd /= num_segments;
                }

                // Accumulate into the overall average PSD
                avg_psd += psd;
            }

            // Finalize average PSD across all estimators
            avg_psd /= num_estimators;

            // Find the frequency with the maximum average PSD
            int max_index;
            avg_psd.maxCoeff(&max_index);

            return frequencies[max_index];
        }
        
    public:
        double algorithm() {
            return doThing();
        }   
};

int main() {
    MovingAvg avg;
    cout << "hellllo are you thereeeeee" << endl;
    cout << avg.algorithm() << endl;
    return 0;
}