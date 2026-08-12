#include "entities/Projectile.h"

Projectile::Projectile(sf::Vector2f position, sf::Vector2f velocity, float damage, float lifetime, float radius, sf::Color color)
    : mPosition(position), mVelocity(velocity), mDamage(damage), mLifetime(lifetime), mRadius(radius)
{
    mShape.setRadius(mRadius);
    mShape.setOrigin({mRadius, mRadius});
    mShape.setFillColor(color);
    mShape.setPosition(mPosition);
}

void Projectile::setTexture(const sf::Texture* tex) {
    if (tex) {
        mSprite.emplace(*tex);
        auto sz = tex->getSize();
        mSprite->setOrigin({sz.x / 2.f, sz.y / 2.f});
        float angle = std::atan2(mVelocity.y, mVelocity.x) * 180.f / 3.14159f;
        mSprite->setRotation(sf::degrees(angle));
    }
}

void Projectile::addAnimFrame(const sf::Texture* tex, sf::IntRect rect) {
    if (tex) {
        mAnimFrames.push_back(tex);
        if (rect.size.x == 0 || rect.size.y == 0)
            rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mAnimRects.push_back(rect);
        if (mAnimFrames.size() == 1) {
            setTexture(tex);
            mSprite->setTextureRect(rect);
            mSprite->setOrigin({rect.size.x / 2.f, rect.size.y / 2.f});
        }
    }
}

void Projectile::update(float dt) {
    if (mVelocity == sf::Vector2f(0.f, 0.f)) {
        if (!mAnimFrames.empty()) {
            mAnimTimer += dt;
            float frameDuration = mLifetime / mAnimFrames.size();
            if (mAnimTimer >= frameDuration) {
                mAnimTimer = 0.f;
                mAnimFrameIdx++;
                if (mAnimFrameIdx >= mAnimFrames.size()) {
                    mLifetime = 0.f;
                    mAnimFrameIdx = mAnimFrames.size() - 1;
                }
            }
            if (mSprite) {
                mSprite->setTexture(*mAnimFrames[mAnimFrameIdx], true);
                if (mAnimFrameIdx < mAnimRects.size()) {
                    mSprite->setTextureRect(mAnimRects[mAnimFrameIdx]);
                    mSprite->setOrigin({mAnimRects[mAnimFrameIdx].size.x / 2.f, mAnimRects[mAnimFrameIdx].size.y / 2.f});
                } else {
                    auto size = mAnimFrames[mAnimFrameIdx]->getSize();
                    mSprite->setOrigin({size.x / 2.f, size.y / 2.f});
                }
            }
        } else mLifetime -= dt;
    } else {
        mPosition += mVelocity * dt;
        mLifetime -= dt;
    }
    mShape.setPosition(mPosition);
    if (mSprite.has_value()) mSprite->setPosition(mPosition);

}

void Projectile::draw(sf::RenderWindow& window) {
    if (mSprite.has_value()) window.draw(*mSprite);
    else window.draw(mShape);
}