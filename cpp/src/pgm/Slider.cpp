#include "Slider.h"
#include <stdexcept>

const Parameter Slider::PAD = Parameter::int_or_off("Pad", 0x00, 0, 64);
const Parameter Slider::PARAMETER_ASSIGN =
    Parameter::enum_type("Parameter", 0x02, {"Tune", "Filter", "Layer", "Attack", "Decay"});
const Parameter Slider::TUNE_RANGE   = Parameter::range("Tune",   0x03, -120, 120);
const Parameter Slider::FILTER_RANGE = Parameter::range("Filter", 0x05, -50, 50);
const Parameter Slider::LAYER_RANGE  = Parameter::range("Layer",  0x07, 0, 127);
const Parameter Slider::ATTACK_RANGE = Parameter::range("Attack", 0x09, 0, 100);
const Parameter Slider::DECAY_RANGE  = Parameter::range("Decay",  0x0B, 0, 100);

Slider::Slider(ByteBuffer& buf, int slider_index)
    : BaseElement(buf, SECTION_OFFSET, slider_index, SLIDER_SIZE)
{
    if (slider_index < 0 || slider_index > 1)
        throw std::out_of_range("Slider index must be 0 or 1");
}
