#include "PadMixer.h"

const Parameter PadMixer::VOLUME = Parameter::integer("Volume", 0x8F, 0, 100);
const Parameter PadMixer::PAN    = Parameter::integer("Pan",    0x90, 0, 100);
const Parameter PadMixer::OUTPUT = Parameter::enum_type("Output", 0x91, {"Stereo", "1-2", "3-4"});
const Parameter PadMixer::FX_SEND = Parameter::enum_type("FX Send", 0x92, {"Off", "1", "2"});
const Parameter PadMixer::FX_SEND_LEVEL = Parameter::integer("FX Send Level", 0x93, 0, 100);

PadMixer::PadMixer(BaseElement& parent_pad)
    : BaseElement(parent_pad, SECTION_OFFSET, 0, SIZE) {}
