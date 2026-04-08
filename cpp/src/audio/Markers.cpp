#include "Markers.h"
#include "Slicer.h"
#include <algorithm>
#include <cmath>
#include <limits>

void Markers::clear(int frame_length, int frame_rate) {
    positions_.clear();
    frame_length_ = frame_length;
    frame_rate_   = frame_rate;
    selected_index_ = 0;
}

void Markers::add(int frame_position) {
    positions_.push_back(frame_position);
    std::sort(positions_.begin(), positions_.end());
}

LocationRange Markers::get_range_from(int index) const {
    int from = positions_[index];
    int to   = (index + 1 < static_cast<int>(positions_.size()))
                   ? positions_[index + 1]
                   : frame_length_;
    return {from, to};
}

void Markers::clamp_selected() {
    if (positions_.empty()) { selected_index_ = 0; return; }
    int n = static_cast<int>(positions_.size());
    if (selected_index_ < 0)    selected_index_ = 0;
    if (selected_index_ >= n)   selected_index_ = n - 1;
}

void Markers::set_selected_index(int i) {
    selected_index_ = i;
    clamp_selected();
}

int Markers::get_selected_location() const {
    if (positions_.empty()) return 0;
    return positions_[selected_index_];
}

void Markers::select_marker(int shift) {
    selected_index_ += shift;
    clamp_selected();
}

void Markers::select_closest(int frame_position) {
    if (positions_.empty()) return;
    int best_idx  = 0;
    int best_dist = std::numeric_limits<int>::max();
    for (int i = 0; i < static_cast<int>(positions_.size()); i++) {
        int dist = std::abs(positions_[i] - frame_position);
        if (dist < best_dist) {
            best_dist = dist;
            best_idx  = i;
        }
    }
    selected_index_ = best_idx;
}

void Markers::nudge(int frames, const Slicer& slicer) {
    if (positions_.empty()) return;
    int pos     = positions_[selected_index_] + frames;
    pos         = std::max(0, std::min(pos, frame_length_ - 1));
    int excursion = std::abs(frames) + 512;
    pos         = slicer.adjust_zero_crossing(pos, excursion);
    positions_[selected_index_] = pos;
    std::sort(positions_.begin(), positions_.end());
    // re-find selected after sort
    selected_index_ = static_cast<int>(
        std::find(positions_.begin(), positions_.end(), pos) - positions_.begin());
}

void Markers::delete_selected() {
    if (positions_.empty()) return;
    positions_.erase(positions_.begin() + selected_index_);
    clamp_selected();
}

void Markers::insert_marker() {
    // Insert midway between selected marker and next (or at frame 0 if empty)
    if (positions_.empty()) {
        add(0);
        return;
    }
    LocationRange r = get_range_from(selected_index_);
    int mid = (r.from + r.to) / 2;
    add(mid);
    // select the newly inserted marker
    for (int i = 0; i < static_cast<int>(positions_.size()); i++) {
        if (positions_[i] == mid) { selected_index_ = i; break; }
    }
}

double Markers::get_duration() const {
    if (frame_rate_ == 0) return 0.0;
    return static_cast<double>(frame_length_) / frame_rate_;
}

float Markers::get_tempo(int beats) const {
    int n = std::min(beats + 1, static_cast<int>(positions_.size()));
    if (n < 2 || frame_rate_ == 0) return 0.0f;
    int span = positions_[n - 1] - positions_[0];
    if (span == 0) return 0.0f;
    double seconds  = static_cast<double>(span) / frame_rate_;
    double bpm      = (n - 1) / seconds * 60.0;
    return static_cast<float>(bpm);
}
