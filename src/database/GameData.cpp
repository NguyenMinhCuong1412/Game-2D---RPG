#include "database/GameData.h"

GameData::GameData() {
    loadCharacters();
    loadEnemies();
    loadItems();
    loadLootTables();
    loadSpawnList();
    loadAnimations();
    loadAssetPaths();
    loadBalanceConstants();

    if (std::filesystem::exists("assets/backgrounds/defeat.png") && mDefeatTexture.loadFromFile("assets/backgrounds/defeat.png"))
        mHasDefeatTexture = true;
}

const sf::Texture* GameData::getDefeatTexture() const { return mHasDefeatTexture ? &mDefeatTexture : nullptr; }

void GameData::loadCharacters() {
    mCharacters[CharacterType::Knight]    = { "Knight",     130, 220.f,  6.f,  8.f, 15.f,  13.f, 10.f, 5.0f, 45, 4.0f };
    mCharacters[CharacterType::Archer]    = { "Archer",      80, 280.f,  4.f,  7.f, 14.f,  10.f, 18.f, 8.0f, 25, 1.5f };
    mCharacters[CharacterType::Lancer]    = { "Lancer",     110, 250.f,  6.f,  9.f, 18.f,  14.f, 12.f, 6.0f, 35, 3.0f };
    mCharacters[CharacterType::Swordsman] = { "Swordsman",   95, 240.f,  5.f,  7.f, 15.f,  16.f, 12.f, 7.5f, 30, 2.5f };
    mDefaultCharacter = mCharacters[CharacterType::Knight];
}

void GameData::loadEnemies() {
    mEnemies["Skeleton Archer"]   = { "Skeleton Archer",   "Skeleton Archer",   40, 20, 6.f, 2.4f, 0.f, 0.f, 160.f, 8.f, 15, 8,  5, 4, 22.f, 350.f, 200.f, 600.f, 1.6f };
    mEnemies["Bat"]               = { "Bat",               "Bat",               30, 13, 5.f, 1.8f, 0.f, 0.f, 180.f, 10.f, 12, 6,  4, 3, 18.f, 250.f, 35.f, 600.f, 1.2f };
    mEnemies["Skeleton"]          = { "Skeleton",          "Skeleton",          50, 25, 8.f, 3.5f, 5.f, 1.0f, 140.f, 6.f, 18, 9,  6, 5, 22.f, 250.f, 35.f, 600.f, 1.5f };

    mEnemies["Werewolf"]          = { "Werewolf",          "Werewolf",          70, 33, 11.f, 4.2f, 0.f, 0.f, 190.f, 12.f, 30, 15, 12, 8, 24.f, 280.f, 40.f, 600.f, 1.3f };
    mEnemies["Slime"]             = { "Slime",             "Slime",             60, 30, 10.f, 4.0f, 10.f, 1.5f, 120.f, 5.f, 25, 12, 10, 6, 20.f, 260.f, 35.f, 600.f, 1.6f };
    mEnemies["Armored Skeleton"]  = { "Armored Skeleton",  "Armored Skeleton",  80, 39, 10.f, 3.6f, 0.f, 0.f, 130.f, 6.f, 35, 18, 15, 10, 24.f, 260.f, 35.f, 600.f, 1.7f };
    mEnemies["Orc"]               = { "Orc",               "Orc",               80, 40, 12.f, 5.0f, 15.f, 2.0f, 150.f, 8.f, 32, 16, 14, 9, 24.f, 280.f, 35.f, 600.f, 1.4f };

    mEnemies["Elite Orc"]         = { "Elite Orc",         "Elite Orc",        250, 80, 20.f, 8.0f, 25.f, 3.0f, 160.f, 9.f, 60, 30, 25, 15, 26.f, 300.f, 45.f, 600.f, 1.4f };
    mEnemies["Orc rider"]         = { "Orc rider",         "Orc rider",        110, 52, 16.f, 6.6f, 0.f, 0.f, 200.f, 14.f, 55, 28, 22, 14, 26.f, 300.f, 45.f, 600.f, 1.3f };
    mEnemies["Greatsword Skeleton"]={ "Greatsword Skeleton","Greatsword Skeleton",130, 65, 20.f, 7.8f, 0.f, 0.f, 140.f, 7.f, 65, 32, 28, 18, 26.f, 300.f, 45.f, 600.f, 1.6f };
    mEnemies["Necromancer"]       = { "Necromancer",       "Necromancer",      100, 46, 15.f, 6.0f, 0.f, 0.f, 150.f, 8.f, 50, 25, 20, 12, 22.f, 300.f, 150.f, 600.f, 1.5f };
    mEnemies["Werebear"]          = { "Werebear",          "Werebear",         150, 78, 22.f, 9.0f, 0.f, 0.f, 130.f, 6.f, 75, 38, 32, 20, 28.f, 300.f, 45.f, 600.f, 1.8f };

    mEnemies["Goblin Giant"]      = { "Goblin Giant",      "Goblin Giant",     800, 150, 30.f, 6.f, 25.f, 3.f, 130.f, 5.f, 180, 80, 80, 40, 45.f, 350.f, 60.f, 1000.f, 1.2f };
    mEnemies["Demon"]             = { "Demon",             "Demon",            2000, 300, 50.f, 10.f, 45.f, 5.f, 140.f, 6.f, 350, 150, 150, 80, 50.f, 380.f, 70.f, 1000.f, 1.0f };
    mEnemies["Bringer Of Death"]  = { "Bringer Of Death",  "Bringer Of Death", 4500, 400, 80.f, 15.f, 70.f, 6.f, 160.f, 8.f, 800, 300, 300, 150, 45.f, 2500.f, 250.f, 99999.f, 1.4f };

    mDefaultEnemy = mEnemies["Skeleton"];
}

void GameData::loadItems() {
    mItems["potion"]         = { "potion",         "Health Potion",     50,        15 };
    mItems["scroll_attack"]  = { "scroll_attack",  "Attack Scroll",      5,        30 };
    mItems["scroll_defense"] = { "scroll_defense", "Defense Scroll",    12,        20 };
    mItems["scroll_hp"]      = { "scroll_hp",      "HP Scroll",          8,        25 };

    mDefaultItem = mItems["potion"];
}

void GameData::loadLootTables() {
    mLootTables["default"] = {
        { "gold",    100 },
        { "potion",   15 },
        { "scroll",   5 }
    };

    mLootTables["boss"] = {
        { "gold",    100 },
        { "potion",   50 },
        { "scroll",   20 }
    };

    mDefaultLoot = mLootTables["default"];
}

void GameData::loadSpawnList() {
    mSpawnList.push_back({ "Goblin Giant",       Region::Center,      5, {2000.f, 2000.f}, true });
    mSpawnList.push_back({ "Demon",              Region::Center,     13, {2000.f, 2000.f}, true });
    mSpawnList.push_back({ "Bringer Of Death",   Region::Center,     29, {2000.f, 2000.f}, true });

    for (int i = 0; i < 6; ++i) mSpawnList.push_back({ "Skeleton", Region::NorthWest, 1, {0.f, 0.f}, false });
    for (int i = 0; i < 6; ++i) mSpawnList.push_back({ "Skeleton", Region::NorthEast, 1, {0.f, 0.f}, false });
    for (int i = 0; i < 6; ++i) mSpawnList.push_back({ "Skeleton", Region::SouthWest, 1, {0.f, 0.f}, false });
    for (int i = 0; i < 7; ++i) mSpawnList.push_back({ "Skeleton", Region::SouthEast, 1, {0.f, 0.f}, false });
}

void GameData::loadAnimations() {
    mAnimationClips["Player_Idle"]   = { 0.15f, true };
    mAnimationClips["Player_Walk"]   = { 0.10f, true };
    mAnimationClips["Player_Attack"] = { 0.08f, false };

    mAnimationClips["Knight_Skill_Q"] = { 0.12f, false };
    mAnimationClips["Knight_Skill_E"] = { 0.06f, true };
    mAnimationClips["Knight_Skill_R"] = { 0.06f, false };

    mAnimationClips["Lancer_Skill_Q"] = { 0.10f, false };
    mAnimationClips["Lancer_Skill_E"] = { 0.10f, false };
    mAnimationClips["Lancer_Skill_R"] = { 0.08f, false };

    mAnimationClips["Archer_Skill_R"] = { 0.08f, false };

    mAnimationClips["Swordsman_Skill_Q"] = { 0.10f, false };
    mAnimationClips["Swordsman_Skill_E"] = { 0.05f, false };
    mAnimationClips["Swordsman_Skill_R"] = { 0.07f, false };

    mAnimationClips["Enemy_Idle"]    = { 0.15f, true };
    mAnimationClips["Enemy_Walk"]    = { 0.12f, true };
    mAnimationClips["Enemy_Attack"]  = { 0.10f, false };

    mAnimationClips["Idle"]          = { 0.15f, true };
    mAnimationClips["Walk"]          = { 0.12f, true };
    mAnimationClips["Attack"]        = { 0.10f, false };

    mDefaultAnimationClip = mAnimationClips["Idle"];
}

void GameData::loadAssetPaths() {
    mAssetPaths["gold"]            = "assets/textures/items/gold.png";
    mAssetPaths["potion"]          = "assets/textures/items/potion.png";
    mAssetPaths["scroll"]          = "assets/textures/items/scroll_hp.png";
    mAssetPaths["scroll_hp"]       = "assets/textures/items/scroll_hp.png";
    mAssetPaths["scroll_attack"]   = "assets/textures/items/scroll_attack.png";
    mAssetPaths["scroll_defense"]  = "assets/textures/items/scroll_defense.png";
    mAssetPaths["arrow_attack"]    = "assets/textures/projectiles/archer_arrow/archer_attack_arrow.png";
    mAssetPaths["arrow_skill"]     = "assets/textures/projectiles/archer_arrow/archer_skill_arrow.png";
    mAssetPaths["skeleton_arrow"]  = "assets/textures/projectiles/skeletonArcher_arrow/skeletonArcher_attack_arrow.png";
    mAssetPaths["necromancer_spell"]= "assets/textures/projectiles/necromancer/necromancer_skill_1_effect.png";
    mAssetPaths["necromancer_spell2"]="assets/textures/projectiles/necromancer/necromancer_skill_2_effect.png";
    mAssetPaths["bringer_spell"]   = "assets/textures/projectiles/bringerOfDeath_spell/bringerOfDeath_spell_0.png";
    mAssetPaths["ui_button"]       = "assets/textures/ui/button.png";
}

void GameData::loadBalanceConstants() {
    mCritChance          = 15;
    mCritMultiplier      = 2.0f;
    mRespawnTime         = 8.0f;
    mDeathGoldPenalty    = 0.15f;
    mDeathExpPenalty     = 0.15f;
    mPotionHealAmount    = 50;
    mWhirlwindDamageMult = 1.2f;
    mDashSpeedMult       = 3.0f;
}

const CharacterData& GameData::getCharacter(CharacterType type) const {
    auto it = mCharacters.find(type);
    if (it != mCharacters.end()) return it->second;
    return mDefaultCharacter;
}

const EnemyData& GameData::getEnemy(const std::string& id) const {
    auto it = mEnemies.find(id);
    if (it != mEnemies.end()) return it->second;
    return mDefaultEnemy;
}

const ItemData& GameData::getItem(const std::string& id) const {
    auto it = mItems.find(id);
    if (it != mItems.end()) return it->second;
    return mDefaultItem;
}

int GameData::getCurrentShopPrice(const std::string& itemId, int playerLevel) const {
    const ItemData& item = getItem(itemId);
    float basePrice = static_cast<float>(item.buyPrice);
    float multiplier = 1.0f;
    if (itemId == "potion") {
        multiplier += (playerLevel - 1) * 0.02f;
    } else {
        multiplier += (playerLevel - 1) * 0.03f;
    }
    return static_cast<int>(std::round(basePrice * multiplier));
}

const std::vector<LootEntry>& GameData::getLootTable(const std::string& tableId) const {
    auto it = mLootTables.find(tableId);
    if (it != mLootTables.end()) return it->second;
    return mDefaultLoot;
}

const std::vector<SpawnEntry>& GameData::getSpawnList() const { return mSpawnList; }

const AnimationClipData& GameData::getAnimationClip(const std::string& key) const {
    auto it = mAnimationClips.find(key);
    if (it != mAnimationClips.end()) return it->second;
    return mDefaultAnimationClip;
}

std::string GameData::getAssetPath(const std::string& assetId) const {
    auto it = mAssetPaths.find(assetId);
    if (it != mAssetPaths.end()) return it->second;
    return "";
}

int GameData::getCritChance() const { return mCritChance; }
float GameData::getCritMultiplier() const { return mCritMultiplier; }
float GameData::getRespawnTime() const { return mRespawnTime; }
float GameData::getDeathGoldPenalty() const { return mDeathGoldPenalty; }
float GameData::getDeathExpPenalty() const { return mDeathExpPenalty; }
int GameData::getPotionHealAmount() const { return mPotionHealAmount; }
float GameData::getWhirlwindDamageMult() const { return mWhirlwindDamageMult; }
float GameData::getDashSpeedMult() const { return mDashSpeedMult; }
