#pragma once
#include "BaseElement.h"
#include "Parameter.h"

class PadMixer : public BaseElement {
public:
    static constexpr int SECTION_OFFSET = 0x00;
    static constexpr int SIZE           = 0x00;

    static const Parameter VOLUME;
    static const Parameter PAN;
    static const Parameter OUTPUT;
    static const Parameter FX_SEND;
    static const Parameter FX_SEND_LEVEL;

    explicit PadMixer(BaseElement& parent_pad);
};
