#include "PadFilter.h"

const Parameter PadFilter1::TYPE =
    Parameter::enum_type("Type", 0x71, {"Off", "Lowpass", "Bandpass", "Highpass"});
const Parameter PadFilter1::CUTOFF    = Parameter::integer("Frequency", 0x72, 0, 100);
const Parameter PadFilter1::RESONANCE = Parameter::integer("Resonance", 0x73, 0, 100);
const Parameter PadFilter1::VELOCITY_TO_FREQ =
    Parameter::integer("Velocity to Freq.", 0x78, 0, 100);
const Parameter PadFilter1::PRE_ATTENUATION =
    Parameter::enum_type("Pre-attenuation", 0x94, {"0dB", "-6dB", "-12dB"});

PadFilter1::PadFilter1(BaseElement& parent_pad)
    : BaseElement(parent_pad, SECTION_OFFSET, 0, SIZE) {}

const Parameter PadFilter2::TYPE =
    Parameter::enum_type("Type", 0x79, {"Off", "Lowpass", "Bandpass", "Highpass", "Link"});
const Parameter PadFilter2::CUTOFF    = Parameter::integer("Frequency", 0x7A, 0, 100);
const Parameter PadFilter2::RESONANCE = Parameter::integer("Resonance", 0x7B, 0, 100);
const Parameter PadFilter2::VELOCITY_TO_FREQ =
    Parameter::integer("Velocity to Freq.", 0x80, 0, 100);

PadFilter2::PadFilter2(BaseElement& parent_pad)
    : BaseElement(parent_pad, SECTION_OFFSET, 0, SIZE) {}
