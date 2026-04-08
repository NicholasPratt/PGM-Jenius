#include "ByteBuffer.h"
#include <fstream>
#include <stdexcept>
#include <cstring>

ByteBuffer::ByteBuffer(int length) : bytes_(length, 0) {}

ByteBuffer::ByteBuffer(std::vector<uint8_t> bytes) : bytes_(std::move(bytes)) {}

ByteBuffer::ByteBuffer(const ByteBuffer& other) : bytes_(other.bytes_) {}

std::string ByteBuffer::getString(int offset) const {
    std::string result;
    for (int i = 0; i < 16; i++) {
        char ch = static_cast<char>(bytes_[offset + i]);
        if (ch == 0) break;
        result += ch;
    }
    return result;
}

void ByteBuffer::setString(int offset, const std::string& s) {
    if (static_cast<int>(s.size()) > 16)
        throw std::invalid_argument("String too long (16 chars max): " + s);
    for (int i = 0; i < 16; i++)
        bytes_[offset + i] = 0;
    for (int i = 0; i < static_cast<int>(s.size()); i++)
        bytes_[offset + i] = static_cast<uint8_t>(s[i]);
}

uint8_t ByteBuffer::getByte(int offset) const {
    return bytes_[offset];
}

void ByteBuffer::setByte(int offset, int value) {
    bytes_[offset] = static_cast<uint8_t>(value);
}

int16_t ByteBuffer::getShort(int offset) const {
    int low  = bytes_[offset]     & 0xFF;
    int high = bytes_[offset + 1] & 0xFF;
    return static_cast<int16_t>((high << 8) | low);
}

void ByteBuffer::setShort(int offset, int16_t v) {
    bytes_[offset]     = static_cast<uint8_t>(v);
    bytes_[offset + 1] = static_cast<uint8_t>(v >> 8);
}

int32_t ByteBuffer::getInt(int offset) const {
    return ((bytes_[offset + 3] & 0xFF) << 24)
         | ((bytes_[offset + 2] & 0xFF) << 16)
         | ((bytes_[offset + 1] & 0xFF) << 8)
         |  (bytes_[offset + 0] & 0xFF);
}

void ByteBuffer::setInt(int offset, int32_t v) {
    bytes_[offset + 0] = static_cast<uint8_t>(v);
    bytes_[offset + 1] = static_cast<uint8_t>(v >> 8);
    bytes_[offset + 2] = static_cast<uint8_t>(v >> 16);
    bytes_[offset + 3] = static_cast<uint8_t>(v >> 24);
}

Range ByteBuffer::getRange(int offset) const {
    return Range(
        static_cast<int>(static_cast<int8_t>(bytes_[offset])),
        static_cast<int>(static_cast<int8_t>(bytes_[offset + 1]))
    );
}

void ByteBuffer::setRange(int offset, const Range& r) {
    bytes_[offset]     = static_cast<uint8_t>(r.low);
    bytes_[offset + 1] = static_cast<uint8_t>(r.high);
}

ByteBuffer ByteBuffer::open(const std::string& path, int expected_length) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open file: " + path);
    std::vector<uint8_t> data(expected_length, 0);
    f.read(reinterpret_cast<char*>(data.data()), expected_length);
    if (!f && !f.eof())
        throw std::runtime_error("Failed to read file: " + path);
    return ByteBuffer(std::move(data));
}

void ByteBuffer::save(const std::string& path) const {
    std::ofstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot write file: " + path);
    f.write(reinterpret_cast<const char*>(bytes_.data()), bytes_.size());
}
