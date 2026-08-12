#include "states/SettingsState.h"
#include "states/StateManager.h"
#include "database/GameData.h"
#include "core/AudioManager.h"

SettingsState::SettingsState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData)
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

    // Load current settings from AudioManager
    mMasterVolume = AudioManager::instance().getMasterVolume();
    mMusicVolume = AudioManager::instance().getMusicVolume();
    mSfxVolume = AudioManager::instance().getSfxVolume();
    mFullscreen = AudioManager::instance().isFullscreen();
    mDamageNumbers = AudioManager::instance().isDamageNumbersEnabled();
}

void SettingsState::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Escape) {
            mStateManager.popState();
        }
        else if (keyPressed->code == sf::Keyboard::Key::Up || keyPressed->code == sf::Keyboard::Key::W) {
            mAnyHovered = true;
            mSelectedIdx = (mSelectedIdx - 1 + ITEM_COUNT) % ITEM_COUNT;
        }
        else if (keyPressed->code == sf::Keyboard::Key::Down || keyPressed->code == sf::Keyboard::Key::S) {
            mAnyHovered = true;
            mSelectedIdx = (mSelectedIdx + 1) % ITEM_COUNT;
        }
        else if (keyPressed->code == sf::Keyboard::Key::Left || keyPressed->code == sf::Keyboard::Key::A) {
            // Decrease slider value
            if (mSelectedIdx == 0) mMasterVolume = std::max(0.f, mMasterVolume - 5.f);
            else if (mSelectedIdx == 1) mMusicVolume = std::max(0.f, mMusicVolume - 5.f);
            else if (mSelectedIdx == 2) mSfxVolume = std::max(0.f, mSfxVolume - 5.f);
            else if (mSelectedIdx == 3) mFullscreen = !mFullscreen;
            else if (mSelectedIdx == 4) mDamageNumbers = !mDamageNumbers;
        }
        else if (keyPressed->code == sf::Keyboard::Key::Right || keyPressed->code == sf::Keyboard::Key::D) {
            // Increase slider value
            if (mSelectedIdx == 0) mMasterVolume = std::min(100.f, mMasterVolume + 5.f);
            else if (mSelectedIdx == 1) mMusicVolume = std::min(100.f, mMusicVolume + 5.f);
            else if (mSelectedIdx == 2) mSfxVolume = std::min(100.f, mSfxVolume + 5.f);
            else if (mSelectedIdx == 3) mFullscreen = !mFullscreen;
            else if (mSelectedIdx == 4) mDamageNumbers = !mDamageNumbers;
        }
        else if (keyPressed->code == sf::Keyboard::Key::Enter) {
            AudioManager::instance().playSfx("button");
            if (mSelectedIdx == 3) mFullscreen = !mFullscreen;
            else if (mSelectedIdx == 4) mDamageNumbers = !mDamageNumbers;
            else if (mSelectedIdx == 5) {
                // APPLY
                applySettings();
            }
            else if (mSelectedIdx == 6) {
                // BACK
                mStateManager.popState();
            }
        }
    }
    else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos = mWindow.mapPixelToCoords(mouseMoved->position);

        // Check which item is hovered
        bool anyHovered = false;
        for (int i = 0; i < ITEM_COUNT; ++i) {
            float itemY = 160.f + i * 55.f;
            sf::FloatRect bounds({200.f, itemY - 15.f}, {400.f, 40.f});
            if (bounds.contains(mousePos)) {
                anyHovered = true;
                mSelectedIdx = i;
            }
        }
        mAnyHovered = anyHovered;

        // Handle slider dragging
        if (mDraggingSlider && mDragSliderIdx >= 0 && mDragSliderIdx <= 2) {
            float sliderLeft = 410.f;
            float sliderRight = 580.f;
            float val = (mousePos.x - sliderLeft) / (sliderRight - sliderLeft) * 100.f;
            val = std::clamp(val, 0.f, 100.f);
            if (mDragSliderIdx == 0) mMasterVolume = val;
            else if (mDragSliderIdx == 1) mMusicVolume = val;
            else if (mDragSliderIdx == 2) mSfxVolume = val;
        }
    }
    else if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos = mWindow.mapPixelToCoords(mouseBtn->position);

            // Check slider click
            for (int i = 0; i < 3; ++i) {
                float itemY = 160.f + i * 55.f;
                sf::FloatRect sliderBounds({405.f, itemY - 8.f}, {180.f, 20.f});
                if (sliderBounds.contains(mousePos)) {
                    mDraggingSlider = true;
                    mDragSliderIdx = i;
                    mSelectedIdx = i;
                    float sliderLeft = 410.f;
                    float sliderRight = 580.f;
                    float val = (mousePos.x - sliderLeft) / (sliderRight - sliderLeft) * 100.f;
                    val = std::clamp(val, 0.f, 100.f);
                    if (i == 0) mMasterVolume = val;
                    else if (i == 1) mMusicVolume = val;
                    else if (i == 2) mSfxVolume = val;
                }
            }

            // Toggle buttons
            float fsY = 160.f + 3 * 55.f;
            sf::FloatRect fsBounds({200.f, fsY - 15.f}, {400.f, 40.f});
            if (fsBounds.contains(mousePos)) {
                mFullscreen = !mFullscreen;
                mSelectedIdx = 3;
            }

            float dnY = 160.f + 4 * 55.f;
            sf::FloatRect dnBounds({200.f, dnY - 15.f}, {400.f, 40.f});
            if (dnBounds.contains(mousePos)) {
                mDamageNumbers = !mDamageNumbers;
                mSelectedIdx = 4;
            }

            // APPLY button
            float applyY = 160.f + 5 * 55.f;
            sf::FloatRect applyBounds({300.f, applyY - 15.f}, {200.f, 40.f});
            if (applyBounds.contains(mousePos)) {
                AudioManager::instance().playSfx("button");
                applySettings();
            }

            // BACK button
            float backY = 160.f + 6 * 55.f;
            sf::FloatRect backBounds({300.f, backY - 15.f}, {200.f, 40.f});
            if (backBounds.contains(mousePos)) {
                AudioManager::instance().playSfx("button");
                mStateManager.popState();
            }
        }
    }
    else if (const auto* mouseRel = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseRel->button == sf::Mouse::Button::Left) {
            mDraggingSlider = false;
            mDragSliderIdx = -1;
        }
    }
}

void SettingsState::applySettings() {
    AudioManager::instance().setMasterVolume(mMasterVolume);
    AudioManager::instance().setMusicVolume(mMusicVolume);
    AudioManager::instance().setSfxVolume(mSfxVolume);
    AudioManager::instance().setDamageNumbersEnabled(mDamageNumbers);

    bool currentlyFullscreen = AudioManager::instance().isFullscreen();
    AudioManager::instance().setFullscreen(mFullscreen);

    // Fullscreen toggle: recreate window if state changed
    if (mFullscreen != currentlyFullscreen) {
        if (mFullscreen) {
            mWindow.create(sf::VideoMode::getDesktopMode(), "Legend of the Realm", sf::State::Fullscreen);
        } else {
            mWindow.create(sf::VideoMode({800, 600}), "Legend of the Realm", sf::State::Windowed);
        }
        mWindow.setFramerateLimit(60);
        sf::View view({400.f, 300.f}, {800.f, 600.f});
        mWindow.setView(view);
    }
}

void SettingsState::update(float dt) {
    (void)dt;
}

void SettingsState::draw(sf::RenderWindow& window) {
    // Background
    if (mHasBg && mBgSprite) {
        mBgSprite->setColor(sf::Color(80, 80, 80));
        window.draw(*mBgSprite);
        mBgSprite->setColor(sf::Color::White);
    } else {
        window.clear(sf::Color(15, 15, 25));
    }

    // Panel
    sf::RectangleShape panelBg({500.f, 450.f});
    panelBg.setFillColor(sf::Color(10, 10, 20, 230));
    panelBg.setOutlineThickness(2.f);
    panelBg.setOutlineColor(sf::Color(255, 215, 0, 180));
    panelBg.setPosition({150.f, 75.f});
    window.draw(panelBg);

    // Title
    sf::Text titleText(mFont, "SETTING", 32);
    titleText.setFillColor(sf::Color(255, 215, 0));
    titleText.setOutlineThickness(2.f);
    titleText.setOutlineColor(sf::Color::Black);
    auto tb = titleText.getLocalBounds();
    titleText.setOrigin({tb.size.x / 2.f, tb.size.y / 2.f});
    titleText.setPosition({400.f, 110.f});
    window.draw(titleText);

    // Settings items
    std::string labels[] = {
        "MASTER VOLUME",
        "MUSIC VOLUME",
        "SFX VOLUME",
        "FULLSCREEN",
        "DAMAGE NUMBERS",
        "APPLY",
        "BACK"
    };

    for (int i = 0; i < ITEM_COUNT; ++i) {
        float itemY = 160.f + i * 55.f;
        bool selected = (i == mSelectedIdx);

        // Label
        sf::Text label(mFont, labels[i], 18);
        label.setFillColor(selected && mAnyHovered ? sf::Color::Yellow : sf::Color(200, 200, 200));
        label.setPosition({220.f, itemY - 8.f});

        if (i < 3) {
            // Volume slider
            window.draw(label);

            float val = (i == 0) ? mMasterVolume : (i == 1) ? mMusicVolume : mSfxVolume;
            float sliderLeft = 410.f;
            float sliderWidth = 170.f;

            // Slider track
            sf::RectangleShape track({sliderWidth, 6.f});
            track.setFillColor(sf::Color(50, 50, 60));
            track.setPosition({sliderLeft, itemY});
            window.draw(track);

            // Slider fill
            float fillWidth = (val / 100.f) * sliderWidth;
            sf::RectangleShape fill({fillWidth, 6.f});
            fill.setFillColor(selected && mAnyHovered ? sf::Color(255, 215, 0) : sf::Color(180, 140, 50));
            fill.setPosition({sliderLeft, itemY});
            window.draw(fill);

            // Slider knob
            sf::CircleShape knob(7.f);
            knob.setOrigin({7.f, 7.f});
            knob.setPosition({sliderLeft + fillWidth, itemY + 3.f});
            knob.setFillColor(selected && mAnyHovered ? sf::Color::Yellow : sf::Color::White);
            window.draw(knob);

            // Value text
            sf::Text valText(mFont, std::to_string(static_cast<int>(val)), 14);
            valText.setFillColor(sf::Color::White);
            valText.setPosition({sliderLeft + sliderWidth + 10.f, itemY - 6.f});
            window.draw(valText);
        }
        else if (i == 3 || i == 4) {
            // Toggle
            window.draw(label);

            bool val = (i == 3) ? mFullscreen : mDamageNumbers;
            std::string toggleStr = val ? "ON" : "OFF";

            sf::Text toggleText(mFont, toggleStr, 18);
            toggleText.setFillColor(val ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
            toggleText.setPosition({530.f, itemY - 8.f});
            window.draw(toggleText);

            // Toggle brackets
            sf::Text openBracket(mFont, "[", 18);
            openBracket.setFillColor(selected && mAnyHovered ? sf::Color::Yellow : sf::Color(150, 150, 150));
            openBracket.setPosition({510.f, itemY - 8.f});
            window.draw(openBracket);

            sf::Text closeBracket(mFont, "]", 18);
            closeBracket.setFillColor(selected && mAnyHovered ? sf::Color::Yellow : sf::Color(150, 150, 150));
            float closeX = 530.f + (toggleText.findCharacterPos(toggleStr.size()).x - toggleText.findCharacterPos(0).x) + 8.f;
            closeBracket.setPosition({closeX, itemY - 8.f});
            window.draw(closeBracket);
        }
        else {
            // APPLY / BACK buttons
            if (mHasBtnTex) {
                sf::Sprite btnSprite(mBtnTexture);
                auto btnSize = mBtnTexture.getSize();
                btnSprite.setOrigin({btnSize.x / 2.f, btnSize.y / 2.f});
                btnSprite.setScale({180.f / btnSize.x, 45.f / btnSize.y});
                btnSprite.setPosition({400.f, itemY});
                btnSprite.setColor(selected && mAnyHovered ? sf::Color(255, 255, 180) : sf::Color(150, 150, 150));
                window.draw(btnSprite);
            }

            label.setCharacterSize(20);
            auto lb = label.getLocalBounds();
            label.setOrigin({lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f});
            label.setPosition({400.f, itemY});
            label.setFillColor(selected && mAnyHovered ? sf::Color::Yellow : sf::Color::White);
            window.draw(label);
        }
    }
}
