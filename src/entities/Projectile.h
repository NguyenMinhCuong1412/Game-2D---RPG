#pragma once
#include "common/lib.h"

class Projectile {
private:
    sf::Vector2f mPosition;
    sf::Vector2f mVelocity;
    float mDamage;
    float mLifetime;
    float mRadius = 6.f;
    sf::CircleShape mShape;
    std::optional<sf::Sprite> mSprite;
    bool mIsEnemy = false;

    std::vector<const sf::Texture*> mAnimFrames;
    std::vector<sf::IntRect> mAnimRects;
    float mAnimTimer = 0.f;
    std::size_t mAnimFrameIdx = 0;
    bool mIsEffectOnly = false;
    bool mHasHit = false;
public:
    Projectile(sf::Vector2f position, sf::Vector2f velocity, float damage, float lifetime, float radius = 6.f, sf::Color color = sf::Color::Yellow);

    void update(float dt);
    void draw(sf::RenderWindow& window);

    bool isExpired() const { return mLifetime <= 0.f; }
    sf::Vector2f getPosition() const { return mPosition; }
    float getRadius() const { return mRadius; }
    float getDamage() const { return mDamage; }
    sf::Vector2f getVelocity() const { return mVelocity; }
    float getLifetime() const { return mLifetime; }

    bool isEnemy() const { return mIsEnemy; }
    void setIsEnemy(bool val) { mIsEnemy = val; }

    void setTexture(const sf::Texture* tex);
    void addAnimFrame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect());

    void setEffectOnly(bool val) { mIsEffectOnly = val; }

    bool hasHit() const { return mHasHit; }
    void setHasHit(bool val) { mHasHit = val; }

    std::size_t getAnimFrameIndex() const { return mAnimFrameIdx; }
    std::size_t getAnimFrameCount() const { return mAnimFrames.empty() ? 1 : mAnimFrames.size(); }
};