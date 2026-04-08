#pragma once
#include "ByteBuffer.h"
#include "Pad.h"
#include "Slider.h"
#include "Parameter.h"
#include <string>
#include <memory>

class Program {
public:
    static constexpr int FILE_LENGTH = 0x2A04;
    static const std::string FILE_VERSION;

    static const Parameter MIDI_PROGRAM_CHANGE;

    // Construct empty program
    Program();

    // Load from file
    static Program open(const std::string& path);

    // Save to file
    void save(const std::string& path) const;

    // Access pads (0-63)
    Pad get_pad(int pad_index);

    // Access sliders (0-1)
    Slider get_slider(int slider_index);

    uint8_t get_midi_program_change() const;
    void    set_midi_program_change(int value);

    ByteBuffer& buffer() { return buf_; }
    const ByteBuffer& buffer() const { return buf_; }

private:
    explicit Program(ByteBuffer buf);
    ByteBuffer buf_;
};
