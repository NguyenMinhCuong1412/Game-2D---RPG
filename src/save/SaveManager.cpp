#include "save/SaveManager.h"

std::string SaveManager::getSlotFilePath(int slot) { return "saves/slot_" + std::to_string(slot) + ".sav"; }

bool SaveManager::saveGame(int slot, const SaveData& data) {
    if (slot < 0 || slot >= MAX_SLOTS) return false;

    std::filesystem::create_directories("saves");

    std::ofstream file(getSlotFilePath(slot), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open save file for slot " << slot << std::endl;
        return false;
    }

    const char magic[] = "G2D3";
    file.write(magic, 4);

    int charType = static_cast<int>(data.characterType);
    file.write(reinterpret_cast<const char*>(&charType), sizeof(int));
    file.write(reinterpret_cast<const char*>(&data.hp), sizeof(int));
    file.write(reinterpret_cast<const char*>(&data.maxHP), sizeof(int));
    file.write(reinterpret_cast<const char*>(&data.level), sizeof(int));
    file.write(reinterpret_cast<const char*>(&data.exp), sizeof(int));
    file.write(reinterpret_cast<const char*>(&data.gold), sizeof(int));
    file.write(reinterpret_cast<const char*>(&data.potions), sizeof(int));
    file.write(reinterpret_cast<const char*>(&data.inventorySkills), sizeof(int));
    file.write(reinterpret_cast<const char*>(&data.posX), sizeof(float));
    file.write(reinterpret_cast<const char*>(&data.posY), sizeof(float));
    file.write(reinterpret_cast<const char*>(&data.bossGoblinDead), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&data.bossDemonDead), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&data.bossBringerDead), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&data.hpScrollCount), sizeof(int));
    file.write(reinterpret_cast<const char*>(&data.damageScrollCount), sizeof(int));
    file.write(reinterpret_cast<const char*>(&data.defenseScrollCount), sizeof(int));

    file.close();
    std::cout << "Game saved to slot " << slot << std::endl;
    return true;
}

SaveData SaveManager::loadGame(int slot) {
    SaveData data;
    if (slot < 0 || slot >= MAX_SLOTS) return data;

    std::ifstream file(getSlotFilePath(slot), std::ios::binary);
    if (!file.is_open()) return data;

    auto safeRead = [&](void* dest, std::size_t size) -> bool {
        file.read(reinterpret_cast<char*>(dest), size);
        return file.gcount() == static_cast<std::streamsize>(size) && !file.fail();
    };

    char magic[3];
    char version;
    if (!safeRead(magic, 3)) return data;
    if (magic[0] != 'G' || magic[1] != '2' || magic[2] != 'D') return data;

    if (!safeRead(&version, 1)) return data;
    if (version != '3') return data;


    int charType;
    if (!safeRead(&charType, sizeof(int))) return data;
    data.characterType = static_cast<CharacterType>(charType);

    if (!safeRead(&data.hp, sizeof(int))) return data;
    if (!safeRead(&data.maxHP, sizeof(int))) return data;
    if (!safeRead(&data.level, sizeof(int))) return data;
    if (!safeRead(&data.exp, sizeof(int))) return data;
    if (!safeRead(&data.gold, sizeof(int))) return data;
    if (!safeRead(&data.potions, sizeof(int))) return data;
    if (!safeRead(&data.inventorySkills, sizeof(int))) return data;
    if (!safeRead(&data.posX, sizeof(float))) return data;
    if (!safeRead(&data.posY, sizeof(float))) return data;
    if (!safeRead(&data.bossGoblinDead, sizeof(bool))) return data;
    if (!safeRead(&data.bossDemonDead, sizeof(bool))) return data;
    if (!safeRead(&data.bossBringerDead, sizeof(bool))) return data;
    if (!safeRead(&data.hpScrollCount, sizeof(int))) return data;
    if (!safeRead(&data.damageScrollCount, sizeof(int))) return data;
    if (!safeRead(&data.defenseScrollCount, sizeof(int))) return data;

    bool isCharTypeValid = (charType >= static_cast<int>(CharacterType::Knight) && charType <= static_cast<int>(CharacterType::Swordsman));
    if (!isCharTypeValid) return data;
    if (data.hp < 0) return data;
    if (data.maxHP <= 0) return data;
    if (data.hp > data.maxHP) return data;
    if (data.level < 1) return data;
    if (data.exp < 0) return data;
    if (data.gold < 0) return data;
    if (data.potions < 0) return data;
    if (data.inventorySkills < 0) return data;
    if (!std::isfinite(data.posX) || !std::isfinite(data.posY)) return data;
    if (data.hpScrollCount < 0 || data.damageScrollCount < 0 || data.defenseScrollCount < 0) return data;

    data.isValid = true;
    file.close();
    return data;
}

std::array<SaveData, SaveManager::MAX_SLOTS> SaveManager::loadAllSlots() {
    std::array<SaveData, MAX_SLOTS> slots;
    for (int i = 0; i < MAX_SLOTS; i++) slots[i] = loadGame(i);
    return slots;
}

bool SaveManager::deleteSlot(int slot) {
    if (slot < 0 || slot >= MAX_SLOTS) return false;
    std::string path = getSlotFilePath(slot);
    if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
        std::cout << "Deleted save slot " << slot << std::endl;
        return true;
    }
    return false;
}
