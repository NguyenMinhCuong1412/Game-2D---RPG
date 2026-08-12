#pragma once
#include "common/lib.h"
#include "states/State.h"
#include "character/CharacterType.h"
#include "character/CharacterUtil.h"

class GameData;

class CharacterSelectionState : public State {
public:
    CharacterSelectionState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void updateSelection();
    void drawInfoPanel(sf::RenderWindow& window);

private:
    sf::RenderWindow& mWindow;
    const sf::Font& mFont;
    const GameData& mGameData;
    sf::Text mTitleText;
    std::vector<sf::Text> mOptions;
    std::vector<CharacterType> mCharacters;
    std::size_t mSelectedIdx = 0;

    sf::Texture mBgTexture;
    std::unique_ptr<sf::Sprite> mBgSprite;
    bool mHasBg = false;

    sf::Texture mBtnTexture;
    std::vector<std::unique_ptr<sf::Sprite>> mBtnSprites;
    bool mHasBtnTex = false;

    sf::Texture mHeroTexture;
    std::unique_ptr<sf::Sprite> mHeroSprite;
    float mAnimTimer = 0.f;
    std::size_t mAnimFrame = 0;

    // SELECT button hover
    bool mSelectBtnHovered = false;
    bool mBackBtnHovered = false;
    bool mAnyHovered = false;
    int mHoveredIdx = -1;

    // Skill icons
    sf::Texture mSkillQTexture;
    sf::Texture mSkillETexture;
    sf::Texture mSkillRTexture;
    std::unique_ptr<sf::Sprite> mSkillQSprite;
    std::unique_ptr<sf::Sprite> mSkillESprite;
    std::unique_ptr<sf::Sprite> mSkillRSprite;
    bool mHasSkillIcons = false;
};
