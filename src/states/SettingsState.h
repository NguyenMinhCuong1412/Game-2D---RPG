#pragma once
#include "common/lib.h"
#include "states/State.h"

class GameData;

class SettingsState : public State {
public:
    SettingsState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void applySettings();
    void updateSliderPositions();

private:
    sf::RenderWindow& mWindow;
    const sf::Font& mFont;
    const GameData& mGameData;

    // Setting values (local copies, applied on APPLY)
    float mMasterVolume = 100.f;
    float mMusicVolume = 100.f;
    float mSfxVolume = 100.f;
    bool mFullscreen = false;
    bool mDamageNumbers = true;

    int mSelectedIdx = 0; // 0-6: Master, Music, SFX, Fullscreen, DamageNumbers, Apply, Back
    static constexpr int ITEM_COUNT = 7;
    bool mAnyHovered = false;

    sf::Texture mBgTexture;
    std::unique_ptr<sf::Sprite> mBgSprite;
    bool mHasBg = false;

    sf::Texture mBtnTexture;
    bool mHasBtnTex = false;

    bool mDraggingSlider = false;
    int mDragSliderIdx = -1;
};
