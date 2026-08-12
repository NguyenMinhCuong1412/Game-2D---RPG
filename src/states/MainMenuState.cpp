#include "states/MainMenuState.h"
#include "database/GameData.h"
#include "states/StateManager.h"
#include "states/CharacterSelectionState.h"
#include "states/LoadGameState.h"
#include "states/HowToPlayState.h"
#include "states/SettingsState.h"
#include "core/AudioManager.h"

MainMenuState::MainMenuState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData)
    : State(stateManager)
    , mWindow(window)
    , mFont(font)
    , mGameData(gameData)
    , mTitleText(font)
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


    // Title formatted into 3 lines, bigger font size, centered in the left frame
    mTitleText.setString("LEGEND OF\n   THE REALM");
    mTitleText.setCharacterSize(36);
    mTitleText.setFillColor(sf::Color(255, 215, 0)); // Gold color
    mTitleText.setOutlineThickness(2.5f);
    mTitleText.setOutlineColor(sf::Color::Black);

    auto titleBounds = mTitleText.getLocalBounds();
    mTitleText.setOrigin({titleBounds.size.x / 2.f, titleBounds.size.y / 2.f});
    mTitleText.setPosition({145.f, 310.f}); // Centered in left frame

    mOptionStrings = { "Play", "Continue", "How to play", "Setting", "Quit" };

    // Buttons positioned on the FAR RIGHT side clear of the skull
    float btnX = 680.f;
    float startY = 150.f;
    float spacing = 80.f;

    for (std::size_t i = 0; i < mOptionStrings.size(); ++i) {
        sf::Text text(mFont);
        text.setString(mOptionStrings[i]);
        text.setCharacterSize(22);
        text.setFillColor(sf::Color::White);

        float posY = startY + i * spacing;
        auto bounds = text.getLocalBounds();
        text.setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
        text.setPosition({ btnX, posY });
        mOptions.push_back(text);

        if (mHasBtnTex) {
            auto btnSprite = std::make_unique<sf::Sprite>(mBtnTexture);
            auto btnSize = mBtnTexture.getSize();
            btnSprite->setOrigin({btnSize.x / 2.f, btnSize.y / 2.f});
            btnSprite->setScale({160.f / btnSize.x, 55.f / btnSize.y});
            btnSprite->setPosition({btnX, posY});
            mBtnSprites.push_back(std::move(btnSprite));
        }
    }

    updateSelection();
    AudioManager::instance().playMenuMusic();
}

void MainMenuState::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Up || keyPressed->code == sf::Keyboard::Key::W) {
            mAnyHovered = true;
            if (mSelectedIdx >= 1) {
                mSelectedIdx--;
                updateSelection();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Down || keyPressed->code == sf::Keyboard::Key::S) {
            mAnyHovered = true;
            if (mSelectedIdx < mOptions.size() - 1) {
                mSelectedIdx++;
                updateSelection();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Enter) {
            AudioManager::instance().playSfx("button");
            if (mSelectedIdx == 0) {
                // Play -> Character Selection
                mStateManager.pushState(std::make_unique<CharacterSelectionState>(mStateManager, mWindow, mFont, mGameData), StateID::CharacterSelection);
            }
            else if (mSelectedIdx == 1) {
                // Continue -> LoadGameState
                mStateManager.pushState(std::make_unique<LoadGameState>(mStateManager, mWindow, mFont, mGameData), StateID::LoadGame);
            }
            else if (mSelectedIdx == 2) {
                // How To Play
                mStateManager.pushState(std::make_unique<HowToPlayState>(mStateManager, mWindow, mFont, mGameData), StateID::HowToPlay);
            }
            else if (mSelectedIdx == 3) {
                // Settings
                mStateManager.pushState(std::make_unique<SettingsState>(mStateManager, mWindow, mFont, mGameData), StateID::Settings);
            }
            else if (mSelectedIdx == 4) {
                // Quit -> Close Window
                mWindow.close();
            }
        }
    }
    else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos = mWindow.mapPixelToCoords(mouseMoved->position);
        float btnX = 650.f;
        float startY = 150.f;
        float spacing = 80.f;

        bool anyHovered = false;
        for (std::size_t i = 0; i < mOptions.size(); ++i) {
            float posY = startY + i * spacing;
            sf::FloatRect bounds({ btnX - 80.f, posY - 27.5f }, { 160.f, 55.f });
            if (bounds.contains(mousePos)) {
                anyHovered = true;
                if (mSelectedIdx != i) {
                    mSelectedIdx = i;
                }
            }
        }
        mAnyHovered = anyHovered;
        updateSelection();
    }
    else if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos = mWindow.mapPixelToCoords(mouseBtn->position);
            float btnX = 650.f;
            float startY = 150.f;
            float spacing = 80.f;

            for (std::size_t i = 0; i < mOptions.size(); ++i) {
                float posY = startY + i * spacing;
                sf::FloatRect bounds({ btnX - 80.f, posY - 27.5f }, { 160.f, 55.f });
                if (bounds.contains(mousePos)) {
                    AudioManager::instance().playSfx("button");
                    mSelectedIdx = i;
                    if (mSelectedIdx == 0) {
                        mStateManager.pushState(std::make_unique<CharacterSelectionState>(mStateManager, mWindow, mFont, mGameData), StateID::CharacterSelection);
                    }
                    else if (mSelectedIdx == 1) {
                        mStateManager.pushState(std::make_unique<LoadGameState>(mStateManager, mWindow, mFont, mGameData), StateID::LoadGame);
                    }
                    else if (mSelectedIdx == 2) {
                        mStateManager.pushState(std::make_unique<HowToPlayState>(mStateManager, mWindow, mFont, mGameData), StateID::HowToPlay);
                    }
                    else if (mSelectedIdx == 3) {
                        mStateManager.pushState(std::make_unique<SettingsState>(mStateManager, mWindow, mFont, mGameData), StateID::Settings);
                    }
                    else if (mSelectedIdx == 4) {
                        mWindow.close();
                    }
                }
            }
        }
    }
}

void MainMenuState::update(float dt) {
    (void)dt;
}

void MainMenuState::draw(sf::RenderWindow& window) {
    if (mHasBg && mBgSprite) {
        window.draw(*mBgSprite);
    } else {
        window.clear(sf::Color(20, 20, 30));
    }

    for (std::size_t i = 0; i < mOptions.size(); ++i) {
        if (mHasBtnTex && i < mBtnSprites.size()) {
            if (i == mSelectedIdx && mAnyHovered) {
                mBtnSprites[i]->setColor(sf::Color(255, 255, 180));
            } else {
                mBtnSprites[i]->setColor(sf::Color(180, 180, 180));
            }
            window.draw(*mBtnSprites[i]);
        }
        window.draw(mOptions[i]);
    }

    window.draw(mTitleText);
}

void MainMenuState::updateSelection() {
    for (std::size_t i = 0; i < mOptions.size(); ++i) {
        if (i == mSelectedIdx && mAnyHovered) {
            mOptions[i].setFillColor(sf::Color::Yellow);
            mOptions[i].setScale({1.1f, 1.1f});
        } else {
            mOptions[i].setFillColor(sf::Color::White);
            mOptions[i].setScale({1.0f, 1.0f});
        }
        auto bounds = mOptions[i].getLocalBounds();
        mOptions[i].setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
    }
}
