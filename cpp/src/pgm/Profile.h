#pragma once
#include <string>

struct Profile {
    std::string name;
    int row_number;
    int col_number;
    int slider_number;
    int filter_number;

    int pad_number() const { return row_number * col_number; }

    static const Profile MPC1000;

    static const Profile& get(const std::string& /*name*/) {
        return MPC1000;
    }
};

inline const Profile Profile::MPC1000 = {"MPC1000", 4, 4, 2, 2};
