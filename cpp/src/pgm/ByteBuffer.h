#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Range.h"

class ByteBuffer {
public:
    explicit ByteBuffer(int length);
    explicit ByteBuffer(std::vector<uint8_t> bytes);
    ByteBuffer(const ByteBuffer& other);

    // Accessors
    std::string getString(int offset) const;
    void        setString(int offset, const std::string& s);

    uint8_t  getByte(int offset) const;
    void     setByte(int offset, int value);

    int16_t  getShort(int offset) const;      // little-endian
    void     setShort(int offset, int16_t v);

    int32_t  getInt(int offset) const;        // little-endian
    void     setInt(int offset, int32_t v);

    Range    getRange(int offset) const;
    void     setRange(int offset, const Range& r);

    int size() const { return static_cast<int>(bytes_.size()); }
    const uint8_t* data() const { return bytes_.data(); }

    // File I/O
    static ByteBuffer open(const std::string& path, int expected_length);
    void save(const std::string& path) const;

private:
    std::vector<uint8_t> bytes_;
};
