#include "states/VictoryState.h"
#include "states/StateManager.h"
#include "states/MainMenuState.h"
#include "core/AudioManager.h"

VictoryState::VictoryState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const GameData& gameData)
    : State(stateManager), mWindow(window), mFont(font), mGameData(gameData)
{
    if (std::filesystem::exists("assets/backgrounds/victory.png")) {
        if (mBgTexture.loadFromFile("assets/backgrounds/victory.png")) {
            mBgSprite = std::make_unique<sf::Sprite>(mBgTexture);
            auto size = mBgTexture.getSize();
            mBgSprite->setScale({800.f / size.x, 600.f / size.y});
            mHasBg = true;
        }
    }
    AudioManager::instance().playVictoryMusic();
}

void VictoryState::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) 
        if (keyPressed->code == sf::Keyboard::Key::Enter || keyPressed->code == sf::Keyboard::Key::Escape) 
            mStateManager.changeState(std::make_unique<MainMenuState>(mStateManager, mWindow, mFont, mGameData), StateID::MainMenu);
}

void VictoryState::update(float dt) { mTimer += dt; }

void VictoryState::draw(sf::RenderWindow& window) {
    window.setView(window.getDefaultView());

    if (mHasBg && mBgSprite) window.draw(*mBgSprite); 
    else {
        sf::RectangleShape bg({800.f, 600.f});
        bg.setFillColor(sf::Color(15, 15, 30));
        window.draw(bg);
    }

    // Golden border
    sf::RectangleShape border({780.f, 580.f});
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(4.f);
    border.setOutlineColor(sf::Color(255, 215, 0));
    border.setPosition({10.f, 10.f});
    window.draw(border);

    // Victory Crown decorative circles
    for (int i = 0; i < 5; i++) {
        sf::CircleShape star(8.f);
        star.setOrigin({8.f, 8.f});
        star.setFillColor(sf::Color(255, 215, 0));
        star.setPosition({300.f + i * 50.f, 120.f});
        window.draw(star);
    }

    // Flashing prompt
    if (static_cast<int>(mTimer * 2.f) % 2 == 0) {
        sf::Text prompt(mFont, "Press ENTER to return to Main Menu", 18);
        prompt.setFillColor(sf::Color(200, 200, 200));
        auto pb = prompt.getLocalBounds();
        prompt.setOrigin({pb.size.x / 2.f, pb.size.y / 2.f});
        prompt.setPosition({400.f, 440.f});
        window.draw(prompt);
    }
}
