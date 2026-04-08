#pragma once
#include "BaseElement.h"
#include "Layer.h"
#include "PadEnvelope.h"
#include "PadFilter.h"
#include "PadMixer.h"
#include "Parameter.h"
#include <array>

class Program;

class Pad : public BaseElement {
public:
    static constexpr int PAD_SECTION = 0x14 + 4;  // 0x18
    static constexpr int PAD_SIZE    = 0xA4;
    static constexpr int LAYER_COUNT = 4;

    static const Parameter MIDI_NOTE;
    static const Parameter VOICE_OVERLAP;
    static const Parameter MUTE_GROUP;

    Pad(ByteBuffer& buf, int pad_index);

    int pad_index() const { return get_element_index(); }

    Layer       get_layer(int layer_index);
    PadEnvelope get_envelope();
    PadFilter1  get_filter1();
    PadFilter2  get_filter2();
    PadMixer    get_mixer();

    uint8_t get_voice_overlap() const;
    uint8_t get_mute_group() const;
    void    set_mute_group(int value);

    // MIDI note for this pad is stored in the global MIDI note table at the
    // program level. Pad only exposes a convenience wrapper.
    uint8_t get_pad_midi_note(const ByteBuffer& program_buf) const;
    void    set_pad_midi_note(ByteBuffer& program_buf, int midi_note);

    int  get(const Parameter& p) const;
    void set(const Parameter& p, int value);

    void copy_pad_from(Pad& src);
};
