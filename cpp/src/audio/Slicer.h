#pragma once
#include "Sample.h"
#include "Markers.h"
#include <string>
#include <vector>

class Slicer {
public:
    static constexpr int DEFAULT_WINDOW_SIZE        = 1024;
    static constexpr int DEFAULT_OVERLAP_RATIO      = 2;
    static constexpr int DEFAULT_LOCAL_ENERGY_WINDOW = 20;
    static constexpr int DEFAULT_SENSITIVITY        = 130;

    Slicer(const Sample& sample,
           int window_size        = DEFAULT_WINDOW_SIZE,
           int overlap_ratio      = DEFAULT_OVERLAP_RATIO,
           int local_energy_window = DEFAULT_LOCAL_ENERGY_WINDOW);

    void extract_markers();
    void set_sensitivity(int sensitivity);
    int  get_sensitivity() const { return sensitivity_; }

    const Markers& get_markers() const { return markers_; }
    Markers&       get_markers()       { return markers_; }

    Sample get_slice(int marker_index) const;
    Sample get_selected_slice() const;

    // Export each slice as a WAV file: <path>/<prefix><n>.wav
    std::vector<std::string> export_slices(const std::string& dir_path,
                                           const std::string& prefix) const;

    int frame_length() const { return sample_.frame_length(); }

    // Public zero-crossing finder — used by Markers::nudge
    int adjust_zero_crossing(int location, int excursion) const;

    // Channel data access for waveform rendering
    const std::vector<std::vector<int>>& get_channels() const { return channels_; }

private:
    void extract_markers(const std::vector<std::vector<int>>& channels);

    static std::vector<long long> energy_history(
        const std::vector<std::vector<int>>& channels,
        int window_size, int overlap_ratio);

    static long long local_energy(int i, const std::vector<long long>& history,
                                  int local_window);

    static int  nearest_zero_crossing(const std::vector<int>& samples,
                                      int index, int excursion);
    static bool is_zero_cross(const std::vector<int>& samples, int index);

    const Sample& sample_;
    std::vector<std::vector<int>> channels_;
    int window_size_;
    int overlap_ratio_;
    int local_energy_window_;
    int sensitivity_;
    Markers markers_;
};
