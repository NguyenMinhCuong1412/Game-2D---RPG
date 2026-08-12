#include "states/HowToPlayState.h"
#include "states/StateManager.h"
#include "database/GameData.h"
#include "core/AudioManager.h"

HowToPlayState::HowToPlayState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData)
    : State(stateManager)
    , mWindow(window)
    , mFont(font)
    , mGameData(gameData)
{
    std::string bgPath = "assets/backgrounds/menu.png";
    if (std::filesystem::exists(bgPath)) {
        if (mBgTexture.loadFromFile(bgPath)) {
            mBgSprite = std::make_unique<sf::Sprite>(mBgTexture);
            auto size = mBgTexture.getSize();
            mBgSprite->setScale({800.f / size.x, 600.f / size.y});
            mHasBg = true;
        }
    }

    std::string btnPath = mGameData.getAssetPath("ui_button");
    if (!btnPath.empty() && std::filesystem::exists(btnPath))
        if (mBtnTexture.loadFromFile(btnPath)) mHasBtnTex = true;

    // Load hero textures
    std::string paths[4] = {
        "assets/textures/player/Knight/knight_idle.png",
        "assets/textures/player/Archer/archer_idle.png",
        "assets/textures/player/Lancer/lancer_idle.png",
        "assets/textures/player/Swordsman/swordsman_idle.png"
    };
    bool allLoaded = true;
    for (int i = 0; i < 4; ++i) {
        if (std::filesystem::exists(paths[i]) && mHeroTextures[i].loadFromFile(paths[i])) {
            sf::Sprite sprite(mHeroTextures[i]);
            auto size = mHeroTextures[i].getSize();
            sprite.setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(size.y), static_cast<int>(size.y)}));
            sprite.setScale({3.5f, 3.5f});
            mHeroSprites.push_back(sprite);
        } else {
            allLoaded = false;
        }
    }
    mHasHeroes = allLoaded;

    buildPages();
}

void HowToPlayState::buildPages() {
    mPages.clear();

    // PAGE 1 — WELCOME
    mPages.push_back({ "WELCOME", {
        "Welcome to #RLegend of the Realm#W!",
        "",
        "You are about to step into a",
        "treacherous world overrun by",
        "fiendish beasts and dark magic.",
        "",
        "Your destiny: forge your path,",
        "slay monstrous hordes,",
        "and vanquish all three",
        "#RLegendary Bosses#W",
        "to restore peace.",
        "",
        "Choose your #OChampion#W and prepare",
        "for glory!"
    } });

    // PAGE 2 — STARTING THE GAME
    mPages.push_back({ "STARTING THE GAME", {
        "From the #CMain Menu#W, select #CPLAY#W",
        "to embark on your journey.",
        "",
        "Select your #OClass#W to begin.",
        "4 #OHeroes#W await your command:",
        "",
        "   #OKNIGHT#W [ Stalwart Tank, Shield ]",
        "   #OARCHER#W [ Lethal Ranged Marksman ]",
        "   #OLANCER#W [ High HP, Charging Strike ]",
        "   #OSWORDSMAN#W [ Swift Melee Blade ]",
        "",
        "Each hero possesses distinct skills,",
        "attributes, and playstyles.",
        "",
        "Pick your hero and enter battle!"
    } });

    // PAGE 3 — CONTROLS
    mPages.push_back({ "CONTROLS", {
        "    #YW A S D#W      MOVEMENT",
        "    #YSPACE#W        DASH / DODGE",
        "    #YLMB#W          BASIC ATTACK",
        "    #YQ#W            SKILL Q",
        "    #YE#W            SKILL E",
        "    #YR#W            ULTIMATE / SKILL R",
        "    #YH#W            USE POTION",
        "    #YTAB#W          OPEN SHOP",
        "    #YESC#W          PAUSE MENU",
        "    #YMOUSE WHEEL#W  CAMERA ZOOM",
        "",
        "Traverse with #YWASD#W keys.",
        "Aim your strikes with the #Ccursor#W.",
        "Unleash devastating #Oskills#W when",
        "#Ycooldowns#W are ready."
    } });

    // PAGE 4 — COMBAT
    mPages.push_back({ "COMBAT", {
        "#RBASIC ATTACK#W (#YLMB#W):",
        "Melee heroes cleave nearby foes.",
        "Archers rain arrows from afar.",
        "",
        "#RHERO SKILLS#W (#YQ#W, #YE#W, #YR#W):",
        "3 unique abilities per class.",
        "Keep an eye on cooldown timers",
        "at the bottom skill bar.",
        "",
        "#RDASH#W (#YSPACE#W):",
        "Evade incoming damage in your",
        "movement direction. Grants brief",
        "INVULNERABILITY frame.",
        "",
        "Stay alive by dodging attacks and",
        "consuming #GPotions#W (#YH#W)."
    } });

    // PAGE 5 — EXPLORATION
    mPages.push_back({ "EXPLORATION", {
        "Venture across a vast open world",
        "divided into 5 dangerous regions:",
        "#GNorthWest#W, #GNorthEast#W,",
        "#GSouthWest#W, #GSouthEast#W,",
        "the #CSanctuary Center#W.",
        "",
        "#RMINIMAP GUIDE#W:",
        "Bottom-left corner shows your",
        "location (Cyan) and nearby",
        "threats (Red/Yellow).",
        "",
        "#RTHE SHOP ZONE#W (#YTAB#W):",
        "The central hub holds the Shop.",
        "Stand in the golden circle and",
        "",
        "Adjust focus with #YMOUSE WHEEL#W."
    } });

    // PAGE 6 — ITEMS & RESOURCES
    mPages.push_back({ "ITEMS & RESOURCES", {
        "#YGOLD#W (#Ycurrency#W):",
        "Looted from fallen monsters.",
        "Used to purchase potion & buffs.",
        "",
        "#GPOTIONS#W:",
        "Restores 50 HP upon use (#YH#W).",
        "Restock at the Shop.",
        "",
        "#OANCIENT SCROLLS#W:",
        "Vitality Scroll #G+8% Max HP#W",
        "Might Scroll    #R+5% DMG#W",
        "Aegis Scroll    #C+12% DEF#W",
        "Bonus stacks linearly.",
        "",
        "Scrolls also drop as loot.",
        "Walk over items to collect them."
    } });

    // PAGE 7 — PROGRESSION
    mPages.push_back({ "PROGRESSION", {
        "#OCHARACTER LEVEL UP#W:",
        "Slay foes to gather EXP.",
        "Required EXP = Level x 100.",
        "Each Level grants stat gains.",
        "All classes start with #G100 HP#W, #R15 DMG#W, #C5 DEF#W",
        "#RMAX LEVEL: 29#W",
        "",
        "#RBOSS CONQUEST PROGRESSION#W:",
        "#RGoblin Giant#W (#YLVL 5#W)",
        "|",
        "#RDemon Lord#W (#YLVL 13#W)",
        "|",
        "#RBringer Of Death#W (#YLVL 29#W)",
        "#RFINAL BOSS#W"
    } });

    // PAGE 8 — COMPLETE YOUR JOURNEY
    mPages.push_back({ "COMPLETE YOUR JOURNEY", {
        "#OYOUR ULTIMATE QUEST#W:",
        "",
        "Vanquish the #Y3#W #RBoss Titans#W:",
        "",
        "#RGoblin Giant#W",
        "#RDemon Lord#W",
        "#RBringer Of Death#W",
        "",
        "You have #R3 Lives#W per #Rrun#W.",
        "#RDeath#W penalizes #R15% Gold & EXP#W.",
        "#RExhausting all lives#W ends the #Rrun#W.",
        "",
        "Save progress from the #CPAUSE MENU#W.",
        "",
        "#GMay luck favor your blade!#W"
    } });

    mTotalPages = static_cast<int>(mPages.size());
}

void HowToPlayState::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Escape) {
            mStateManager.popState();
        }
        else if (keyPressed->code == sf::Keyboard::Key::Right || keyPressed->code == sf::Keyboard::Key::D) {
            if (mCurrentPage < mTotalPages - 1) {
                mCurrentPage++;
                AudioManager::instance().playSfx("button");
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Left || keyPressed->code == sf::Keyboard::Key::A) {
            if (mCurrentPage > 0) {
                mCurrentPage--;
                AudioManager::instance().playSfx("button");
            }
        }
    }
    else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos = mWindow.mapPixelToCoords(mouseMoved->position);
        mHoveredBtn = -1;
        // Previous button
        if (mCurrentPage > 0) {
            sf::FloatRect prevBounds({120.f, 540.f}, {140.f, 40.f});
            if (prevBounds.contains(mousePos)) mHoveredBtn = 0;
        }
        // Next button
        if (mCurrentPage < mTotalPages - 1) {
            sf::FloatRect nextBounds({540.f, 540.f}, {140.f, 40.f});
            if (nextBounds.contains(mousePos)) mHoveredBtn = 1;
        }
        // Back button
        sf::FloatRect backBounds({330.f, 540.f}, {140.f, 40.f});
        if (backBounds.contains(mousePos)) mHoveredBtn = 2;
    }
    else if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos = mWindow.mapPixelToCoords(mouseBtn->position);
            // Previous
            if (mCurrentPage > 0) {
                sf::FloatRect prevBounds({120.f, 540.f}, {140.f, 40.f});
                if (prevBounds.contains(mousePos)) {
                    mCurrentPage--;
                    AudioManager::instance().playSfx("button");
                }
            }
            // Next
            if (mCurrentPage < mTotalPages - 1) {
                sf::FloatRect nextBounds({540.f, 540.f}, {140.f, 40.f});
                if (nextBounds.contains(mousePos)) {
                    mCurrentPage++;
                    AudioManager::instance().playSfx("button");
                }
            }
            // Back
            sf::FloatRect backBounds({330.f, 540.f}, {140.f, 40.f});
            if (backBounds.contains(mousePos)) {
                AudioManager::instance().playSfx("button");
                mStateManager.popState();
            }
        }
    }
}

void HowToPlayState::update(float dt) {
    if (mHasHeroes && mHeroSprites.size() == 4) {
        mHeroAnimTime += dt;
        if (mHeroAnimTime >= 0.15f) { // 0.15s per frame
            mHeroAnimTime = 0.f;
            mHeroFrame++;
            for (size_t i = 0; i < mHeroSprites.size(); ++i) {
                auto size = mHeroTextures[i].getSize();
                int frameWidth = static_cast<int>(size.y);
                int maxFrames = static_cast<int>(size.x) / frameWidth;
                if (maxFrames > 0) {
                    int currentFrame = mHeroFrame % maxFrames;
                    mHeroSprites[i].setTextureRect(sf::IntRect({currentFrame * frameWidth, 0}, {frameWidth, frameWidth}));
                }
            }
        }
    }
}

void HowToPlayState::draw(sf::RenderWindow& window) {
    // Background
    if (mHasBg && mBgSprite) {
        mBgSprite->setColor(sf::Color(80, 80, 80)); // Darken background
        window.draw(*mBgSprite);
        mBgSprite->setColor(sf::Color::White);
    } else {
        window.clear(sf::Color(15, 15, 25));
    }

    // Dark panel overlay
    sf::RectangleShape panelBg({700.f, 470.f});
    panelBg.setFillColor(sf::Color(10, 10, 20, 220));
    panelBg.setOutlineThickness(2.f);
    panelBg.setOutlineColor(sf::Color(255, 215, 0, 180));
    panelBg.setPosition({50.f, 30.f});
    window.draw(panelBg);

    if (mCurrentPage == 1 && mHasHeroes && mHeroSprites.size() == 4) {
        float startX = 320.f;
        float spacingX = 140.f;
        
        float topY = 30.f;
        float bottomY = 180.f;
        
        mHeroSprites[0].setPosition({startX, topY});
        mHeroSprites[1].setPosition({startX + spacingX, topY});
        mHeroSprites[2].setPosition({startX, bottomY});
        mHeroSprites[3].setPosition({startX + spacingX, bottomY});
        
        for (int i = 0; i < 4; ++i) {
            window.draw(mHeroSprites[i]);
        }
    }

    if (mCurrentPage >= 0 && mCurrentPage < static_cast<int>(mPages.size())) {
        const auto& page = mPages[mCurrentPage];

        // Page title
        sf::Text titleText(mFont, page.title, 28);
        titleText.setFillColor(sf::Color(255, 215, 0));
        titleText.setOutlineThickness(2.f);
        titleText.setOutlineColor(sf::Color::Black);
        auto tb = titleText.getLocalBounds();
        titleText.setOrigin({tb.size.x / 2.f, tb.size.y / 2.f});
        titleText.setPosition({400.f, 65.f});
        window.draw(titleText);

        // Page content
        float defaultLineY = 120.f;
        int lineIndex = 0;
        for (const auto& line : page.lines) {
            float totalLineWidth = 0.f;
            if (mCurrentPage == 0 || mCurrentPage == 3 || mCurrentPage == 4 || mCurrentPage == 5 || mCurrentPage == 6 || mCurrentPage == 7) {
                std::string rawString = "";
                for (size_t i = 0; i < line.size(); ++i) {
                    if (line[i] == '#' && i + 1 < line.size()) {
                        i++; // skip color tag
                    } else {
                        rawString += line[i];
                    }
                }
                if (!rawString.empty()) {
                    sf::Text fullText(mFont, rawString, 16);
                    totalLineWidth = fullText.findCharacterPos(rawString.size()).x;
                }
            }

            float currentX = 90.f;
            float lineY = defaultLineY;
            if (mCurrentPage == 0 || mCurrentPage == 6 || mCurrentPage == 7) {
                currentX = 400.f - (totalLineWidth / 2.f);
            } else if (mCurrentPage == 2) {
                if (lineIndex < 10) {
                    currentX = 80.f;
                    lineY = 130.f + lineIndex * 35.f; // Căn giữa khoảng không giữa tiêu đề và đáy khung
                } else if (lineIndex == 10) {
                    // empty line
                } else {
                    currentX = 460.f;
                    lineY = 190.f + (lineIndex - 11) * 35.f; // Vertically centered
                }
            } else if (mCurrentPage == 3) {
                if (lineIndex <= 2) {
                    currentX = 400.f - (totalLineWidth / 2.f);
                    lineY = 120.f + lineIndex * 26.f;
                } else if (lineIndex >= 4 && lineIndex <= 7) { // HERO SKILLS
                    currentX = 225.f - (totalLineWidth / 2.f);
                    lineY = 240.f + (lineIndex - 4) * 26.f;
                } else if (lineIndex >= 9 && lineIndex <= 12) { // DASH
                    currentX = 575.f - (totalLineWidth / 2.f);
                    lineY = 240.f + (lineIndex - 9) * 26.f;
                } else if (lineIndex >= 14) { // Potion
                    currentX = 400.f - (totalLineWidth / 2.f);
                    lineY = 400.f + (lineIndex - 14) * 26.f;
                }
            } else if (mCurrentPage == 4) {
                if (lineIndex <= 4) {
                    currentX = 400.f - (totalLineWidth / 2.f);
                    lineY = 110.f + lineIndex * 26.f;
                } else if (lineIndex >= 6 && lineIndex <= 9) { // MINIMAP
                    currentX = 225.f - (totalLineWidth / 2.f);
                    lineY = 260.f + (lineIndex - 6) * 26.f;
                } else if (lineIndex >= 11 && lineIndex <= 13) { // SHOP
                    currentX = 575.f - (totalLineWidth / 2.f);
                    lineY = 260.f + (lineIndex - 11) * 26.f;
                } else if (lineIndex == 15) { // Mouse wheel
                    currentX = 400.f - (totalLineWidth / 2.f);
                    lineY = 400.f;
                }
            } else if (mCurrentPage == 5) {
                if (lineIndex <= 2) { // GOLD
                    currentX = 225.f - (totalLineWidth / 2.f);
                    lineY = 110.f + lineIndex * 26.f;
                } else if (lineIndex >= 4 && lineIndex <= 6) { // POTIONS
                    currentX = 575.f - (totalLineWidth / 2.f);
                    lineY = 110.f + (lineIndex - 4) * 26.f;
                } else if (lineIndex >= 8 && lineIndex <= 12) { // SCROLLS
                    currentX = 400.f - (totalLineWidth / 2.f);
                    lineY = 220.f + (lineIndex - 8) * 26.f;
                } else if (lineIndex >= 14) { // Last lines
                    currentX = 400.f - (totalLineWidth / 2.f);
                    lineY = 380.f + (lineIndex - 14) * 26.f;
                }
            }
            
            sf::Color currentColor = sf::Color(220, 220, 220); // Default White/Gray
            std::string currentSegment = "";
            
            for (size_t i = 0; i < line.size(); ++i) {
                // Skip leading spaces for the snap logic by requiring i > 2
                if (mCurrentPage == 2 && lineIndex < 10 && i > 2 && line[i] == ' ' && i + 1 < line.size() && line[i+1] == ' ') {
                    if (!currentSegment.empty()) {
                        sf::Text text(mFont, currentSegment, 16);
                        text.setFillColor(currentColor);
                        text.setPosition({currentX, lineY});
                        window.draw(text);
                        currentX = text.findCharacterPos(currentSegment.size()).x;
                        currentSegment = "";
                    }
                    currentX = 220.f; // Snap to fixed column for descriptions
                    while (i < line.size() && line[i] == ' ') i++;
                    i--;
                    continue;
                }

                if (line[i] == '#' && i + 1 < line.size()) {
                    if (!currentSegment.empty()) {
                        sf::Text text(mFont, currentSegment, 16);
                        text.setFillColor(currentColor);
                        text.setPosition({currentX, lineY});
                        window.draw(text);
                        
                        currentX = text.findCharacterPos(currentSegment.size()).x;
                        currentSegment = "";
                    }
                    
                    char colorCode = line[i+1];
                    if (colorCode == 'R') currentColor = sf::Color(255, 100, 100);
                    else if (colorCode == 'Y') currentColor = sf::Color(255, 215, 0);
                    else if (colorCode == 'G') currentColor = sf::Color(100, 255, 100);
                    else if (colorCode == 'C') currentColor = sf::Color(100, 255, 255);
                    else if (colorCode == 'O') currentColor = sf::Color(255, 165, 0);
                    else if (colorCode == 'M') currentColor = sf::Color(255, 100, 255);
                    else if (colorCode == 'W') currentColor = sf::Color(220, 220, 220);
                    i++; // skip color code
                } else {
                    currentSegment += line[i];
                }
            }
            if (!currentSegment.empty() && !(mCurrentPage == 2 && lineIndex == 10)) {
                sf::Text text(mFont, currentSegment, 16);
                text.setFillColor(currentColor);
                text.setPosition({currentX, lineY});
                window.draw(text);
            }
            defaultLineY += 24.f;
            lineIndex++;
        }
    }

    // Page indicator
    sf::Text pageText(mFont, "PAGE " + std::to_string(mCurrentPage + 1) + " / " + std::to_string(mTotalPages), 16);
    pageText.setFillColor(sf::Color(180, 180, 180));
    auto pb = pageText.getLocalBounds();
    pageText.setOrigin({pb.size.x / 2.f, pb.size.y / 2.f});
    pageText.setPosition({400.f, 515.f});
    window.draw(pageText);

    // Navigation buttons
    auto drawNavButton = [&](float x, float y, const std::string& label, bool enabled, int btnIdx) {
        if (!enabled) return;
        bool hovered = (mHoveredBtn == btnIdx);

        if (mHasBtnTex) {
            sf::Sprite btnSprite(mBtnTexture);
            auto btnSize = mBtnTexture.getSize();
            btnSprite.setOrigin({btnSize.x / 2.f, btnSize.y / 2.f});
            btnSprite.setScale({140.f / btnSize.x, 40.f / btnSize.y});
            btnSprite.setPosition({x + 70.f, y + 20.f});
            btnSprite.setColor(hovered ? sf::Color(255, 255, 180) : sf::Color(150, 150, 150));
            window.draw(btnSprite);
        }

        sf::Text text(mFont, label, 18);
        text.setFillColor(hovered ? sf::Color::Yellow : sf::Color::White);
        auto bounds = text.getLocalBounds();
        text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
        text.setPosition({x + 70.f, y + 18.f});
        window.draw(text);
    };

    drawNavButton(120.f, 540.f, "< PREVIOUS", mCurrentPage > 0, 0);
    drawNavButton(330.f, 540.f, "BACK", true, 2);
    drawNavButton(540.f, 540.f, "NEXT >", mCurrentPage < mTotalPages - 1, 1);
}
