#include "HeartRateAlgorithm.h"

using namespace std;
using namespace Eigen;

vector<vector<double>> MovingAvg::moving_average(
            const vector<vector<double>>& yuv,
            int n
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
vector<double_t> MovingAvg::average_keyed(std::vector<std::vector<std::tuple<double, double, double>>> yuv, 
std::vector<std::vector<bool>> skinkey) {
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

double MovingAvg::calculateHeartRate(struct input_BGRA_data *BGRA_data) { // Assume frame in YUV format: struct obs_source_frame *source
    uint8_t* rbg_values = BGRA_data->data;
    frame_data.push_back(rbg_values);

    if (frame_data.size() >= (fps * update_time)) { // Calculate heart rate when frame list "full"
        vector<vector<double>> rgbValues; // Build this from frame data
            
        std::vector<std::vector<double>> ppg_rgb_ma = magnify_colour_ma(rgbValues); 
        
        std::vector<vector<double>> ppg_w_ma;
        for (auto& f: ppg_rgb_ma) {
            std::vector<double> avg; 
            avg.push_back((f[0] + f[1] + f[2]) / 3);
            ppg_w_ma.push_back(avg);
        }

        frame_data = {}; // Naive approach - can change but just for simplicity
        prev_hr = Welch_cpu_heart_rate(ppg_w_ma, 60);
    }

    return prev_hr;
}

// Function to magnify color
std::vector<vector<double>> MovingAvg::magnify_colour_ma(const vector<vector<double>>& yuv, double delta, int n_bg_ma, int n_smooth_ma ) {
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
        
double MovingAvg::Welch_cpu_heart_rate(const std::vector<std::vector<double>>& bvps, double fps, int nfft) {
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