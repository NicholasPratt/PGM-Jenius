#include "Program.h"
#include <filesystem>
#include <stdexcept>

static constexpr int MIDI_PROGRAM_CHANGE_OFFSET = 0x29D8;
static constexpr int PAD_TO_MIDI_NOTE_OFFSET = 0x2918;
static constexpr int MIDI_TO_PAD_OFFSET = 0x2958;
static constexpr int UNASSIGNED_PAD = 64;
static constexpr int VERSION_STRING_OFFSET = 0x04;
static constexpr int HEADER_FLAGS_OFFSET = 0x14;

namespace {
ByteBuffer make_initial_buffer() {
#ifdef MPCMAID_SOURCE_DIR
    const std::filesystem::path jjos_template_path =
        std::filesystem::path(MPCMAID_SOURCE_DIR) / "cpp" / "resources" / "jjos_128xl_template.pgm";
    try {
        return ByteBuffer::open(jjos_template_path.string(), Program::FILE_LENGTH);
    } catch (const std::exception&) {
    }

    const std::filesystem::path template_path =
        std::filesystem::path(MPCMAID_SOURCE_DIR) / "PGM-Jenius" / "src" / "com" / "pgmjenius" / "gui" / "default.pgm";
    try {
        return ByteBuffer::open(template_path.string(), Program::FILE_LENGTH);
    } catch (const std::exception&) {
    }
#endif
    return ByteBuffer(Program::FILE_LENGTH);
}
}

const std::string Program::FILE_VERSION = "MPC1000 PGM 8.00";
const Parameter Program::MIDI_PROGRAM_CHANGE =
    Parameter::int_or_off("MIDI Program Change", MIDI_PROGRAM_CHANGE_OFFSET, 0, 128);

Program::Program()
    : buf_(make_initial_buffer())
{
    // Match the header format written by the user's JJOS MPC1000.
    buf_.setString(VERSION_STRING_OFFSET, FILE_VERSION);
    // Write file length as little-endian short at 0x00
    buf_.setShort(0x00, static_cast<int16_t>(FILE_LENGTH));
    // Machine-created JJOS files observed here use 00 00 08 00 at 0x14..0x17.
    buf_.setByte(HEADER_FLAGS_OFFSET + 0, 0x00);
    buf_.setByte(HEADER_FLAGS_OFFSET + 1, 0x00);
    buf_.setByte(HEADER_FLAGS_OFFSET + 2, 0x08);
    buf_.setByte(HEADER_FLAGS_OFFSET + 3, 0x00);

    // JJOS-targeted default MIDI note mapping:
    // pads A01-D16 map chromatically from MIDI note 36 (C1) to 99.
    for (int note = 0; note < 128; ++note)
        buf_.setByte(MIDI_TO_PAD_OFFSET + note, UNASSIGNED_PAD);

    for (int pad = 0; pad < 64; ++pad) {
        const int midi_note = 36 + pad;
        buf_.setByte(PAD_TO_MIDI_NOTE_OFFSET + pad, midi_note);
        buf_.setByte(MIDI_TO_PAD_OFFSET + midi_note, pad);
    }

    // Reset editable program content while preserving template bytes we do not
    // understand yet. This keeps JJOS-friendly structural defaults without
    // carrying sample assignments from the template program.
    set_midi_program_change(0);

    for (int pad_index = 0; pad_index < 64; ++pad_index) {
        Pad pad = get_pad(pad_index);

        pad.set(Pad::VOICE_OVERLAP, 0);
        pad.set(Pad::MUTE_GROUP, 0);

        for (int layer_index = 0; layer_index < Pad::LAYER_COUNT; ++layer_index) {
            Layer layer = pad.get_layer(layer_index);
            layer.set_sample_name("");
            layer.set_level(100);
            layer.set_range({0, 127});
            layer.set_tuning(0.0);
            layer.set_one_shot();
        }

        PadEnvelope envelope = pad.get_envelope();
        envelope.set(PadEnvelope::ATTACK, 0);
        envelope.set(PadEnvelope::DECAY, 0);
        envelope.set(PadEnvelope::DECAY_MODE, 0);
        envelope.set(PadEnvelope::VELOCITY_TO_LEVEL, 0);

        PadFilter1 filter1 = pad.get_filter1();
        filter1.set(PadFilter1::TYPE, 0);
        filter1.set(PadFilter1::CUTOFF, 100);
        filter1.set(PadFilter1::RESONANCE, 0);
        filter1.set(PadFilter1::VELOCITY_TO_FREQ, 0);
        filter1.set(PadFilter1::PRE_ATTENUATION, 0);

        PadFilter2 filter2 = pad.get_filter2();
        filter2.set(PadFilter2::TYPE, 0);
        filter2.set(PadFilter2::CUTOFF, 100);
        filter2.set(PadFilter2::RESONANCE, 0);
        filter2.set(PadFilter2::VELOCITY_TO_FREQ, 0);

        PadMixer mixer = pad.get_mixer();
        mixer.set(PadMixer::VOLUME, 100);
        mixer.set(PadMixer::PAN, 50);
        mixer.set(PadMixer::OUTPUT, 0);
        mixer.set(PadMixer::FX_SEND, 0);
        mixer.set(PadMixer::FX_SEND_LEVEL, 0);
    }
}

Program::Program(ByteBuffer buf) : buf_(std::move(buf)) {}

Program Program::open(const std::string& path) {
    try {
        ByteBuffer buf = ByteBuffer::open(path, FILE_LENGTH);
        return Program(std::move(buf));
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Not a valid MPC1000 pgm file: ") + e.what());
    }
}

void Program::save(const std::string& path) const {
    buf_.save(path);
}

Pad Program::get_pad(int pad_index) {
    return Pad(buf_, pad_index);
}

Slider Program::get_slider(int slider_index) {
    return Slider(buf_, slider_index);
}

uint8_t Program::get_midi_program_change() const {
    return buf_.getByte(MIDI_PROGRAM_CHANGE_OFFSET);
}

void Program::set_midi_program_change(int value) {
    if (value < 0 || value > 128)
        throw std::out_of_range("MIDI program change must be 0-128");
    buf_.setByte(MIDI_PROGRAM_CHANGE_OFFSET, value);
}
