#pragma once
#include "BaseElement.h"
#include "Parameter.h"

class PadEnvelope : public BaseElement {
public:
    static constexpr int SECTION_OFFSET = 0x00;
    static constexpr int SIZE           = 0x00;

    static const Parameter ATTACK;
    static const Parameter DECAY;
    static const Parameter DECAY_MODE;
    static const Parameter VELOCITY_TO_LEVEL;

    explicit PadEnvelope(BaseElement& parent_pad);
};
