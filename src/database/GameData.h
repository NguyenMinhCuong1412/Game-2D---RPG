#pragma once
#include "common/lib.h"
#include "character/CharacterType.h"
#include "world/Map.h"

struct CharacterData {
    std::string name;
    int baseHp;
    float baseSpeed;
    float qCooldown;
    float eCooldown;
    float rCooldown;
    float baseMeleeDamage;
    float baseRangedDamage;
    float damagePerLevel;
    int hpPerLevel;
    float defensePerLevel;
};

struct EnemyData {
    std::string id;
    std::string displayName;
    int baseHp;
    int hpPerLevel;
    float baseDamage;
    float damagePerLevel;
    float baseDefense;
    float defensePerLevel;
    float baseSpeed;
    float speedPerLevel;
    int baseExp;
    int expPerLevel;
    int baseGold;
    int goldPerLevel;
    float radius;
    float detectionRange;
    float attackRange;
    float leashRange;
    float attackCooldown;
};

struct ItemData {
    std::string id;
    std::string name;
    int effectValue;
    int buyPrice;
};

struct LootEntry {
    std::string itemId;
    int dropChance;
};

struct SpawnEntry {
    std::string enemyName;
    Region region;
    int baseLevel;
    sf::Vector2f spawnPoint;
    bool isBoss;
};

struct AnimationClipData {
    float frameDuration;
    bool loop;
};

class GameData {
private:
    void loadCharacters();
    void loadEnemies();
    void loadItems();
    void loadLootTables();
    void loadSpawnList();
    void loadAnimations();
    void loadAssetPaths();
    void loadBalanceConstants();

    std::unordered_map<CharacterType, CharacterData> mCharacters;
    std::unordered_map<std::string, EnemyData> mEnemies;
    std::unordered_map<std::string, ItemData> mItems;
    std::unordered_map<std::string, std::vector<LootEntry>> mLootTables;
    std::vector<SpawnEntry> mSpawnList;
    std::unordered_map<std::string, AnimationClipData> mAnimationClips;
    std::unordered_map<std::string, std::string> mAssetPaths;

    CharacterData mDefaultCharacter;
    EnemyData mDefaultEnemy;
    ItemData mDefaultItem;
    std::vector<LootEntry> mDefaultLoot;
    AnimationClipData mDefaultAnimationClip;

    int mCritChance;
    float mCritMultiplier;
    float mRespawnTime;
    float mDeathGoldPenalty;
    float mDeathExpPenalty;
    int mPotionHealAmount;
    float mWhirlwindDamageMult;
    float mDashSpeedMult;

    sf::Texture mDefeatTexture;
    bool mHasDefeatTexture = false;
public:
    GameData();

    const CharacterData& getCharacter(CharacterType type) const;
    const EnemyData& getEnemy(const std::string& id) const;
    const ItemData& getItem(const std::string& id) const;
    int getCurrentShopPrice(const std::string& itemId, int playerLevel) const;
    const std::vector<LootEntry>& getLootTable(const std::string& tableId) const;
    const std::vector<SpawnEntry>& getSpawnList() const;
    const AnimationClipData& getAnimationClip(const std::string& key) const;
    std::string getAssetPath(const std::string& assetId) const;
    const sf::Texture* getDefeatTexture() const;

    int getCritChance() const;
    float getCritMultiplier() const;
    float getRespawnTime() const;
    float getDeathGoldPenalty() const;
    float getDeathExpPenalty() const;
    int getPotionHealAmount() const;
    float getWhirlwindDamageMult() const;
    float getDashSpeedMult() const;
};