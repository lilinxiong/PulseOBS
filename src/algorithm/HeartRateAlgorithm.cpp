#include "HeartRateAlgorithm.h"

using namespace std;
using namespace Eigen;


// Calculating the moving average across a vector of frames
vector<vector<double>> MovingAvg::moving_average(
            const vector<vector<double>>& rgb,
            int n
        ) {
    size_t width = rgb[0].size();
    size_t height = rgb.size();
    // Convert the input vector to an Eigen matrix
    MatrixXd input(height, width);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            input(i, j) = static_cast<double>(rgb[i][j]);
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


// Calculating the average/mean RGB values of a frame
vector<double_t> MovingAvg::average_keyed(std::vector<std::vector<std::tuple<double, double, double>>> rgb, 
std::vector<std::vector<bool>> skinkey) {
    double sumR = 0.0, sumG = 0.0, sumB = 0.0;
    size_t count = 0;

    // Iterate through the frame pixels using the key
    for (int i = 0; i < rgb.size(); ++i) {
        for (int j = 0; j < rgb[0].size(); ++j) {
            if (skinkey.size() != 0) {
                if (skinkey[i][j]) {
                    sumR += get<0>(rgb[i][j]);
                    sumG += get<1>(rgb[i][j]);
                    sumB += get<2>(rgb[i][j]);
                    count++;
                }
            } else {
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

double MovingAvg::calculateHeartRate(struct input_BGRA_data *BGRA_data) { // Assume frame in YUV format: struct obs_source_frame *source
    uint8_t *data = BGRA_data->data;
    uint32_t width = BGRA_data->width;
    uint32_t height = BGRA_data->height;
    uint32_t linesize = BGRA_data->linesize;
    
    uint32_t pixel_count = width * height;
    // Create a 2D vector to store RGB tuples
    std::vector<std::vector<std::tuple<double, double, double>>> rgb(height, std::vector<std::tuple<double, double, double>>(width));

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint8_t B = data[y * linesize + x * 4 + 0];
            uint8_t G = data[y * linesize + x * 4 + 1];
            uint8_t R = data[y * linesize + x * 4 + 2];

            // Store as a tuple in the vector
            rgb[y][x] = std::make_tuple(R, G, B);
        }
    }

    // We can ignore the face detection part and use the whole frame (all the pixels on the frame) for now
    //std::vector<std::vector<bool>> skinKey = stddetectFacesAndCreateMask(BGRA_data);
    std::vector<std::vector<bool>> skinKey(height, std::vetcor<bool>(width, true));
    vector<double_t> averageRGBValues = average_keyed(rgb, skinkey);


    frame_data.push_back(averageRGBValues);

    if (frame_data.size() >= (fps * update_time)) { // Calculate heart rate when frame list "full"
            
        std::vector<std::vector<double>> ppg_rgb_ma = magnify_colour_ma(frame_data); 
        
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
std::vector<vector<double>> MovingAvg::magnify_colour_ma(const vector<vector<double>>& rgb, double delta, int n_bg_ma, int n_smooth_ma ) {
    size_t height = rgb.size();
    size_t width = rgb[0].size();

    // Return if one of the parameters are 0
    if (width == 0 || height == 0) return {};

    // Step 1: Remove slow-moving background component
    vector<vector<double>> rgb_bg_ma = moving_average(rgb, n_bg_ma);
    
    vector<vector<double>> rgb_detrended(height, vector<double>(width));
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {                   
            // Subtract the values
            rgb_detrended[i][j] = rgb[i][j] - rgb_bg_ma[i][j];
        }
    }


    // Step 2: Smooth the resulting PPG
    vector<vector<double>> ppg_smoothed = moving_average(rgb_detrended, n_smooth_ma);

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