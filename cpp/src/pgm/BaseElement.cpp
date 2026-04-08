#include "BaseElement.h"

BaseElement::BaseElement(ByteBuffer& buf, int section_offset, int element_index, int element_size)
    : buf_(buf), section_offset_(section_offset), element_index_(element_index), element_size_(element_size) {}

BaseElement::BaseElement(BaseElement& parent, int section_offset, int element_index, int element_size)
    : buf_(parent.buf_),
      section_offset_(parent.base_offset() + section_offset),
      element_index_(element_index),
      element_size_(element_size) {}

uint8_t BaseElement::getByte(int rel_offset) const {
    return buf_.getByte(base_offset() + rel_offset);
}

void BaseElement::setByte(int rel_offset, int value) {
    buf_.setByte(base_offset() + rel_offset, value);
}

int16_t BaseElement::getShort(int rel_offset) const {
    return buf_.getShort(base_offset() + rel_offset);
}

void BaseElement::setShort(int rel_offset, int16_t value) {
    buf_.setShort(base_offset() + rel_offset, value);
}

std::string BaseElement::getString(int rel_offset) const {
    return buf_.getString(base_offset() + rel_offset);
}

void BaseElement::setString(int rel_offset, const std::string& s) {
    buf_.setString(base_offset() + rel_offset, s);
}

Range BaseElement::getRange(int rel_offset) const {
    return buf_.getRange(base_offset() + rel_offset);
}

void BaseElement::setRange(int rel_offset, const Range& r) {
    buf_.setRange(base_offset() + rel_offset, r);
}

int BaseElement::get(const Parameter& p) const {
    return getByte(p.get_offset());
}

void BaseElement::set(const Parameter& p, int value) {
    setByte(p.get_offset(), value);
}

void BaseElement::copy_from(const BaseElement& src) {
    int size = element_size_ > 0 ? element_size_ : 0;
    for (int i = 0; i < size; i++) {
        setByte(i, src.getByte(i));
    }
}
