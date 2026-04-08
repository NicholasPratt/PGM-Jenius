#include "MidiSequenceBuilder.h"
#include <fstream>
#include <stdexcept>
#include <cmath>

MidiSequenceBuilder::MidiSequenceBuilder(int sample_rate, int bpm, int ppq)
    : sample_rate_(sample_rate), bpm_(bpm), ppq_(ppq) {}

// Convert audio frames to MIDI ticks
// ticks = frames * (bpm * ppq) / (60 * sample_rate)
int MidiSequenceBuilder::frames_to_ticks(int frames) const {
    double seconds = static_cast<double>(frames) / sample_rate_;
    double beats   = seconds * bpm_ / 60.0;
    return static_cast<int>(std::round(beats * ppq_));
}

void MidiSequenceBuilder::write_uint32_be(std::vector<uint8_t>& buf, uint32_t v) {
    buf.push_back((v >> 24) & 0xFF);
    buf.push_back((v >> 16) & 0xFF);
    buf.push_back((v >>  8) & 0xFF);
    buf.push_back((v >>  0) & 0xFF);
}

void MidiSequenceBuilder::write_uint16_be(std::vector<uint8_t>& buf, uint16_t v) {
    buf.push_back((v >> 8) & 0xFF);
    buf.push_back((v >> 0) & 0xFF);
}

void MidiSequenceBuilder::write_var_len(std::vector<uint8_t>& buf, uint32_t v) {
    // MIDI variable-length encoding
    uint8_t parts[4];
    int n = 0;
    do {
        parts[n++] = v & 0x7F;
        v >>= 7;
    } while (v > 0);
    for (int i = n - 1; i >= 0; i--) {
        buf.push_back(parts[i] | (i > 0 ? 0x80 : 0x00));
    }
}

void MidiSequenceBuilder::write_byte(std::vector<uint8_t>& buf, uint8_t b) {
    buf.push_back(b);
}

void MidiSequenceBuilder::patch_uint32_be(std::vector<uint8_t>& buf, size_t offset, uint32_t v) {
    buf[offset + 0] = (v >> 24) & 0xFF;
    buf[offset + 1] = (v >> 16) & 0xFF;
    buf[offset + 2] = (v >>  8) & 0xFF;
    buf[offset + 3] = (v >>  0) & 0xFF;
}

void MidiSequenceBuilder::build(const std::string& output_path,
                                 const std::vector<int>& frame_positions,
                                 int starting_note) const
{
    std::vector<uint8_t> track;

    // Tempo meta-event (at tick 0)
    uint32_t microseconds_per_beat = 60000000u / static_cast<uint32_t>(bpm_);
    write_var_len(track, 0);                         // delta time = 0
    write_byte(track, 0xFF);                         // meta event
    write_byte(track, 0x51);                         // set tempo
    write_byte(track, 0x03);                         // length = 3
    write_byte(track, (microseconds_per_beat >> 16) & 0xFF);
    write_byte(track, (microseconds_per_beat >>  8) & 0xFF);
    write_byte(track, (microseconds_per_beat >>  0) & 0xFF);

    int prev_tick = 0;
    int n = static_cast<int>(frame_positions.size());

    for (int i = 0; i < n; i++) {
        int tick     = frames_to_ticks(frame_positions[i]);
        int delta    = tick - prev_tick;
        int note     = starting_note + (i % 64);  // wrap at 64 pads
        int velocity = DEFAULT_VELOCITY;

        // Note On
        write_var_len(track, static_cast<uint32_t>(delta));
        write_byte(track, 0x99);                   // Note On, channel 10 (drums)
        write_byte(track, static_cast<uint8_t>(note));
        write_byte(track, static_cast<uint8_t>(velocity));

        // Note Off (after fixed duration)
        write_var_len(track, static_cast<uint32_t>(NOTE_DURATION_TICKS));
        write_byte(track, 0x89);                   // Note Off, channel 10
        write_byte(track, static_cast<uint8_t>(note));
        write_byte(track, 0x00);

        prev_tick = tick + NOTE_DURATION_TICKS;
    }

    // End of track
    write_var_len(track, 0);
    write_byte(track, 0xFF);
    write_byte(track, 0x2F);
    write_byte(track, 0x00);

    // Assemble full SMF
    std::vector<uint8_t> smf;

    // MThd header
    smf.push_back('M'); smf.push_back('T'); smf.push_back('h'); smf.push_back('d');
    write_uint32_be(smf, 6);        // header length always 6
    write_uint16_be(smf, 0);        // format 0 (single track)
    write_uint16_be(smf, 1);        // 1 track
    write_uint16_be(smf, static_cast<uint16_t>(ppq_));

    // MTrk chunk
    smf.push_back('M'); smf.push_back('T'); smf.push_back('r'); smf.push_back('k');
    write_uint32_be(smf, static_cast<uint32_t>(track.size()));
    smf.insert(smf.end(), track.begin(), track.end());

    // Write to file
    std::ofstream f(output_path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot write MIDI file: " + output_path);
    f.write(reinterpret_cast<const char*>(smf.data()), smf.size());
}
