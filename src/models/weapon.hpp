#pragma once

#include <string>
#include <string_view>
#include <stdexcept>
#include <nlohmann/json.hpp>

enum class Weapon {
    Foil,
    Epee,
    Sabre
};

inline std::string weaponToString(Weapon weapon) {
    switch (weapon) {
        case Weapon::Foil: return "Foil";
        case Weapon::Epee: return "Epee";
        case Weapon::Sabre: return "Sabre";
        default: return "Foil";
    }
}

inline Weapon stringToWeapon(std::string_view str) {
    if (str == "Foil" || str == "foil") return Weapon::Foil;
    if (str == "Epee" || str == "epee") return Weapon::Epee;
    if (str == "Sabre" || str == "sabre") return Weapon::Sabre;
    return Weapon::Foil;
}

inline void to_json(nlohmann::json& j, const Weapon& weapon) {
    j = weaponToString(weapon);
}

inline void from_json(const nlohmann::json& j, Weapon& weapon) {
    if (j.is_string()) {
        weapon = stringToWeapon(j.get<std::string>());
    } else if (j.is_number_integer()) {
        weapon = static_cast<Weapon>(j.get<int>());
    } else {
        weapon = Weapon::Foil;
    }
}
