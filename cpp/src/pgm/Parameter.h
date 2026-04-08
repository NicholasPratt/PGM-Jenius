#pragma once
#include <string>
#include <vector>
#include "Range.h"

enum class ParamType {
    TEXT,
    INT,
    OFF_INT,
    ENUM,
    RANGE,
    TUNING
};

struct Parameter {
    std::string label;
    int offset;
    ParamType type;
    Range value_range;
    std::vector<std::string> enum_values;

    // Factory methods
    static Parameter integer(const std::string& label, int offset, int min, int max);
    static Parameter int_or_off(const std::string& label, int offset, int min, int max);
    static Parameter tuning(const std::string& label, int offset, int min, int max);
    static Parameter range(const std::string& label, int offset, int min, int max);
    static Parameter string(const std::string& label, int offset);
    static Parameter enum_type(const std::string& label, int offset, std::vector<std::string> values);

    int get_offset() const { return offset; }
    const std::string& get_label() const { return label; }
    ParamType get_type() const { return type; }
    const Range& get_range() const { return value_range; }
    const std::vector<std::string>& get_enum_values() const { return enum_values; }
};
