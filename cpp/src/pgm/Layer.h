#pragma once
#include "BaseElement.h"
#include "Parameter.h"
#include <string>

class Pad;

class Layer : public BaseElement {
public:
    static constexpr int LAYER_SIZE = 0x18;

    static const Parameter SAMPLE_NAME;
    static const Parameter LEVEL;
    static const Parameter RANGE_PARAM;
    static const Parameter TUNING_PARAM;
    static const Parameter PLAY_MODE;

    Layer(BaseElement& parent_pad, int layer_index);

    std::string get_sample_name() const;
    void        set_sample_name(const std::string& name);

    double get_tuning() const;
    void   set_tuning(double tuning);

    uint8_t get_level() const;
    void    set_level(int value);

    Range get_range() const;
    void  set_range(const Range& r);

    bool is_one_shot() const;
    bool is_note_on() const;
    void set_one_shot();
    void set_note_on();
};
