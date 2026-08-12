#include "states/GameOverState.h"
#include "states/StateManager.h"
#include "states/MainMenuState.h"
#include "core/AudioManager.h"
#include "database/GameData.h"

GameOverState::GameOverState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData)
    : State(stateManager)
    , mWindow(window)
    , mFont(font)
    , mGameData(gameData)
{
    if (const sf::Texture* preloadedTexture = mGameData.getDefeatTexture()) {
        mBgSprite = std::make_unique<sf::Sprite>(*preloadedTexture);
        auto size = preloadedTexture->getSize();
        mBgSprite->setScale({800.f / size.x, 600.f / size.y});
        mHasBg = true;
    }
    AudioManager::instance().playDefeatMusic();
}

void GameOverState::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Enter || keyPressed->code == sf::Keyboard::Key::Escape) {
            mStateManager.changeState(std::make_unique<MainMenuState>(mStateManager, mWindow, mFont, mGameData), StateID::MainMenu);
        }
    }
}

void GameOverState::update(float dt) {
    mTimer += dt;
}

void GameOverState::draw(sf::RenderWindow& window) {
    window.setView(window.getDefaultView());

    if (mHasBg && mBgSprite) {
        window.draw(*mBgSprite);
    } else {
        sf::RectangleShape bg({800.f, 600.f});
        bg.setFillColor(sf::Color(40, 5, 5));
        window.draw(bg);
    }

    // Skull/death vignette border
    sf::RectangleShape border({780.f, 580.f});
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(4.f);
    border.setOutlineColor(sf::Color(120, 10, 10));
    border.setPosition({10.f, 10.f});
    window.draw(border);

    // Flashing prompt
    if (static_cast<int>(mTimer * 2.f) % 2 == 0) {
        sf::Text prompt(mFont, "Press ENTER to return to Main Menu", 18);
        prompt.setFillColor(sf::Color(200, 200, 200));
        auto pb = prompt.getLocalBounds();
        prompt.setOrigin({pb.size.x / 2.f, pb.size.y / 2.f});
        prompt.setPosition({400.f, 450.f}); // Adjusted position below DEFEAT banner
        window.draw(prompt);
    }
}
