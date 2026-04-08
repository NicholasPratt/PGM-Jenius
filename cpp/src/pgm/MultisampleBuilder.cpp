#include "MultisampleBuilder.h"
#include "PadMixer.h"
#include <algorithm>
#include <regex>
#include <filesystem>
#include <cmath>

namespace fs = std::filesystem;

static const std::vector<std::string> NOTE_NAMES = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

// Parse a MIDI note number from a filename containing a note like "C#3" or "A4"
int MultisampleBuilder::parse_midi_note(const std::string& filename) {
    // Match note name + optional sharp + octave digit
    std::regex re("([A-G]#?)(-?[0-9])");
    std::smatch m;
    std::string name = filename;
    if (!std::regex_search(name, m, re)) return -1;

    std::string note = m[1].str();
    int octave = std::stoi(m[2].str());

    auto it = std::find(NOTE_NAMES.begin(), NOTE_NAMES.end(), note);
    if (it == NOTE_NAMES.end()) return -1;

    int semitone = static_cast<int>(std::distance(NOTE_NAMES.begin(), it));
    return (octave + 1) * 12 + semitone;  // C0 = 12, C4 = 60
}

std::string MultisampleBuilder::note_name_from_midi(int midi_note) {
    int octave   = (midi_note / 12) - 1;
    int semitone = midi_note % 12;
    return NOTE_NAMES[semitone] + std::to_string(octave);
}

std::vector<SampleEntry> MultisampleBuilder::scan(const std::string& dir_path) {
    std::vector<SampleEntry> entries;
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        if (ext != ".wav" && ext != ".WAV") continue;

        std::string stem = entry.path().stem().string();
        int midi = parse_midi_note(stem);
        if (midi < 0) continue;

        entries.push_back({stem, note_name_from_midi(midi), midi});
    }
    std::sort(entries.begin(), entries.end(),
              [](const SampleEntry& a, const SampleEntry& b) { return a.midi_note < b.midi_note; });
    return entries;
}

void MultisampleBuilder::apply(Program& program, const std::vector<SampleEntry>& entries) {
    if (entries.empty()) return;

    // Assign one sample per pad (up to 64 pads).
    // When there are fewer samples than pads, distribute with tuning offsets.
    int n = static_cast<int>(entries.size());
    int pad_count = 64;

    for (int pad_idx = 0; pad_idx < pad_count && pad_idx < n; pad_idx++) {
        Pad pad = program.get_pad(pad_idx);
        Layer layer = pad.get_layer(0);
        layer.set_sample_name(entries[pad_idx].filename);
        layer.set_level(100);
        layer.set_range({0, 127});
        PadMixer mixer = pad.get_mixer();
        mixer.set(PadMixer::VOLUME, 100);
        mixer.set(PadMixer::PAN, 50);
        layer.set_tuning(0.0);
        layer.set_one_shot();
    }

    // For remaining pads beyond sample count, interpolate using nearest sample + tuning
    for (int pad_idx = n; pad_idx < pad_count; pad_idx++) {
        // Find closest sample
        const SampleEntry& nearest = entries[n - 1];
        Pad pad = program.get_pad(pad_idx);
        Layer layer = pad.get_layer(0);
        // Semitone offset from nearest sample
        int target_midi = entries[0].midi_note + pad_idx;
        double tuning = static_cast<double>(target_midi - nearest.midi_note);
        tuning = std::max(-36.0, std::min(36.0, tuning));
        layer.set_sample_name(nearest.filename);
        layer.set_level(100);
        layer.set_range({0, 127});
        PadMixer mixer = pad.get_mixer();
        mixer.set(PadMixer::VOLUME, 100);
        mixer.set(PadMixer::PAN, 50);
        layer.set_tuning(tuning);
        layer.set_one_shot();
    }
}
