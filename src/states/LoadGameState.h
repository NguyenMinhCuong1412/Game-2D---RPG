#pragma once
#include "common/lib.h"
#include "states/State.h"
#include "save/SaveManager.h"

class GameData;

class LoadGameState : public State {
public:
    LoadGameState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void refreshSlots();
    void updateSelection();

private:
    sf::RenderWindow& mWindow;
    const sf::Font& mFont;
    const GameData& mGameData;

    sf::Text mTitleText;
    std::vector<sf::Text> mSlotTexts;
    std::size_t mSelectedIdx = 0;
    std::array<SaveData, SaveManager::MAX_SLOTS> mSaveSlots;

    sf::Texture mBgTexture;
    std::unique_ptr<sf::Sprite> mBgSprite;
    bool mHasBg = false;

    sf::Texture mBtnTexture;
    std::vector<std::unique_ptr<sf::Sprite>> mBtnSprites;
    bool mHasBtnTex = false;

    // Slot submenu (Play / Delete)
    bool mShowSlotMenu = false;
    int mSlotMenuSelection = 0; // 0 = Play, 1 = Delete

    sf::Text mBackText;
    std::unique_ptr<sf::Sprite> mBackBtnSprite;
    bool mBackHovered = false;
    bool mSlotHovered = false;
};
