#pragma once
#include "common/lib.h"
#include "states/State.h"
#include "world/Map.h"
#include "save/SaveManager.h"
#include "core/ResourceHolder.h"
#include "database/GameData.h"
#include "entities/Player.h"
#include "entities/Enemy.h"

struct RespawnRequest {
    std::string name;
    Region region;
    int level;
    sf::Vector2f spawnPoint;
    float timer;
};

struct FloatingText {
    sf::Text text;
    sf::Vector2f velocity;
    float lifetime;

    FloatingText(const sf::Font& font, const std::string& str, sf::Vector2f pos, sf::Color color, sf::Vector2f vel, float lt)
        : text(font, str, 20)
        , velocity(vel)
        , lifetime(lt)
    {
        text.setFillColor(color);
        text.setOutlineThickness(1.5f);
        text.setOutlineColor(sf::Color::Black);

        auto bounds = text.getLocalBounds();
        text.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
        text.setPosition(pos);
    }
};

enum class LootType {
    Gold,
    Potion,
    SkillScroll
};

struct LootItem {
    LootType type;
    sf::Vector2f position;
    sf::Sprite sprite;
    int value;

    LootItem(LootType t, sf::Vector2f pos, const sf::Texture& tex, int val)
        : type(t), position(pos), sprite(tex), value(val) {}
};

class PlayingState : public State {
    friend class SpawnSystem;
    friend class LootSystem;
    friend class CollisionSystem;
public:
    Player& getPlayer() { return *mPlayer; }
    const Player& getPlayer() const { return *mPlayer; }

    PlayingState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, CharacterType character, const GameData& gameData);
    PlayingState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const SaveData& save, const GameData& gameData);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    int randomInt(int min, int max) const;

    bool randomChance(int percent) const;

private:
    void initCommon();
    void updateCamera();
    void spawnInitialEnemies();
    void addFloatingText(std::string textStr, sf::Vector2f position, sf::Color color);
    void handleCollisions(float dt);
    void spawnLoot(LootType type, sf::Vector2f position, int value = 0);
    void setupEnemyAnimations(Enemy* enemy);
    void saveCurrentGame(int slot);
    bool checkVictory();
    std::string selectRandomMonsterForLevel(int playerLevel);
    void handlePlayerDeath();
    void applyDeathPenalty();

private:
    sf::RenderWindow& mWindow;
    const sf::Font& mFont;
    const GameData& mGameData;

    Map mMap;
    std::unique_ptr<Player> mPlayer;
    std::vector<Projectile> mProjectiles;
    std::vector<std::unique_ptr<Enemy>> mEnemies;
    std::vector<RespawnRequest> mRespawnQueue;
    std::vector<FloatingText> mFloatingTexts;
    std::vector<LootItem> mLootItems;

    int mInventorySkills = 0;
    bool mShopOpen = false;
    float mShopErrorTimer = 0.f;
    std::string mShopErrorMessage = "";
    int mShopHoveredIdx = -1;
    bool mPauseMenuOpen = false;
    bool mExitConfirm = false;
    int mPauseSelection = 0;
    bool mMeleeHitRegistered = false;
    // Pause menu sub-screens (UI-only)
    bool mPauseSettingsOpen = false;
    bool mPauseControlsOpen = false;
    int mPauseSettingsSelection = 0;
    float mPauseSettingsMaster = 100.f;
    float mPauseSettingsMusic = 100.f;
    float mPauseSettingsSfx = 100.f;
    bool mPauseSettingsFullscreen = false;
    bool mPauseSettingsDamageNumbers = true;
    bool mPauseSettingsDragging = false;
    int mPauseSettingsDragIdx = -1;
    float mPlayTimer = 0.f;
    sf::View mView;
    float mZoomLevel = 1.0f;
    sf::View mMinimapView;
    sf::Texture mBtnTexture;
    bool mHasBtnTex = false;
    sf::Texture mPauseBtnTex;
    std::unique_ptr<sf::Sprite> mPauseBtnSprite;
    bool mHasPauseBtn = false;
    bool mPauseBtnHovered = false;
    int mPauseHoveredIdx = -1;
    bool mPauseSelectionActive = false;
    int mPauseSettingsHoveredIdx = -1;
    bool mPauseSettingsSelectionActive = false;
    bool mPauseControlsBackHovered = false;

    // Boss kill tracking
    bool mBossGoblinDead = false;
    bool mBossDemonDead = false;
    bool mBossBringerDead = false;

    // 3-Life & Checkpoint System
    int mLives = 3;
    bool mIsProcessingDeath = false;
    bool mWaitingForBossDeathSound = false;
    bool mNeedsEnemyRespawn = false;
    bool mBossEncounterActive = false;
    sf::Vector2f mBossCheckpointPos;

    ResourceHolder<sf::Texture, std::string> mTextureHolder;
    mutable std::mt19937 mRng;
};
