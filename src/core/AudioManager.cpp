#include "core/AudioManager.h"

AudioManager& AudioManager::instance() {
    static AudioManager s_instance;
    return s_instance;
}

AudioManager::AudioManager() {
    mBuffers.load("button",  "assets/sounds/button.wav");
    mBuffers.load("attack",  "assets/sounds/attack.wav");
    mBuffers.load("hit",     "assets/sounds/hit.wav");
    mBuffers.load("dash",    "assets/sounds/dash.wav");
    mBuffers.load("player_hurt", "assets/sounds/player_hurt.wav");
    mBuffers.load("player_death", "assets/sounds/player_death.wav");
    mBuffers.load("skill_generic", "assets/sounds/skill_generic.wav");
    mBuffers.load("gold_pickup", "assets/sounds/gold_pickup.wav");
    mBuffers.load("potion_use", "assets/sounds/potion_use.wav");
    mBuffers.load("scroll_pickup", "assets/sounds/scroll_pickup.wav");
    mBuffers.load("enemy_death", "assets/sounds/enemy_death.wav");
    mBuffers.load("boss_death", "assets/sounds/boss_death.wav");
    mBuffers.load("boss_encounter", "assets/sounds/boss_encounter.wav");
    mBuffers.load("explosion", "assets/sounds/explosion.wav");
    mBuffers.load("levelup", "assets/sounds/levelup.wav");
    if (fs::exists("assets/sounds/purchase_success.wav")) mBuffers.load("purchase_success", "assets/sounds/purchase_success.wav");
    if (fs::exists("assets/music/menu.ogg") && mMenuMusic.openFromFile("assets/music/menu.ogg")) mMenuMusicLoaded = true;
    if (fs::exists("assets/music/battle.ogg") && mBattleMusic.openFromFile("assets/music/battle.ogg")) mBattleMusicLoaded = true;
    if (fs::exists("assets/music/victory.ogg") && mVictoryMusic.openFromFile("assets/music/victory.ogg")) mVictoryMusicLoaded = true;
    if (fs::exists("assets/music/defeat.ogg") && mDefeatMusic.openFromFile("assets/music/defeat.ogg")) mDefeatMusicLoaded = true;
}

void AudioManager::playSfx(const std::string& id) {
    try {
        if (!mBuffers.has(id)) return;
        const auto& buf = mBuffers.get(id);
        if (buf.getSampleCount() == 0) return;
        mSounds.remove_if([](const sf::Sound& s) { return s.getStatus() == sf::Sound::Status::Stopped; });
        mSounds.emplace_back(buf);
        mSounds.back().setVolume(mSfxVolume * mMasterVolume / 100.f);
        mSounds.back().play();
    } catch(...) {}
}

bool AudioManager::isSfxPlaying(const std::string& id) {
    if (!mBuffers.has(id)) return false;
    const sf::SoundBuffer& buffer = mBuffers.get(id);
    for (auto& s : mSounds) if (s.getStatus() == sf::Sound::Status::Playing && &s.getBuffer() == &buffer) return true;
    return false;
}

void AudioManager::playMenuMusic() {
    if (!mMenuMusicLoaded) return;
    stopMusic();
    if (mMenuMusic.getStatus() != sf::SoundStream::Status::Playing) {
        mMenuMusic.setLooping(true);
        mMenuMusic.setVolume(mMusicVolume * mMasterVolume / 100.f);
        mMenuMusic.play();
    }
}

void AudioManager::playBattleMusic() {
    if (!mBattleMusicLoaded) return;
    stopMusic();
    if (mBattleMusic.getStatus() != sf::SoundStream::Status::Playing) {
        mBattleMusic.setLooping(true);
        mBattleMusic.setVolume(mMusicVolume * mMasterVolume / 100.f);
        mBattleMusic.play();
    }
}

void AudioManager::playVictoryMusic() {
    if (!mVictoryMusicLoaded) return;
    stopMusic();
    if (mVictoryMusic.getStatus() != sf::SoundStream::Status::Playing) {
        mVictoryMusic.setLooping(false);
        mVictoryMusic.setVolume(mMusicVolume * mMasterVolume / 100.f);
        mVictoryMusic.play();
    }
}

void AudioManager::playDefeatMusic() {
    if (!mDefeatMusicLoaded) return;
    stopMusic();
    if (mDefeatMusic.getStatus() != sf::SoundStream::Status::Playing) {
        mDefeatMusic.setLooping(false);
        mDefeatMusic.setVolume(mMusicVolume * mMasterVolume / 100.f);
        mDefeatMusic.play();
    }
}

void AudioManager::stopMusic() {
    if (mMenuMusicLoaded) mMenuMusic.stop();
    if (mBattleMusicLoaded) mBattleMusic.stop();
    if (mVictoryMusicLoaded) mVictoryMusic.stop();
    if (mDefeatMusicLoaded) mDefeatMusic.stop();
}

void AudioManager::setMasterVolume(float v) {
    mMasterVolume = std::clamp(v, 0.f, 100.f);
    // Apply to currently playing music
    float musicVol = mMusicVolume * mMasterVolume / 100.f;
    if (mMenuMusicLoaded) mMenuMusic.setVolume(musicVol);
    if (mBattleMusicLoaded) mBattleMusic.setVolume(musicVol);
    if (mVictoryMusicLoaded) mVictoryMusic.setVolume(musicVol);
    if (mDefeatMusicLoaded) mDefeatMusic.setVolume(musicVol);
}

void AudioManager::setMusicVolume(float v) {
    mMusicVolume = std::clamp(v, 0.f, 100.f);
    float musicVol = mMusicVolume * mMasterVolume / 100.f;
    if (mMenuMusicLoaded) mMenuMusic.setVolume(musicVol);
    if (mBattleMusicLoaded) mBattleMusic.setVolume(musicVol);
    if (mVictoryMusicLoaded) mVictoryMusic.setVolume(musicVol);
    if (mDefeatMusicLoaded) mDefeatMusic.setVolume(musicVol);
}

void AudioManager::setSfxVolume(float v) {
    mSfxVolume = std::clamp(v, 0.f, 100.f);
}