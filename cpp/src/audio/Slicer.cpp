#include "Slicer.h"
#include <cmath>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

Slicer::Slicer(const Sample& sample, int window_size, int overlap_ratio, int local_energy_window)
    : sample_(sample),
      window_size_(window_size),
      overlap_ratio_(overlap_ratio),
      local_energy_window_(local_energy_window),
      sensitivity_(DEFAULT_SENSITIVITY)
{
    channels_ = sample_.as_samples();
    extract_markers(channels_);
}

void Slicer::extract_markers() {
    extract_markers(channels_);
}

void Slicer::set_sensitivity(int sensitivity) {
    sensitivity_ = sensitivity;
    extract_markers();
}

void Slicer::extract_markers(const std::vector<std::vector<int>>& channels) {
    markers_.clear(sample_.frame_length(),
                   sample_.format().sample_rate);

    int step = window_size_ / overlap_ratio_;
    auto history = energy_history(channels, window_size_, overlap_ratio_);

    if (history.empty() || static_cast<int>(history.size()) < local_energy_window_) {
        markers_.add(0);
        return;
    }

    bool last_state = false;
    const std::vector<int>& samples_l = channels[0];

    for (int i = 0; i < static_cast<int>(history.size()); i++) {
        long long e      = history[i];
        long long local_e = local_energy(i, history, local_energy_window_);
        double C = static_cast<double>(sensitivity_) / 100.0;

        if (local_e > 0 && e > C * local_e) {
            if (!last_state) {
                int location = i * step;
                int adjusted = nearest_zero_crossing(samples_l, location, window_size_);
                markers_.add(adjusted);
                last_state = true;
            }
        } else {
            last_state = false;
        }
    }
}

std::vector<long long> Slicer::energy_history(
    const std::vector<std::vector<int>>& channels,
    int window_size, int overlap_ratio)
{
    const std::vector<int>& L = channels[0];
    const std::vector<int>& R = (channels.size() > 1) ? channels[1] : channels[0];
    int n    = static_cast<int>(L.size());
    int step = window_size / overlap_ratio;
    int num_frames = n / step;

    if (num_frames < 1) return {};

    std::vector<long long> energy(num_frames);
    int wi = 0;
    for (int i = 0; i + window_size < n; i += step) {
        long long sum = 0;
        for (int j = 0; j < window_size; j++) {
            long long sl = L[i + j];
            long long sr = R[i + j];
            sum += sl * sl + sr * sr;
        }
        energy[wi++] = sum;
    }
    energy.resize(wi);
    return energy;
}

long long Slicer::local_energy(int i, const std::vector<long long>& history, int local_window) {
    int n = static_cast<int>(history.size());
    int m = local_window;
    int from, to;
    if (i < m) {
        from = 0; to = m;
    } else if (i + m < n) {
        from = i; to = i + m;
    } else {
        from = n - m; to = n;
    }
    long long sum = 0;
    for (int j = from; j < to; j++) sum += history[j];
    return sum / m;
}

int Slicer::nearest_zero_crossing(const std::vector<int>& samples, int index, int excursion) {
    if (index == 0) return 0;
    int i   = index;
    int min = (index - excursion >= 0) ? (index - excursion) : 0;
    while (!is_zero_cross(samples, i) && i > min)
        i--;
    return i;
}

bool Slicer::is_zero_cross(const std::vector<int>& samples, int index) {
    int n = static_cast<int>(samples.size());
    if (index == 0 || index >= n - 1) return true;
    int a = samples[index - 1];
    int b = samples[index];
    return (a > 0 && b < 0) || (a < 0 && b > 0) || (a == 0 && b != 0);
}

Sample Slicer::get_slice(int marker_index) const {
    LocationRange r = markers_.get_range_from(marker_index);
    return sample_.sub_region(r.from, r.to);
}

Sample Slicer::get_selected_slice() const {
    return get_slice(markers_.get_selected_index());
}

int Slicer::adjust_zero_crossing(int location, int excursion) const {
    if (channels_.empty()) return location;
    return nearest_zero_crossing(channels_[0], location, excursion);
}

std::vector<std::string> Slicer::export_slices(const std::string& dir_path,
                                                 const std::string& prefix) const {
    std::vector<std::string> files;
    int n = markers_.size();
    for (int i = 0; i < n; i++) {
        Sample slice = get_slice(i);
        std::string path = dir_path + "/" + prefix + std::to_string(i) + ".wav";
        slice.save(path);
        files.push_back(path);
    }
    return files;
}
