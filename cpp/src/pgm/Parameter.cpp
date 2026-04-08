#include "Parameter.h"

Parameter Parameter::integer(const std::string& label, int offset, int min, int max) {
    Parameter p;
    p.label = label;
    p.offset = offset;
    p.type = ParamType::INT;
    p.value_range = Range(min, max);
    return p;
}

Parameter Parameter::int_or_off(const std::string& label, int offset, int min, int max) {
    Parameter p;
    p.label = label;
    p.offset = offset;
    p.type = ParamType::OFF_INT;
    p.value_range = Range(min, max);
    return p;
}

Parameter Parameter::tuning(const std::string& label, int offset, int min, int max) {
    Parameter p;
    p.label = label;
    p.offset = offset;
    p.type = ParamType::TUNING;
    p.value_range = Range(min, max);
    return p;
}

Parameter Parameter::range(const std::string& label, int offset, int min, int max) {
    Parameter p;
    p.label = label;
    p.offset = offset;
    p.type = ParamType::RANGE;
    p.value_range = Range(min, max);
    return p;
}

Parameter Parameter::string(const std::string& label, int offset) {
    Parameter p;
    p.label = label;
    p.offset = offset;
    p.type = ParamType::TEXT;
    return p;
}

Parameter Parameter::enum_type(const std::string& label, int offset, std::vector<std::string> values) {
    Parameter p;
    p.label = label;
    p.offset = offset;
    p.type = ParamType::ENUM;
    p.enum_values = std::move(values);
    return p;
}
