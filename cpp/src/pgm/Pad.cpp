#include "Pad.h"
#include <stdexcept>
#include <vector>

static constexpr int MIDI_NOTE_TABLE_OFFSET = 0x2918;
static constexpr int MIDI_TO_PAD_TABLE_OFFSET = 0x2958;
static constexpr int UNASSIGNED_PAD = 64;

static std::vector<std::string> note_names() {
    static const std::vector<std::string> names = {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    std::vector<std::string> notes(128);
    for (int midi = 0; midi < 128; ++midi) {
        const int chromatic = midi % 12;
        const int octave = (midi / 12) - 2;
        notes[midi] = "(" + std::to_string(midi) + ") - " + names[chromatic] + std::to_string(octave);
    }
    return notes;
}

const Parameter Pad::MIDI_NOTE = Parameter::enum_type("Note", MIDI_NOTE_TABLE_OFFSET, note_names());
const Parameter Pad::VOICE_OVERLAP = Parameter::enum_type("Voice Overlap", 0x62, {"Poly", "Mono"});
const Parameter Pad::MUTE_GROUP    = Parameter::int_or_off("Mute Group",   0x63, 0, 32);

Pad::Pad(ByteBuffer& buf, int pad_index)
    : BaseElement(buf, PAD_SECTION, pad_index, PAD_SIZE)
{
    if (pad_index < 0 || pad_index > 63)
        throw std::out_of_range("Pad index must be 0-63");
}

Layer Pad::get_layer(int layer_index) {
    return Layer(*this, layer_index);
}

PadEnvelope Pad::get_envelope() {
    return PadEnvelope(*this);
}

PadFilter1 Pad::get_filter1() {
    return PadFilter1(*this);
}

PadFilter2 Pad::get_filter2() {
    return PadFilter2(*this);
}

PadMixer Pad::get_mixer() {
    return PadMixer(*this);
}

uint8_t Pad::get_voice_overlap() const {
    return getByte(VOICE_OVERLAP.get_offset());
}

uint8_t Pad::get_mute_group() const {
    return getByte(MUTE_GROUP.get_offset());
}

void Pad::set_mute_group(int value) {
    setByte(MUTE_GROUP.get_offset(), value);
}

uint8_t Pad::get_pad_midi_note(const ByteBuffer& program_buf) const {
    return program_buf.getByte(MIDI_NOTE_TABLE_OFFSET + pad_index());
}

void Pad::set_pad_midi_note(ByteBuffer& program_buf, int midi_note) {
    if (midi_note < 0 || midi_note > 127)
        throw std::out_of_range("MIDI note must be 0-127");

    program_buf.setByte(MIDI_NOTE_TABLE_OFFSET + pad_index(), midi_note);

    // Keep the inverse MIDI-note lookup table synchronized with the pad table.
    for (int note = 0; note < 128; ++note)
        program_buf.setByte(MIDI_TO_PAD_TABLE_OFFSET + note, UNASSIGNED_PAD);

    for (int pad = 0; pad < 64; ++pad) {
        const int assigned_note = program_buf.getByte(MIDI_NOTE_TABLE_OFFSET + pad);
        if (assigned_note >= 0 && assigned_note <= 127)
            program_buf.setByte(MIDI_TO_PAD_TABLE_OFFSET + assigned_note, pad);
    }
}

int Pad::get(const Parameter& p) const {
    if (p.get_label() == MIDI_NOTE.get_label() && p.get_offset() == MIDI_NOTE.get_offset())
        return get_pad_midi_note(get_parent_buffer());
    return BaseElement::get(p);
}

void Pad::set(const Parameter& p, int value) {
    if (p.get_label() == MIDI_NOTE.get_label() && p.get_offset() == MIDI_NOTE.get_offset()) {
        set_pad_midi_note(get_parent_buffer(), value);
        return;
    }
    BaseElement::set(p, value);
}

void Pad::copy_pad_from(Pad& src) {
    for (int i = 0; i < PAD_SIZE; i++)
        setByte(i, src.getByte(i));
}
