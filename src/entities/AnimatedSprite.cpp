#include "entities/AnimatedSprite.h"

AnimatedSprite::AnimatedSprite(const sf::Texture& texture) : mTexture(texture), mSprite(texture) {}

void AnimatedSprite::addClipFlippable(const std::string& state, AnimClip clip) { mClips[state] = clip; }

void AnimatedSprite::play(const std::string& state, Facing facing, bool restartIfSame) {
    std::string suffix = "Down";
    if (facing == Facing::Up) suffix = "Up";
    else if (facing == Facing::Down) suffix = "Down";
    else if (facing == Facing::Left) suffix = "Left";
    else if (facing == Facing::Right) suffix = "Right";

    std::string key = state + "|" + suffix;

    if (mClips.find(key) == mClips.end()) key = state;
    if (mCurrentKey == key && !restartIfSame) return;

    mCurrentKey = key;
    mFrameIndex = 0;
    mFrameTimer = 0.f;
    mFinished = false;
}

void AnimatedSprite::playFlippable(const std::string& state, bool restartIfSame) {
    if (mCurrentKey == state && !restartIfSame) return;
    mCurrentKey = state;
    mFrameIndex = 0;
    mFrameTimer = 0.f;
    mFinished = false;
}

void AnimatedSprite::update(float dt) {
    if (mDeathFading) {
        mDeathTimer -= dt;
        float alpha = std::max(0.f, (mDeathTimer / mDeathDuration)) * 255.f;
        sf::Color c = mSprite.getColor();
        mSprite.setColor(sf::Color(c.r, c.g, c.b, static_cast<std::uint8_t>(alpha)));
        return;
    }

    auto it = mClips.find(mCurrentKey);
    if (it == mClips.end()) return;

    const auto& clip = it->second;
    if (clip.frames.empty()) return;

    mFinished = false;
    mFrameTimer += dt;
    if (mFrameTimer >= clip.frameDuration) {
        mFrameTimer = 0.f;
        if (mFrameIndex + 1 < clip.frames.size()) mFrameIndex++;
        else {
            if (clip.loop) mFrameIndex = 0;
            else mFinished = true;
        }
    }
    mSprite.setTextureRect(clip.frames[mFrameIndex]);
}

void AnimatedSprite::setPosition(sf::Vector2f pos) { mSprite.setPosition(pos); }
void AnimatedSprite::setFlipX(bool flip) { mFlipX = flip; }
void AnimatedSprite::setScale(sf::Vector2f scale) { mScale = scale; }
void AnimatedSprite::setTint(sf::Color color) { mSprite.setColor(color); }

void AnimatedSprite::draw(sf::RenderWindow& window) const {
    auto textureRect = mSprite.getTextureRect();
    sf::Vector2f finalScale = mScale;
    if (mFlipX) finalScale.x = -std::abs(mScale.x);
    else finalScale.x = std::abs(mScale.x);
    mSprite.setScale(finalScale);
    mSprite.setOrigin(sf::Vector2f(textureRect.size.x / 2.f, textureRect.size.y / 2.f));
    window.draw(mSprite);
}

sf::FloatRect AnimatedSprite::getLocalBounds() const { return mSprite.getLocalBounds(); }
sf::FloatRect AnimatedSprite::getGlobalBounds() const { return mSprite.getGlobalBounds(); }