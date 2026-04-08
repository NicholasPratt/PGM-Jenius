#pragma once
#include <vector>

struct LocationRange {
    int from;
    int to;
};

class Slicer;  // forward declare for nudge

class Markers {
public:
    void clear(int frame_length, int frame_rate);
    void add(int frame_position);

    int size() const { return static_cast<int>(positions_.size()); }
    bool empty() const { return positions_.empty(); }
    int get(int index) const { return positions_[index]; }

    // Range from marker[index] to marker[index+1] (or end of file)
    LocationRange get_range_from(int index) const;

    // Selection
    int  get_selected_index() const { return selected_index_; }
    void set_selected_index(int i);
    int  get_selected_location() const;

    // Navigation
    void select_marker(int shift);                      // move selection by ±1
    void select_closest(int frame_position);            // select nearest marker
    void nudge(int frames, const Slicer& slicer);       // move selected marker (with zero-crossing)
    void delete_selected();
    void insert_marker();                               // insert new marker near selected

    double get_duration() const;     // seconds
    float  get_tempo(int beats) const; // estimated BPM from first N beats
    bool   has_beat() const { return size() > 1; }

    const std::vector<int>& positions() const { return positions_; }

private:
    void clamp_selected();

    std::vector<int> positions_;
    int frame_length_ = 0;
    int frame_rate_   = 44100;
    int selected_index_ = 0;
};
