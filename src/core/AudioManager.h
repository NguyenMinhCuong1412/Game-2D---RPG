#pragma once
#include "common/lib.h"
#include "core/ResourceHolder.h"

class AudioManager {
private:
    AudioManager();

    ResourceHolder<sf::SoundBuffer, std::string> mBuffers;
    std::list<sf::Sound> mSounds;
    sf::Music mMenuMusic;
    sf::Music mBattleMusic;
    sf::Music mVictoryMusic;
    sf::Music mDefeatMusic;

    bool mMenuMusicLoaded = false;
    bool mBattleMusicLoaded = false;
    bool mVictoryMusicLoaded = false;
    bool mDefeatMusicLoaded = false;

    float mMasterVolume = 100.f;
    float mMusicVolume = 100.f;
    float mSfxVolume = 100.f;
    bool mDamageNumbersEnabled = true;
    bool mFullscreen = false;
public:
    static AudioManager& instance();

    void playSfx(const std::string& id);
    bool isSfxPlaying(const std::string& id);
    void playMenuMusic();
    void playBattleMusic();
    void playVictoryMusic();
    void playDefeatMusic();
    void stopMusic();

    // Volume control (UI-only, does not affect gameplay)
    float getMasterVolume() const { return mMasterVolume; }
    float getMusicVolume() const { return mMusicVolume; }
    float getSfxVolume() const { return mSfxVolume; }
    void setMasterVolume(float v);
    void setMusicVolume(float v);
    void setSfxVolume(float v);

    // Display settings (UI-only, does not affect gameplay calculations)
    bool isDamageNumbersEnabled() const { return mDamageNumbersEnabled; }
    void setDamageNumbersEnabled(bool enabled) { mDamageNumbersEnabled = enabled; }
    bool isFullscreen() const { return mFullscreen; }
    void setFullscreen(bool fs) { mFullscreen = fs; }
};