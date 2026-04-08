#pragma once
#include "ByteBuffer.h"
#include "Parameter.h"
#include <cstdint>
#include <string>

// BaseElement wraps a ByteBuffer with a base offset for a particular
// section/element within the buffer. All get/set calls are relative
// to that base offset.
class BaseElement {
public:
    // Root element (owns/shares the buffer directly)
    explicit BaseElement(ByteBuffer& buf, int section_offset = 0, int element_index = 0, int element_size = 0);

    // Child element references parent's buffer
    BaseElement(BaseElement& parent, int section_offset, int element_index, int element_size);

    int get_element_index() const { return element_index_; }

    // Buffer accessors (relative to this element's base offset)
    uint8_t     getByte(int rel_offset) const;
    void        setByte(int rel_offset, int value);
    int16_t     getShort(int rel_offset) const;
    void        setShort(int rel_offset, int16_t value);
    std::string getString(int rel_offset) const;
    void        setString(int rel_offset, const std::string& s);
    Range       getRange(int rel_offset) const;
    void        setRange(int rel_offset, const Range& r);

    // Access parent buffer directly (for cross-element parameters like MIDI note map)
    ByteBuffer& get_parent_buffer() { return buf_; }
    const ByteBuffer& get_parent_buffer() const { return buf_; }
    ByteBuffer& get_buffer() { return buf_; }

    // Generic parameter get/set (byte-level by default)
    int  get(const Parameter& p) const;
    void set(const Parameter& p, int value);

    // Copy all parameters from source, skipping those in ignore list
    void copy_from(const BaseElement& src);

protected:
    int base_offset() const { return section_offset_ + element_index_ * element_size_; }

    ByteBuffer& buf_;
    int section_offset_;
    int element_index_;
    int element_size_;
};
