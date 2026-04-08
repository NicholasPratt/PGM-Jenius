#include "Layer.h"

const Parameter Layer::SAMPLE_NAME = Parameter::string("Sample",    0x00);
const Parameter Layer::LEVEL       = Parameter::integer("Level",    0x11, 0, 100);
const Parameter Layer::RANGE_PARAM = Parameter::range("Range",      0x12, 0, 127);
const Parameter Layer::TUNING_PARAM= Parameter::tuning("Tuning",   0x14, -36, 36);
const Parameter Layer::PLAY_MODE   = Parameter::enum_type("Play Mode", 0x16, {"One Shot", "Note On"});

Layer::Layer(BaseElement& parent_pad, int layer_index)
    : BaseElement(parent_pad, 0, layer_index, LAYER_SIZE) {}

std::string Layer::get_sample_name() const {
    return getString(SAMPLE_NAME.get_offset());
}

void Layer::set_sample_name(const std::string& name) {
    setString(SAMPLE_NAME.get_offset(), name);
}

double Layer::get_tuning() const {
    return getShort(TUNING_PARAM.get_offset()) / 100.0;
}

void Layer::set_tuning(double tuning) {
    setShort(TUNING_PARAM.get_offset(), static_cast<int16_t>(tuning * 100.0));
}

uint8_t Layer::get_level() const {
    return getByte(LEVEL.get_offset());
}

void Layer::set_level(int value) {
    setByte(LEVEL.get_offset(), value);
}

Range Layer::get_range() const {
    return getRange(RANGE_PARAM.get_offset());
}

void Layer::set_range(const Range& r) {
    setRange(RANGE_PARAM.get_offset(), r);
}

bool Layer::is_one_shot() const {
    return getByte(PLAY_MODE.get_offset()) == 0;
}

bool Layer::is_note_on() const {
    return !is_one_shot();
}

void Layer::set_one_shot() {
    setByte(PLAY_MODE.get_offset(), 0);
}

void Layer::set_note_on() {
    setByte(PLAY_MODE.get_offset(), 1);
}
