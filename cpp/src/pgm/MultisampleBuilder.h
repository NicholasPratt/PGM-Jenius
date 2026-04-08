#pragma once
#include "Program.h"
#include <string>
#include <vector>

struct SampleEntry {
    std::string filename;   // basename without extension
    std::string note_name;  // e.g. "C#3", "E5"
    int midi_note;          // 0-127
};

class MultisampleBuilder {
public:
    // Scan a directory for WAV files with note names in their filename.
    // Returns entries sorted by MIDI note.
    static std::vector<SampleEntry> scan(const std::string& dir_path);

    // Assign scanned samples to pads in program, distributing across layers
    // with pitch transposition to cover gaps.
    static void apply(Program& program, const std::vector<SampleEntry>& entries);

private:
    static int parse_midi_note(const std::string& filename);
    static std::string note_name_from_midi(int midi_note);
};
