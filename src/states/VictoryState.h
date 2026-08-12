#pragma once
#include "common/lib.h"
#include "states/State.h"

class GameData;

class VictoryState : public State {
public:
    VictoryState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    sf::RenderWindow& mWindow;
    const sf::Font& mFont;
    const GameData& mGameData;
    float mTimer = 0.f;

    sf::Texture mBgTexture;
    std::unique_ptr<sf::Sprite> mBgSprite;
    bool mHasBg = false;
};
