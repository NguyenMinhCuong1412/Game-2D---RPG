#include "world/Map.h"

Map::Map() {
    float halfW = mWidth / 2.f;
    float halfH = mHeight / 2.f;

    // NorthWest: Top-Left (Forest Green)
    mNorthWestRect.setSize({halfW, halfH});
    mNorthWestRect.setPosition({0.f, 0.f});
    mNorthWestRect.setFillColor(sf::Color(60, 100, 60));

    if (fs::exists("assets/textures/map/northwest/tileset.png")) {
        if (mNorthWestTex.loadFromFile("assets/textures/map/northwest/tileset.png")) {
            mNorthWestTex.setRepeated(true);
            mNorthWestSprite = std::make_unique<sf::Sprite>(mNorthWestTex);
            mNorthWestSprite->setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(halfW), static_cast<int>(halfH)}));
            mNorthWestSprite->setPosition({0.f, 0.f});
            mHasNorthWestTex = true;
        }
    }

    // NorthEast: Top-Right (Olive Green)
    mNorthEastRect.setSize({halfW, halfH});
    mNorthEastRect.setPosition({halfW, 0.f});
    mNorthEastRect.setFillColor(sf::Color(70, 110, 50));

    if (fs::exists("assets/textures/map/northeast/tileset.png")) {
        if (mNorthEastTex.loadFromFile("assets/textures/map/northeast/tileset.png")) {
            mNorthEastTex.setRepeated(true);
            mNorthEastSprite = std::make_unique<sf::Sprite>(mNorthEastTex);
            mNorthEastSprite->setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(halfW), static_cast<int>(halfH)}));
            mNorthEastSprite->setPosition({halfW, 0.f});
            mHasNorthEastTex = true;
        }
    }

    // SouthWest: Bottom-Left (Swamp Green)
    mSouthWestRect.setSize({halfW, halfH});
    mSouthWestRect.setPosition({0.f, halfH});
    mSouthWestRect.setFillColor(sf::Color(80, 90, 40));

    if (fs::exists("assets/textures/map/southwest/tileset.png")) {
        if (mSouthWestTex.loadFromFile("assets/textures/map/southwest/tileset.png")) {
            mSouthWestTex.setRepeated(true);
            mSouthWestSprite = std::make_unique<sf::Sprite>(mSouthWestTex);
            mSouthWestSprite->setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(halfW), static_cast<int>(halfH)}));
            mSouthWestSprite->setPosition({0.f, halfH});
            mHasSouthWestTex = true;
        }
    }

    // SouthEast: Bottom-Right (Dark Green)
    mSouthEastRect.setSize({halfW, halfH});
    mSouthEastRect.setPosition({halfW, halfH});
    mSouthEastRect.setFillColor(sf::Color(50, 90, 60));

    if (fs::exists("assets/textures/map/southeast/tileset.png")) {
        if (mSouthEastTex.loadFromFile("assets/textures/map/southeast/tileset.png")) {
            mSouthEastTex.setRepeated(true);
            mSouthEastSprite = std::make_unique<sf::Sprite>(mSouthEastTex);
            mSouthEastSprite->setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(halfW), static_cast<int>(halfH)}));
            mSouthEastSprite->setPosition({halfW, halfH});
            mHasSouthEastTex = true;
        }
    }

    // Center spawn zone (Stone circle)
    float centerRadius = 150.f;
    mCenterCircle.setRadius(centerRadius);
    mCenterCircle.setFillColor(sf::Color(100, 100, 100));
    mCenterCircle.setOutlineThickness(5.f);
    mCenterCircle.setOutlineColor(sf::Color(150, 150, 150));
    mCenterCircle.setOrigin({centerRadius, centerRadius});
    mCenterCircle.setPosition({halfW, halfH});

    if (fs::exists("assets/textures/map/center/tileset.png")) {
        if (mCenterTex.loadFromFile("assets/textures/map/center/tileset.png")) {
            mCenterTex.setRepeated(true);
            mCenterSprite = std::make_unique<sf::Sprite>(mCenterTex);
            mCenterSprite->setTextureRect(sf::IntRect({0, 0}, {300, 300}));
            mCenterSprite->setOrigin({150.f, 150.f});
            mCenterSprite->setPosition({halfW, halfH});
            mHasCenterTex = true;
        }
    }
}

void Map::draw(sf::RenderWindow& window) {
    if (mHasNorthWestTex && mNorthWestSprite) window.draw(*mNorthWestSprite); else window.draw(mNorthWestRect);
    if (mHasNorthEastTex && mNorthEastSprite) window.draw(*mNorthEastSprite); else window.draw(mNorthEastRect);
    if (mHasSouthWestTex && mSouthWestSprite) window.draw(*mSouthWestSprite); else window.draw(mSouthWestRect);
    if (mHasSouthEastTex && mSouthEastSprite) window.draw(*mSouthEastSprite); else window.draw(mSouthEastRect);
    if (mHasCenterTex && mCenterSprite) window.draw(*mCenterSprite); else window.draw(mCenterCircle);
}

sf::Vector2f Map::getCenter() const { return {mWidth / 2.f, mHeight / 2.f}; }

