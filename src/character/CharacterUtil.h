#pragma once
#include "CharacterType.h"
#include "common/lib.h"

// Chuyển: CharacterType -> asset key
inline std::string characterKey(CharacterType t) {
    switch (t) {
        case CharacterType::Knight: return "Knight/knight";
        case CharacterType::Archer: return "Archer/archer";
        case CharacterType::Lancer: return "Lancer/lancer";
        case CharacterType::Swordsman: return "Swordsman/swordsman";
        default: return "Knight/knight";
    }
}

// Chuyển: CharacterType -> string - tên class nhân vật
inline std::string getCharacterClassName(CharacterType type) {
    switch (type) {
        case CharacterType::Knight: return "Knight";
        case CharacterType::Archer: return "Archer";
        case CharacterType::Lancer: return "Lancer";
        case CharacterType::Swordsman: return "Swordsman";
        default: return "Unknown";
    }
}

// Chuyển: CharacterType -> string - tên nhân vật
inline std::string characterName(CharacterType t) {
    switch (t) {
        case CharacterType::Knight: return "Knight";
        case CharacterType::Archer: return "Archer";
        case CharacterType::Lancer: return "Lancer";
        case CharacterType::Swordsman: return "Swordsman";
        default: return "Knight";
    }
}

// Chuyển: tên quái -> asset key
inline std::string enemyKeyFromName(const std::string& name) {
    static const std::unordered_map<std::string, std::string> kMap = {
        {"Skeleton Archer","Monster/Skeleton Archer_Level_1/skeletonArcher"},
        {"Bat","Monster/Bat_Level_1/bat"},
        {"Skeleton","Monster/Skeleton_Level_1/skeleton"},

        {"Werewolf","Monster/Werewolf_Level_2/werewolf"},
        {"Slime","Monster/Slime_Level_2/slime"},
        {"Armored Skeleton","Monster/Armored Skeleton_Level_2/armoredSkeleton"},
        {"Orc","Monster/Orc_Level_2/orc"},

        {"Elite Orc","Monster/Elite Orc_Level_3/eliteOrc"},
        {"Orc rider","Monster/Orc rider_Level_3/orcRider"},
        {"Greatsword Skeleton","Monster/Greatsword Skeleton_Level_3/greatswordSkeleton"},
        {"Necromancer","Monster/Necromancer_Level_3/necromancer"},
        {"Werebear","Monster/Werebear_Level_3/werebear"},

        {"Demon","Boss/Demon/demon"},
        {"Goblin Giant","Boss/Goblin Giant/goblinGiant"},
        {"Bringer Of Death","Boss/Bringer Of Death/bringerOfDeath"}
    };
    auto it = kMap.find(name); // Tìm
    if (it != kMap.end()) return it->second; // Nếu thấy trả về asset key
    // Nếu không -> in cảnh báo -> chuyển về mặc định Slime
    std::cerr << "[WARNING] Can't find the asset key for the enemy: \"" << name << "\" - using Slime as a stand-in for now." << std::endl;
    return "Monster/Slime_Level_2/slime";
}
