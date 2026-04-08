#pragma once
#include <string>
#include <vector>

struct SliceMarker {
    int frame_position;
    int midi_note;  // 0-127; default: assign pad 0=36, 1=37, ...
};

// Builds a Standard MIDI File (SMF format 0) from a list of slice markers.
// Each marker becomes a note-on/note-off event at the corresponding beat.
class MidiSequenceBuilder {
public:
    static constexpr int DEFAULT_BPM      = 120;
    static constexpr int DEFAULT_PPQ      = 480;   // pulses per quarter note
    static constexpr int DEFAULT_VELOCITY = 100;
    static constexpr int NOTE_DURATION_TICKS = 120; // 1/16 note at 480ppq

    MidiSequenceBuilder(int sample_rate = 44100, int bpm = DEFAULT_BPM, int ppq = DEFAULT_PPQ);

    // Build and write .mid file from beat markers
    // markers: frame positions of each beat
    // starting_note: MIDI note for first slice (subsequent slices increment)
    void build(const std::string& output_path,
               const std::vector<int>& frame_positions,
               int starting_note = 36) const;

private:
    int sample_rate_;
    int bpm_;
    int ppq_;

    int frames_to_ticks(int frames) const;

    // SMF helpers — all big-endian per MIDI spec
    static void write_uint32_be(std::vector<uint8_t>& buf, uint32_t v);
    static void write_uint16_be(std::vector<uint8_t>& buf, uint16_t v);
    static void write_var_len(std::vector<uint8_t>& buf, uint32_t v);
    static void write_byte(std::vector<uint8_t>& buf, uint8_t b);
    static void patch_uint32_be(std::vector<uint8_t>& buf, size_t offset, uint32_t v);
};
