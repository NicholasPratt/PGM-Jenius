#pragma once
#include "BaseElement.h"
#include "Parameter.h"

class Slider : public BaseElement {
public:
    static constexpr int SECTION_OFFSET = 0x29D9;
    static constexpr int SLIDER_SIZE    = 0x29E6 - SECTION_OFFSET;

    static const Parameter PAD;
    static const Parameter PARAMETER_ASSIGN;
    static const Parameter TUNE_RANGE;
    static const Parameter FILTER_RANGE;
    static const Parameter LAYER_RANGE;
    static const Parameter ATTACK_RANGE;
    static const Parameter DECAY_RANGE;

    Slider(ByteBuffer& buf, int slider_index);

    Range getRange(int offset) const { return BaseElement::getRange(offset); }
    void  setRange(int offset, const Range& range) { BaseElement::setRange(offset, range); }
};
