#pragma once
#include "common/lib.h"
#include "states/State.h"

class GameData;

class HowToPlayState : public State {
public:
    HowToPlayState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void buildPages();

private:
    sf::RenderWindow& mWindow;
    const sf::Font& mFont;
    const GameData& mGameData;

    int mCurrentPage = 0;
    int mTotalPages = 8;

    // Each page: title + lines of content
    struct PageData {
        std::string title;
        std::vector<std::string> lines;
    };
    std::vector<PageData> mPages;

    sf::Texture mBgTexture;
    std::unique_ptr<sf::Sprite> mBgSprite;
    bool mHasBg = false;

    sf::Texture mBtnTexture;
    bool mHasBtnTex = false;

    std::array<sf::Texture, 4> mHeroTextures;
    std::vector<sf::Sprite> mHeroSprites;
    bool mHasHeroes = false;
    float mHeroAnimTime = 0.f;
    int mHeroFrame = 0;

    int mHoveredBtn = -1; // 0=Prev, 1=Next, 2=Back
};
