#pragma once
#include "BaseElement.h"
#include "Parameter.h"

// Filter 1 — low pass filter parameters
class PadFilter1 : public BaseElement {
public:
    static constexpr int SECTION_OFFSET = 0x00;
    static constexpr int SIZE           = 0x00;

    static const Parameter TYPE;
    static const Parameter CUTOFF;
    static const Parameter RESONANCE;
    static const Parameter VELOCITY_TO_FREQ;
    static const Parameter PRE_ATTENUATION;

    explicit PadFilter1(BaseElement& parent_pad);
};

// Filter 2 — high pass / band pass filter parameters
class PadFilter2 : public BaseElement {
public:
    static constexpr int SECTION_OFFSET = 0x00;
    static constexpr int SIZE           = 0x00;

    static const Parameter TYPE;
    static const Parameter CUTOFF;
    static const Parameter RESONANCE;
    static const Parameter VELOCITY_TO_FREQ;

    explicit PadFilter2(BaseElement& parent_pad);
};
