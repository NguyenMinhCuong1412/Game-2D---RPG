#pragma once
#include "common/lib.h"

enum class Region {
    Center,
    NorthWest,
    NorthEast,
    SouthWest,
    SouthEast
};

class Map {
public:
    Map();
    void draw(sf::RenderWindow& window);
    sf::Vector2f getCenter() const;

private:
    float mWidth = 4000.f;
    float mHeight = 4000.f;
    sf::RectangleShape mNorthWestRect;
    sf::RectangleShape mNorthEastRect;
    sf::RectangleShape mSouthWestRect;
    sf::RectangleShape mSouthEastRect;
    sf::CircleShape mCenterCircle;

    sf::Texture mNorthWestTex, mNorthEastTex, mSouthWestTex, mSouthEastTex, mCenterTex;
    std::unique_ptr<sf::Sprite> mNorthWestSprite, mNorthEastSprite, mSouthWestSprite, mSouthEastSprite, mCenterSprite;
    bool mHasNorthWestTex = false, mHasNorthEastTex = false, mHasSouthWestTex = false, mHasSouthEastTex = false, mHasCenterTex = false;
};
