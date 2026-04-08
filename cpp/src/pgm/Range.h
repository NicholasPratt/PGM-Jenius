#pragma once

struct Range {
    int low;
    int high;

    Range() : low(0), high(0) {}
    Range(int low, int high) : low(low), high(high) {}

    bool contains(double v) const { return v >= low && v <= high; }
};
