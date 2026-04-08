#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct AudioFormat {
    int sample_rate;
    int channels;
    int bit_depth;
};

// Holds decoded PCM audio data loaded from a WAV file.
// Samples are stored as int32 per channel (normalized from file bit depth).
class Sample {
public:
    // Load from WAV file via libsndfile
    static Sample load(const std::string& path);

    // Save as WAV file
    void save(const std::string& path) const;

    // Sub-region [from, to) frame range — returns a new Sample
    Sample sub_region(int from, int to) const;

    // Return per-channel sample arrays (as int — same as Java int[][] channels)
    // Values are in range of the original bit depth (e.g. -32768..32767 for 16-bit)
    std::vector<std::vector<int>> as_samples() const;

    int frame_length() const { return frame_count_; }
    const AudioFormat& format() const { return format_; }
    bool is_valid() const { return !interleaved_.empty(); }

    // For audio playback: raw interleaved float samples
    const std::vector<float>& interleaved_float() const { return interleaved_; }

private:
    Sample() = default;

    AudioFormat format_{};
    int frame_count_ = 0;
    std::vector<float> interleaved_;  // float [-1, 1], channels interleaved
};
