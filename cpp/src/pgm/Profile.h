#pragma once
#include <string>

struct Profile {
    std::string name;
    int row_number;
    int col_number;
    int slider_number;
    int filter_number;

    int pad_number() const { return row_number * col_number; }

    static const Profile MPC500;
    static const Profile MPC1000;

    static const Profile& get(const std::string& name) {
        if (name == "MPC1000") return MPC1000;
        return MPC500;
    }
};

inline const Profile Profile::MPC500  = {"MPC500",  4, 3, 1, 1};
inline const Profile Profile::MPC1000 = {"MPC1000", 4, 4, 2, 2};
