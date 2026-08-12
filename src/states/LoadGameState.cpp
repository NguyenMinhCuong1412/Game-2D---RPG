#include "states/LoadGameState.h"
#include "states/StateManager.h"
#include "states/PlayingState.h"
#include "character/CharacterUtil.h"
#include "core/AudioManager.h"

LoadGameState::LoadGameState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData)
    : State(stateManager)
    , mWindow(window)
    , mFont(font)
    , mGameData(gameData)
    , mTitleText(font)
    , mBackText(font)
{
    std::string btnPath = mGameData.getAssetPath("ui_button");
    if (!btnPath.empty() && std::filesystem::exists(btnPath)) {
        if (mBtnTexture.loadFromFile(btnPath)) {
            mHasBtnTex = true;
        }
    }

    mTitleText.setString("SELECT SAVE SLOT TO LOAD");
    mTitleText.setCharacterSize(36);
    mTitleText.setFillColor(sf::Color(255, 215, 0)); // Gold color
    mTitleText.setOutlineThickness(2.f);
    mTitleText.setOutlineColor(sf::Color::Black);

    auto titleBounds = mTitleText.getLocalBounds();
    mTitleText.setOrigin({ titleBounds.position.x + titleBounds.size.x / 2.f, titleBounds.position.y + titleBounds.size.y / 2.f });
    mTitleText.setPosition({ 400.f, 70.f });

    mBackText.setString("BACK");
    mBackText.setCharacterSize(20);
    mBackText.setFillColor(sf::Color::White);
    auto backBounds = mBackText.getLocalBounds();
    mBackText.setOrigin({backBounds.position.x + backBounds.size.x / 2.f, backBounds.position.y + backBounds.size.y / 2.f});
    mBackText.setPosition({400.f, 525.f});

    if (mHasBtnTex) {
        mBackBtnSprite = std::make_unique<sf::Sprite>(mBtnTexture);
        auto btnSize = mBtnTexture.getSize();
        mBackBtnSprite->setOrigin({btnSize.x / 2.f, btnSize.y / 2.f});
        mBackBtnSprite->setScale({180.f / btnSize.x, 45.f / btnSize.y});
        mBackBtnSprite->setPosition({400.f, 525.f});
    }

    refreshSlots();
}

void LoadGameState::refreshSlots() {
    mSaveSlots = SaveManager::loadAllSlots();
    mSlotTexts.clear();
    mBtnSprites.clear();

    // 2 Vertical Columns: Slots 1-3 on Left, Slots 4-6 on Right
    for (int i = 0; i < SaveManager::MAX_SLOTS; i++) {
        sf::Text text(mFont);
        std::string desc = "Slot " + std::to_string(i + 1) + ": ";
        if (mSaveSlots[i].isValid) {
            std::string className = getCharacterClassName(mSaveSlots[i].characterType);
            desc += className + "\nLVL " + std::to_string(mSaveSlots[i].level) + " (" + std::to_string(mSaveSlots[i].gold) + "g)";
        } else {
            desc += "[Empty]";
        }

        text.setString(desc);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::White);

        int col = i / 3; // 0 or 1
        int row = i % 3; // 0, 1, 2

        float posX = (col == 0) ? 240.f : 560.f;
        float posY = 180.f + row * 125.f;

        auto bounds = text.getLocalBounds();
        text.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
        text.setPosition({posX, posY});
        mSlotTexts.push_back(text);

        if (mHasBtnTex) {
            auto btnSprite = std::make_unique<sf::Sprite>(mBtnTexture);
            auto btnSize = mBtnTexture.getSize();
            btnSprite->setOrigin({btnSize.x / 2.f, btnSize.y / 2.f});
            btnSprite->setScale({280.f / btnSize.x, 90.f / btnSize.y});
            btnSprite->setPosition({posX, posY});
            mBtnSprites.push_back(std::move(btnSprite));
        }
    }

    updateSelection();
}

void LoadGameState::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (mShowSlotMenu) {
            // Submenu active: Play / Delete
            if (keyPressed->code == sf::Keyboard::Key::Up ||
                keyPressed->code == sf::Keyboard::Key::W) {
                mSlotMenuSelection = 0; // Play
            }
            else if (keyPressed->code == sf::Keyboard::Key::Down ||
                     keyPressed->code == sf::Keyboard::Key::S) {
                mSlotMenuSelection = 1; // Delete
            }
            else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                if (mSlotMenuSelection == 0) {
                    // Play
                    if (mSaveSlots[mSelectedIdx].isValid) {
                        mStateManager.changeState(
                            std::make_unique<PlayingState>(
                                mStateManager,
                                mWindow,
                                mFont,
                                mSaveSlots[mSelectedIdx],
                                mGameData
                            ),
                            StateID::Playing
                        );
                    }
                }
                else {
                    // Delete
                    SaveManager::deleteSlot(static_cast<int>(mSelectedIdx));
                    mShowSlotMenu = false;
                    mSlotMenuSelection = 0;
                    refreshSlots();
                }
            }
            else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                mShowSlotMenu = false;
                mSlotMenuSelection = 0;
            }

            return;
        }

        // Normal slot selection
        if (keyPressed->code == sf::Keyboard::Key::Up ||
            keyPressed->code == sf::Keyboard::Key::W) {
            if (mSelectedIdx >= 1) {
                mSelectedIdx--;
                updateSelection();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Down ||
                 keyPressed->code == sf::Keyboard::Key::S) {
            if (mSelectedIdx < mSlotTexts.size() - 1) {
                mSelectedIdx++;
                updateSelection();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Left ||
                 keyPressed->code == sf::Keyboard::Key::A) {
            if (mSelectedIdx >= 3) {
                mSelectedIdx -= 3;
                updateSelection();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Right ||
                 keyPressed->code == sf::Keyboard::Key::D) {
            if (mSelectedIdx + 3 < mSlotTexts.size()) {
                mSelectedIdx += 3;
                updateSelection();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Enter) {
            if (mSaveSlots[mSelectedIdx].isValid) {
                // Show Play/Delete submenu
                mShowSlotMenu = true;
                mSlotMenuSelection = 0;
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Escape) {
            mStateManager.popState();
        }
    }

    // ============================================================
    // MOUSE HOVER
    // ============================================================
    else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        if (!mShowSlotMenu) {
            sf::Vector2f mousePos =
                mWindow.mapPixelToCoords(mouseMoved->position);

            bool wasBackHovered = mBackHovered;
            mBackHovered = sf::FloatRect({310.f, 502.5f}, {180.f, 45.f}).contains(mousePos);

            bool anySlotHovered = false;
            for (std::size_t i = 0; i < mSlotTexts.size(); ++i) {
                int col = static_cast<int>(i / 3);
                int row = static_cast<int>(i % 3);

                float posX = (col == 0) ? 240.f : 560.f;
                float posY = 180.f + row * 125.f;

                sf::FloatRect bounds(
                    {posX - 140.f, posY - 45.f},
                    {280.f, 90.f}
                );

                if (bounds.contains(mousePos)) {
                    anySlotHovered = true;
                    if (mSelectedIdx != i) {
                        mSelectedIdx = i;
                    }
                }
            }
            mSlotHovered = anySlotHovered;
            updateSelection();
        }
        else {
            sf::Vector2f mousePos =
                mWindow.mapPixelToCoords(mouseMoved->position);

            // PLAY
            if (mousePos.x >= 290.f &&
                mousePos.x <= 510.f &&
                mousePos.y >= 245.f &&
                mousePos.y <= 295.f) {
                mSlotMenuSelection = 0;
            }
            // DELETE
            else if (mousePos.x >= 290.f &&
                     mousePos.x <= 510.f &&
                     mousePos.y >= 305.f &&
                     mousePos.y <= 355.f) {
                mSlotMenuSelection = 1;
            }
        }
    }

    // ============================================================
    // MOUSE CLICK
    // ============================================================
    else if (const auto* mouseBtn =
                 event.getIf<sf::Event::MouseButtonPressed>()) {

        if (mouseBtn->button == sf::Mouse::Button::Left) {

            sf::Vector2f mousePos =
                mWindow.mapPixelToCoords(mouseBtn->position);

            // ----------------------------------------------------
            // PLAY / DELETE SUBMENU
            // ----------------------------------------------------
            if (mShowSlotMenu) {

                // PLAY
                if (mousePos.x >= 290.f &&
                    mousePos.x <= 510.f &&
                    mousePos.y >= 245.f &&
                    mousePos.y <= 295.f) {

                    mSlotMenuSelection = 0;

                    if (mSaveSlots[mSelectedIdx].isValid) {
                        mStateManager.changeState(
                            std::make_unique<PlayingState>(
                                mStateManager,
                                mWindow,
                                mFont,
                                mSaveSlots[mSelectedIdx],
                                mGameData
                            ),
                            StateID::Playing
                        );
                    }
                }

                // DELETE
                else if (mousePos.x >= 290.f &&
                         mousePos.x <= 510.f &&
                         mousePos.y >= 305.f &&
                         mousePos.y <= 355.f) {

                    mSlotMenuSelection = 1;

                    SaveManager::deleteSlot(
                        static_cast<int>(mSelectedIdx)
                    );

                    mShowSlotMenu = false;
                    mSlotMenuSelection = 0;
                    refreshSlots();
                }

                return;
            }

            // ----------------------------------------------------
            // NORMAL SAVE SLOT CLICK
            // ----------------------------------------------------
            if (!mShowSlotMenu) {
                // Check BACK button click
                if (sf::FloatRect({310.f, 502.5f}, {180.f, 45.f}).contains(mousePos)) {
                    AudioManager::instance().playSfx("button");
                    mStateManager.popState();
                    return;
                }

                for (std::size_t i = 0; i < mSlotTexts.size(); ++i) {
                    int col = static_cast<int>(i / 3);
                    int row = static_cast<int>(i % 3);

                    float posX = (col == 0) ? 240.f : 560.f;
                    float posY = 180.f + row * 125.f;

                    sf::FloatRect bounds(
                        {posX - 140.f, posY - 45.f},
                        {280.f, 90.f}
                    );

                    if (bounds.contains(mousePos)) {
                        mSelectedIdx = i;

                        if (mSaveSlots[mSelectedIdx].isValid) {
                            mShowSlotMenu = true;
                            mSlotMenuSelection = 0;
                        }

                        break;
                    }
                }
            }
        }
    }
}

void LoadGameState::update(float dt) {
    (void)dt;
}

void LoadGameState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(15, 20, 30));

    for (std::size_t i = 0; i < mSlotTexts.size(); ++i) {
        if (mHasBtnTex && i < mBtnSprites.size()) {
            if (i == mSelectedIdx && mSlotHovered && !mBackHovered) {
                mBtnSprites[i]->setColor(sf::Color(255, 255, 180));
            } else {
                mBtnSprites[i]->setColor(sf::Color(180, 180, 180));
            }
            window.draw(*mBtnSprites[i]);
        }
        window.draw(mSlotTexts[i]);
    }

    // Draw BACK button
    if (mHasBtnTex && mBackBtnSprite) {
        mBackBtnSprite->setColor(mBackHovered ? sf::Color(255, 255, 180) : sf::Color(180, 180, 180));
        window.draw(*mBackBtnSprite);
    }
    mBackText.setFillColor(mBackHovered ? sf::Color::Yellow : sf::Color::White);
    window.draw(mBackText);

    window.draw(mTitleText);

    // Draw Play/Delete submenu overlay if active
    if (mShowSlotMenu) {
        // Semi-transparent overlay
        sf::RectangleShape overlay({800.f, 600.f});
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);

        // Submenu box
        sf::RectangleShape menuBg({220.f, 130.f});
        menuBg.setFillColor(sf::Color(30, 30, 50, 240));
        menuBg.setOutlineThickness(2.f);
        menuBg.setOutlineColor(sf::Color(255, 215, 0));
        menuBg.setPosition({290.f, 235.f});
        window.draw(menuBg);

        // Play option
        sf::Text playText(mFont, "PLAY", 24);
        playText.setFillColor(mSlotMenuSelection == 0 ? sf::Color::Yellow : sf::Color::White);
        auto pb = playText.getLocalBounds();
        playText.setOrigin({pb.size.x / 2.f, pb.size.y / 2.f});
        playText.setPosition({400.f, 270.f});
        window.draw(playText);

        // Delete option
        sf::Text deleteText(mFont, "DELETE", 24);
        deleteText.setFillColor(mSlotMenuSelection == 1 ? sf::Color(255, 80, 80) : sf::Color::White);
        auto db = deleteText.getLocalBounds();
        deleteText.setOrigin({db.size.x / 2.f, db.size.y / 2.f});
        deleteText.setPosition({400.f, 330.f});
        window.draw(deleteText);
    }
}

void LoadGameState::updateSelection() {
    for (std::size_t i = 0; i < mSlotTexts.size(); ++i) {
        if (i == mSelectedIdx && mSlotHovered && !mBackHovered) {
            mSlotTexts[i].setFillColor(sf::Color::Yellow);
            mSlotTexts[i].setScale({1.05f, 1.05f});
        } else {
            mSlotTexts[i].setFillColor(sf::Color::White);
            mSlotTexts[i].setScale({1.0f, 1.0f});
        }
        auto bounds = mSlotTexts[i].getLocalBounds();
        mSlotTexts[i].setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
    }
}
