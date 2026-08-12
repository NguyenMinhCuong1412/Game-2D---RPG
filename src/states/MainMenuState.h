#pragma once
#include "common/lib.h"
#include "states/State.h"

class GameData;

class MainMenuState : public State {
public:
    MainMenuState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void updateSelection();

private:
    sf::RenderWindow& mWindow;
    const sf::Font& mFont;
    const GameData& mGameData;
    sf::Text mTitleText;
    std::vector<sf::Text> mOptions;
    std::vector<std::string> mOptionStrings;
    std::size_t mSelectedIdx = 0;

    sf::Texture mBgTexture;
    std::unique_ptr<sf::Sprite> mBgSprite;
    bool mHasBg = false;

    sf::Texture mBtnTexture;
    std::vector<std::unique_ptr<sf::Sprite>> mBtnSprites;
    bool mHasBtnTex = false;
    bool mAnyHovered = false;
};
