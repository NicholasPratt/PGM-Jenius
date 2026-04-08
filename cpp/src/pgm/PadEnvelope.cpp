#include "PadEnvelope.h"

const Parameter PadEnvelope::ATTACK = Parameter::integer("Attack", 0x66, 0, 100);
const Parameter PadEnvelope::DECAY  = Parameter::integer("Decay",  0x67, 0, 100);
const Parameter PadEnvelope::DECAY_MODE =
    Parameter::enum_type("Decay Mode", 0x68, {"End", "Start"});
const Parameter PadEnvelope::VELOCITY_TO_LEVEL =
    Parameter::integer("Velocity to Level", 0x6B, 0, 100);

PadEnvelope::PadEnvelope(BaseElement& parent_pad)
    : BaseElement(parent_pad, SECTION_OFFSET, 0, SIZE) {}
