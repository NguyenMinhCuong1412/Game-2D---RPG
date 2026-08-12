#include "states/CharacterSelectionState.h"
#include "states/StateManager.h"
#include "states/PlayingState.h"
#include "character/CharacterUtil.h"
#include "database/GameData.h"
#include "core/AudioManager.h"

// UI-only data: skill names derived from SkillRegistry/SkillEffects class names
// These are display-only strings with NO gameplay effect
namespace CharSelectUI {
    struct SkillInfo {
        std::string qName, eName, rName;
        std::string weapon;
        int difficulty; // 1-5 stars, UI-only placeholder
    };

    inline SkillInfo getSkillInfo(CharacterType type) {
        switch (type) {
            case CharacterType::Knight:
                return {"Shield Block", "Whirlwind", "Divine Charge", "Sword & Shield", 3};
            case CharacterType::Archer:
                return {"Rapid Fire", "Overcharge", "Spread Cone", "Bow", 3};
            case CharacterType::Lancer:
                return {"Speed Boost", "Moving Attack", "Lance Charge", "Lance", 3};
            case CharacterType::Swordsman:
                return {"Speed Boost", "Multi Slash", "Ultimate Slash", "Great Sword", 3};
            default:
                return {"???", "???", "???", "???", 3};
        }
    }
}

CharacterSelectionState::CharacterSelectionState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData)
    : State(stateManager)
    , mWindow(window)
    , mFont(font)
    , mGameData(gameData)
    , mTitleText(font)
{
    if (std::filesystem::exists("assets/backgrounds/character_select.png")) {
        if (mBgTexture.loadFromFile("assets/backgrounds/character_select.png")) {
            mBgSprite = std::make_unique<sf::Sprite>(mBgTexture);
            auto size = mBgTexture.getSize();
            mBgSprite->setScale({ 800.f / size.x, 600.f / size.y });
            mHasBg = true;
        }
    }

    std::string btnPath = mGameData.getAssetPath("ui_button");
    if (!btnPath.empty() && std::filesystem::exists(btnPath)) {
        if (mBtnTexture.loadFromFile(btnPath)) mHasBtnTex = true;
    }

    mTitleText.setString("CHOOSE YOUR HERO");
    mTitleText.setCharacterSize(42);
    mTitleText.setFillColor(sf::Color(255, 215, 0));
    mTitleText.setOutlineThickness(2.f);
    mTitleText.setOutlineColor(sf::Color::Black);

    auto titleBounds = mTitleText.getLocalBounds();
    mTitleText.setOrigin({ titleBounds.position.x + titleBounds.size.x / 2.f, titleBounds.position.y + titleBounds.size.y / 2.f });
    mTitleText.setPosition({ 400.f, 35.f });

    mCharacters = {
        CharacterType::Knight,
        CharacterType::Archer,
        CharacterType::Lancer,
        CharacterType::Swordsman
    };

    std::vector<std::string> names = {
        "Knight",
        "Archer",
        "Lancer",
        "Swordsman"
    };

    for (std::size_t i = 0; i < names.size(); ++i) {
        sf::Text text(mFont);
        text.setString(names[i]);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::White);

        float posX = 155.f;
        float posY = 225.f + i * 65.f;

        auto bounds = text.getLocalBounds();
        text.setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
        text.setPosition({ posX, posY });
        mOptions.push_back(text);

        if (mHasBtnTex) {
            auto btnSprite = std::make_unique<sf::Sprite>(mBtnTexture);
            auto btnSize = mBtnTexture.getSize();
            btnSprite->setOrigin({ btnSize.x / 2.f, btnSize.y / 2.f });
            btnSprite->setScale({ 180.f / btnSize.x, 50.f / btnSize.y });
            btnSprite->setPosition({ posX, posY });
            mBtnSprites.push_back(std::move(btnSprite));
        }
    }

    updateSelection();
}

void CharacterSelectionState::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Down || keyPressed->code == sf::Keyboard::Key::S) {
            mAnyHovered = true;
            mHoveredIdx = -1;
            if (mSelectedIdx < mOptions.size() - 1) {
                mSelectedIdx++;
                updateSelection();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Up || keyPressed->code == sf::Keyboard::Key::W) {
            mAnyHovered = true;
            mHoveredIdx = -1;
            if (mSelectedIdx > 0) {
                mSelectedIdx--;
                updateSelection();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Enter) {
            AudioManager::instance().playSfx("button");
            std::cout << "Character Selected: " << static_cast<int>(mCharacters[mSelectedIdx]) << std::endl;
            mStateManager.changeState(std::make_unique<PlayingState>(mStateManager, mWindow, mFont, mCharacters[mSelectedIdx], mGameData), StateID::Playing);
        }
        else if (keyPressed->code == sf::Keyboard::Key::Escape) {
            mStateManager.popState();
        }
    }
    else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos = mWindow.mapPixelToCoords(mouseMoved->position);

        // Character name buttons
        int hovered = -1;
        for (std::size_t i = 0; i < mOptions.size(); ++i) {
            float posX = 155.f;
            float posY = 225.f + i * 65.f;
            sf::FloatRect bounds({ posX - 90.f, posY - 25.f }, { 180.f, 50.f });
            if (bounds.contains(mousePos)) {
                hovered = static_cast<int>(i);
            }
        }
        mHoveredIdx = hovered;
        mAnyHovered = (mHoveredIdx != -1);
        updateSelection();

        // SELECT button hover
        sf::FloatRect selectBounds({590.f, 460.f}, {160.f, 40.f});
        mSelectBtnHovered = selectBounds.contains(mousePos);

        // BACK button hover
        sf::FloatRect backBounds({30.f, 545.f}, {160.f, 40.f});
        mBackBtnHovered = backBounds.contains(mousePos);
    }
    else if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos = mWindow.mapPixelToCoords(mouseBtn->position);

            // Character name buttons
            for (std::size_t i = 0; i < mOptions.size(); ++i) {
                float posX = 155.f;
                float posY = 225.f + i * 65.f;
                sf::FloatRect bounds({ posX - 90.f, posY - 25.f }, { 180.f, 50.f });
                if (bounds.contains(mousePos)) {
                    mSelectedIdx = i;
                    updateSelection();
                }
            }

            // SELECT button click
            sf::FloatRect selectBounds({590.f, 460.f}, {160.f, 40.f});
            if (selectBounds.contains(mousePos)) {
                AudioManager::instance().playSfx("button");
                mStateManager.changeState(std::make_unique<PlayingState>(mStateManager, mWindow, mFont, mCharacters[mSelectedIdx], mGameData), StateID::Playing);
            }

            // BACK button click
            sf::FloatRect backBounds({30.f, 545.f}, {160.f, 40.f});
            if (backBounds.contains(mousePos)) {
                AudioManager::instance().playSfx("button");
                mStateManager.popState();
            }
        }
    }
}

void CharacterSelectionState::update(float dt) {
    mAnimTimer += dt;
    if (mAnimTimer >= 0.12f) {
        mAnimTimer = 0.f;
        mAnimFrame++;

        if (mHeroSprite) {
            auto texSize = mHeroTexture.getSize();
            unsigned int frameW = (texSize.y > 0 && texSize.x >= texSize.y) ? texSize.y : texSize.x;
            unsigned int totalFrames = (frameW > 0) ? texSize.x / frameW : 1;
            if (totalFrames == 0) totalFrames = 1;
            std::size_t currFrame = mAnimFrame % totalFrames;
            mHeroSprite->setTextureRect(sf::IntRect(sf::Vector2i(static_cast<int>(currFrame * frameW), 0), sf::Vector2i(static_cast<int>(frameW), static_cast<int>(frameW))));
        }
    }
}

void CharacterSelectionState::draw(sf::RenderWindow& window) {
    if (mHasBg && mBgSprite) window.draw(*mBgSprite);
    else window.clear(sf::Color(15, 25, 40));

    for (std::size_t i = 0; i < mOptions.size(); ++i) {
        if (mHasBtnTex && i < mBtnSprites.size()) {
            bool highlight = false;
            if (mHoveredIdx != -1) {
                highlight = (static_cast<int>(i) == mHoveredIdx);
            } else {
                highlight = (i == mSelectedIdx);
            }
            mBtnSprites[i]->setColor(highlight ? sf::Color(255, 255, 180) : sf::Color(180, 180, 180));
            window.draw(*mBtnSprites[i]);
        }
        window.draw(mOptions[i]);
    }

    if (mHeroSprite) window.draw(*mHeroSprite);
    window.draw(mTitleText);

    // Draw character info panel
    drawInfoPanel(window);
}

void CharacterSelectionState::updateSelection() {
    for (std::size_t i = 0; i < mOptions.size(); ++i) {
        bool highlight = false;
        if (mHoveredIdx != -1) {
            highlight = (static_cast<int>(i) == mHoveredIdx);
        } else {
            highlight = (i == mSelectedIdx);
        }

        if (highlight) {
            mOptions[i].setFillColor(sf::Color::Yellow);
            mOptions[i].setScale({ 1.1f, 1.1f });
        }
        else {
            mOptions[i].setFillColor(sf::Color::White);
            mOptions[i].setScale({ 1.f, 1.f });
        }

        auto bounds = mOptions[i].getLocalBounds();
        mOptions[i].setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
    }

    std::string idlePath = "assets/textures/player/" + characterKey(mCharacters[mSelectedIdx]) + "_idle.png";
    if (!std::filesystem::exists(idlePath))
        idlePath = "assets/textures/player/" + characterKey(mCharacters[mSelectedIdx]) + "_idle_0.png";

    if (std::filesystem::exists(idlePath) && mHeroTexture.loadFromFile(idlePath)) {
        mHeroSprite = std::make_unique<sf::Sprite>(mHeroTexture);
        auto texSize = mHeroTexture.getSize();
        unsigned int frameW = (texSize.y > 0 && texSize.x >= texSize.y) ? texSize.y : texSize.x;
        mHeroSprite->setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(frameW), static_cast<int>(frameW))));
        mHeroSprite->setOrigin({ frameW / 2.f, frameW / 2.f });
        mHeroSprite->setScale({ 3.f, 3.f });
        mHeroSprite->setPosition({ 400.f, 430.f });
    }

    // Load skill icons
    std::string charName = getCharacterClassName(mCharacters[mSelectedIdx]);
    std::transform(charName.begin(), charName.end(), charName.begin(), ::tolower);
    std::string qPath = "assets/textures/ui/skills/" + charName + "_icon_skill_Q.png";
    std::string ePath = "assets/textures/ui/skills/" + charName + "_icon_skill_E.png";
    std::string rPath = "assets/textures/ui/skills/" + charName + "_icon_skill_R.png";

    mHasSkillIcons = false;
    if (mSkillQTexture.loadFromFile(qPath) && 
        mSkillETexture.loadFromFile(ePath) && 
        mSkillRTexture.loadFromFile(rPath)) {
        mSkillQSprite = std::make_unique<sf::Sprite>(mSkillQTexture);
        mSkillESprite = std::make_unique<sf::Sprite>(mSkillETexture);
        mSkillRSprite = std::make_unique<sf::Sprite>(mSkillRTexture);

        float targetIconSize = 20.f;
        auto qSize = mSkillQTexture.getSize();
        mSkillQSprite->setScale({targetIconSize / qSize.x, targetIconSize / qSize.y});

        auto eSize = mSkillETexture.getSize();
        mSkillESprite->setScale({targetIconSize / eSize.x, targetIconSize / eSize.y});

        auto rSize = mSkillRTexture.getSize();
        mSkillRSprite->setScale({targetIconSize / rSize.x, targetIconSize / rSize.y});

        mHasSkillIcons = true;
    }
}

void CharacterSelectionState::drawInfoPanel(sf::RenderWindow& window) {
    CharacterType type = mCharacters[mSelectedIdx];
    const auto& data = mGameData.getCharacter(type);
    auto skillInfo = CharSelectUI::getSkillInfo(type);

    // Panel background (right side) - narrowed and sized to fit content
    float panelX = 560.f;
    float panelY = 180.f;
    float panelW = 220.f;
    float panelH = 265.f;

    sf::RectangleShape panelBg({panelW, panelH});
    panelBg.setFillColor(sf::Color(10, 10, 20, 210));
    panelBg.setOutlineThickness(2.f);
    panelBg.setOutlineColor(sf::Color(255, 215, 0, 150));
    panelBg.setPosition({panelX, panelY});
    window.draw(panelBg);

    float contentX = panelX + 15.f;
    float y = panelY + 12.f;

    // Helper: draw stat bar inside narrower panel
    auto drawStatBar = [&](const std::string& label, float value, float maxValue, sf::Color barColor) {
        sf::Text labelText(mFont, label, 14);
        labelText.setFillColor(sf::Color(200, 200, 200));
        labelText.setPosition({contentX, y});
        window.draw(labelText);

        float barX = contentX + 75.f;
        float barW = 110.f;
        float barH = 12.f;

        // Background
        sf::RectangleShape bgBar({barW, barH});
        bgBar.setFillColor(sf::Color(40, 40, 50));
        bgBar.setPosition({barX, y + 4.f});
        window.draw(bgBar);

        // Fill
        float fillW = (value / maxValue) * barW;
        sf::RectangleShape fillBar({fillW, barH});
        fillBar.setFillColor(barColor);
        fillBar.setPosition({barX, y + 4.f});
        window.draw(fillBar);

        y += 24.f;
    };

    // STATS — READ-ONLY from CharacterData
    // HP: baseHp ranges 80-130 across characters
    drawStatBar("HP", static_cast<float>(data.baseHp), 150.f, sf::Color(220, 50, 50));

    // ATTACK: baseMeleeDamage for melee classes, baseRangedDamage for archer
    float attackVal = (type == CharacterType::Archer) ? data.baseRangedDamage : data.baseMeleeDamage;
    drawStatBar("ATTACK", attackVal, 20.f, sf::Color(255, 165, 0));

    // DEFENSE: defensePerLevel
    drawStatBar("DEFENSE", data.defensePerLevel, 5.f, sf::Color(100, 150, 255));

    // SPEED: baseSpeed ranges 220-280 across characters
    drawStatBar("SPEED", data.baseSpeed, 300.f, sf::Color(100, 255, 100));

    y += 4.f;

    // Separator line
    sf::RectangleShape sep({panelW - 30.f, 1.f});
    sep.setFillColor(sf::Color(255, 215, 0, 80));
    sep.setPosition({contentX, y});
    window.draw(sep);
    y += 10.f;

    // SKILLS
    auto drawSkillLine = [&](const std::string& key, const std::string& name, const std::unique_ptr<sf::Sprite>& icon) {
        sf::Text keyText(mFont, key, 14);
        keyText.setFillColor(sf::Color(255, 215, 0));
        auto kb = keyText.getLocalBounds();
        keyText.setOrigin({0.f, kb.position.y + kb.size.y / 2.f});
        keyText.setPosition({contentX, y + 10.f});
        window.draw(keyText);

        float textOffset = 28.f;
        if (mHasSkillIcons && icon) {
            sf::Sprite drawIcon = *icon;
            auto iconSize = drawIcon.getTexture().getSize();
            drawIcon.setOrigin({0.f, static_cast<float>(iconSize.y) / 2.f});
            drawIcon.setPosition({contentX + 26.f, y + 10.f});
            window.draw(drawIcon);
            textOffset = 54.f;
        }

        sf::Text nameText(mFont, name, 13);
        nameText.setFillColor(sf::Color(220, 220, 220));
        auto nb = nameText.getLocalBounds();
        nameText.setOrigin({0.f, nb.position.y + nb.size.y / 2.f});
        nameText.setPosition({contentX + textOffset, y + 10.f});
        window.draw(nameText);

        y += 30.f;
    };

    drawSkillLine("Q", skillInfo.qName, mSkillQSprite);
    drawSkillLine("E", skillInfo.eName, mSkillESprite);
    drawSkillLine("R", skillInfo.rName, mSkillRSprite);

    y += 4.f;

    // Separator
    sf::RectangleShape sep2({panelW - 30.f, 1.f});
    sep2.setFillColor(sf::Color(255, 215, 0, 80));
    sep2.setPosition({contentX, y});
    window.draw(sep2);
    y += 8.f;

    // WEAPON
    sf::Text weaponLabel(mFont, "WEAPON", 12);
    weaponLabel.setFillColor(sf::Color(150, 150, 150));
    weaponLabel.setPosition({contentX, y});
    window.draw(weaponLabel);
    y += 16.f;

    sf::Text weaponName(mFont, skillInfo.weapon, 14);
    weaponName.setFillColor(sf::Color(255, 215, 0));
    weaponName.setPosition({contentX, y});
    window.draw(weaponName);


    // SELECT button
    float selectBtnX = 590.f;
    float selectBtnY = 460.f;
    float selectBtnW = 160.f;
    float selectBtnH = 40.f;

    if (mHasBtnTex) {
        sf::Sprite btnSprite(mBtnTexture);
        auto btnSize = mBtnTexture.getSize();
        btnSprite.setOrigin({btnSize.x / 2.f, btnSize.y / 2.f});
        btnSprite.setScale({selectBtnW / btnSize.x, selectBtnH / btnSize.y});
        btnSprite.setPosition({selectBtnX + selectBtnW / 2.f, selectBtnY + selectBtnH / 2.f});
        btnSprite.setColor(mSelectBtnHovered ? sf::Color(255, 255, 180) : sf::Color(180, 180, 180));
        window.draw(btnSprite);
    }

    sf::Text selectText(mFont, "SELECT", 20);
    selectText.setFillColor(mSelectBtnHovered ? sf::Color::Yellow : sf::Color::White);
    auto sb = selectText.getLocalBounds();
    selectText.setOrigin({sb.position.x + sb.size.x / 2.f, sb.position.y + sb.size.y / 2.f});
    selectText.setPosition({selectBtnX + selectBtnW / 2.f, selectBtnY + selectBtnH / 2.f});
    window.draw(selectText);


    // BACK button
    float backBtnX = 30.f;
    float backBtnY = 545.f;
    float backBtnW = 160.f;
    float backBtnH = 40.f;

    if (mHasBtnTex) {
        sf::Sprite btnSprite(mBtnTexture);
        auto btnSize = mBtnTexture.getSize();
        btnSprite.setOrigin({btnSize.x / 2.f, btnSize.y / 2.f});
        btnSprite.setScale({backBtnW / btnSize.x, backBtnH / btnSize.y});
        btnSprite.setPosition({backBtnX + backBtnW / 2.f, backBtnY + backBtnH / 2.f});
        btnSprite.setColor(mBackBtnHovered ? sf::Color(255, 255, 180) : sf::Color(180, 180, 180));
        window.draw(btnSprite);
    }

    sf::Text backText(mFont, "BACK", 20);
    backText.setFillColor(mBackBtnHovered ? sf::Color::Yellow : sf::Color::White);
    auto bb = backText.getLocalBounds();
    backText.setOrigin({bb.position.x + bb.size.x / 2.f, bb.position.y + bb.size.y / 2.f});
    backText.setPosition({backBtnX + backBtnW / 2.f, backBtnY + backBtnH / 2.f});
    window.draw(backText);
}
