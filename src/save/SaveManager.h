#pragma once
#include "character/CharacterType.h"
#include "world/Map.h"
#include "common/lib.h"

struct SaveData {
    bool isValid = false;
    CharacterType characterType = CharacterType::Knight;
    int hp = 100;
    int maxHP = 100;
    int level = 1;
    int exp = 0;
    int gold = 0;
    int potions = 3;
    int inventorySkills = 0;
    float posX = 2000.f;
    float posY = 2000.f;
    bool bossGoblinDead = false;
    bool bossDemonDead = false;
    bool bossBringerDead = false;
    int hpScrollCount = 0;
    int damageScrollCount = 0;
    int defenseScrollCount = 0;
};

class SaveManager {
public:
    static constexpr int MAX_SLOTS = 6;

    static bool saveGame(int slot, const SaveData& data);
    static SaveData loadGame(int slot);
    static std::array<SaveData, MAX_SLOTS> loadAllSlots();
    static bool deleteSlot(int slot);
    static std::string getSlotFilePath(int slot);
};
