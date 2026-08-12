#pragma once
#include "common/lib.h"

enum class Facing { Up, Down, Left, Right };

struct AnimClip {
    std::vector<sf::IntRect> frames;
    float frameDuration = 0.1f;
    bool loop = true;
};

class AnimatedSprite {
private:
    const sf::Texture& mTexture;
    mutable sf::Sprite mSprite;
    std::unordered_map<std::string, AnimClip> mClips;
    std::string mCurrentKey;
    std::size_t mFrameIndex = 0;
    float mFrameTimer = 0.f;
    bool mFinished = false;
    bool mFlipX = false;
    bool mDeathFading = false;
    float mDeathTimer = 0.f;
    float mDeathDuration = 0.f;
    sf::Vector2f mScale = { 1.f, 1.f };
public:
    explicit AnimatedSprite(const sf::Texture& texture);

    void addClipFlippable(const std::string& state, AnimClip clip);

    void play(const std::string& state, Facing facing, bool restartIfSame = false);
    void playFlippable(const std::string& state, bool restartIfSame = false);

    void update(float dt);

    void setPosition(sf::Vector2f pos);
    void setFlipX(bool flip);
    void setScale(sf::Vector2f scale);
    void setTint(sf::Color color);


    void draw(sf::RenderWindow& window) const;

    sf::FloatRect getLocalBounds() const;
    sf::FloatRect getGlobalBounds() const;
};