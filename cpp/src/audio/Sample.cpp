#include "Sample.h"
#include <sndfile.h>
#include <stdexcept>
#include <cstring>

Sample Sample::load(const std::string& path) {
    SF_INFO info{};
    SNDFILE* sf = sf_open(path.c_str(), SFM_READ, &info);
    if (!sf) throw std::runtime_error("Cannot open audio file: " + path);

    Sample s;
    s.format_.sample_rate = info.samplerate;
    s.format_.channels    = info.channels;
    s.format_.bit_depth   = 16;  // normalized to float internally
    s.frame_count_        = static_cast<int>(info.frames);

    int total_samples = s.frame_count_ * info.channels;
    s.interleaved_.resize(total_samples);
    sf_readf_float(sf, s.interleaved_.data(), info.frames);
    sf_close(sf);
    return s;
}

void Sample::save(const std::string& path) const {
    SF_INFO info{};
    info.samplerate = format_.sample_rate;
    info.channels   = format_.channels;
    info.frames     = frame_count_;
    info.format     = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    SNDFILE* sf = sf_open(path.c_str(), SFM_WRITE, &info);
    if (!sf) throw std::runtime_error("Cannot write audio file: " + path);
    sf_writef_float(sf, interleaved_.data(), frame_count_);
    sf_close(sf);
}

Sample Sample::sub_region(int from, int to) const {
    if (from < 0) from = 0;
    if (to > frame_count_) to = frame_count_;
    int ch = format_.channels;
    Sample s;
    s.format_      = format_;
    s.frame_count_ = to - from;
    s.interleaved_.assign(
        interleaved_.begin() + from * ch,
        interleaved_.begin() + to   * ch);
    return s;
}

// Returns int samples scaled to signed 16-bit range (-32768..32767)
std::vector<std::vector<int>> Sample::as_samples() const {
    int ch = format_.channels;
    std::vector<std::vector<int>> result(ch, std::vector<int>(frame_count_));
    for (int frame = 0; frame < frame_count_; frame++) {
        for (int c = 0; c < ch; c++) {
            float f = interleaved_[frame * ch + c];
            result[c][frame] = static_cast<int>(f * 32767.0f);
        }
    }
    return result;
}
