#include "states/PlayingState.h"
#include "states/StateManager.h"
#include "states/MainMenuState.h"
#include "states/GameOverState.h"
#include "states/VictoryState.h"
#include "database/GameData.h"
#include "systems/SpawnSystem.h"
#include "systems/LootSystem.h"
#include "systems/CollisionSystem.h"
#include "character/CharacterUtil.h"
#include "core/AudioManager.h"
#include "common/lib.h"

// New game constructor
PlayingState::PlayingState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, CharacterType character, const GameData& gameData)
    : State(stateManager)
    , mWindow(window)
    , mFont(font)
    , mGameData(gameData)
    , mPlayer(std::make_unique<Player>(character, mGameData))
    , mRng(std::random_device{}())
{
    mPlayer->setPosition(mMap.getCenter());
    initCommon();
}

// Load from save constructor
PlayingState::PlayingState(StateManager& stateManager, sf::RenderWindow& window, const sf::Font& font, const SaveData& save, const GameData& gameData)
    : State(stateManager)
    , mWindow(window)
    , mFont(font)
    , mGameData(gameData)
    , mPlayer(std::make_unique<Player>(save.characterType, mGameData))
    , mRng(std::random_device{}())
{
    mPlayer->setPosition({save.posX, save.posY});
    mPlayer->setHP(save.hp);
    mPlayer->setMaxHP(save.maxHP);
    mPlayer->setLevel(save.level);
    mPlayer->setEXP(save.exp);
    mPlayer->setGold(save.gold);
    mPlayer->setPotions(save.potions);
    mInventorySkills = save.inventorySkills;
    mBossGoblinDead = save.bossGoblinDead;
    mBossDemonDead = save.bossDemonDead;
    mBossBringerDead = save.bossBringerDead;
    mPlayer->setHPScrollCount(save.hpScrollCount);
    mPlayer->setDamageScrollCount(save.damageScrollCount);
    mPlayer->setDefenseScrollCount(save.defenseScrollCount);
    mPlayer->recalculateStats();
    mPlayer->setHP(save.hp);
    initCommon();
}

void PlayingState::initCommon() {
    mView.setSize({800.f, 600.f});
    mView.setCenter(mPlayer->getPosition());

    std::string btnPath = mGameData.getAssetPath("ui_button");
    if (!btnPath.empty() && std::filesystem::exists(btnPath))
        if (mBtnTexture.loadFromFile(btnPath)) mHasBtnTex = true;

    if (std::filesystem::exists("assets/textures/ui/pause_button.png")) {
        if (mPauseBtnTex.loadFromFile("assets/textures/ui/pause_button.png")) {
            mPauseBtnSprite = std::make_unique<sf::Sprite>(mPauseBtnTex);
            auto pSize = mPauseBtnTex.getSize();
            mPauseBtnSprite->setOrigin({pSize.x / 2.f, pSize.y / 2.f});
            mHasPauseBtn = true;
        }
    }


    // Load loot textures
    mTextureHolder.load("gold", mGameData.getAssetPath("gold"));
    mTextureHolder.load("potion", mGameData.getAssetPath("potion"));
    mTextureHolder.load("scroll", mGameData.getAssetPath("scroll"));
    mTextureHolder.load("scroll_hp", mGameData.getAssetPath("scroll_hp"));
    mTextureHolder.load("scroll_attack", mGameData.getAssetPath("scroll_attack"));
    mTextureHolder.load("scroll_defense", mGameData.getAssetPath("scroll_defense"));

    // Load projectile textures
    mTextureHolder.load("arrow_attack", mGameData.getAssetPath("arrow_attack"));
    mTextureHolder.load("arrow_skill", mGameData.getAssetPath("arrow_skill"));
    mTextureHolder.load("skeleton_arrow", mGameData.getAssetPath("skeleton_arrow"));

    // Load player animations
    std::string pKey = characterKey(mPlayer->getType());
    std::string pIdlePath = "assets/textures/player/" + pKey + "_idle.png";
    std::string pWalkPath = "assets/textures/player/" + pKey + "_walk.png";
    std::string pAtkPath  = "assets/textures/player/" + pKey + "_attack.png";

    std::string pSkillQPath = "assets/textures/player/" + pKey + "_skill_Q.png";
    std::string pSkillEPath = "assets/textures/player/" + pKey + "_skill_E.png";
    std::string pSkillRPath = "assets/textures/player/" + pKey + "_skill_R.png";

    if (std::filesystem::exists(pIdlePath)) {
        mTextureHolder.load("p_idle_sheet", pIdlePath);
        mPlayer->addSpriteSheetFrames(&mTextureHolder.get("p_idle_sheet"), "idle");
    }
    if (std::filesystem::exists(pWalkPath)) {
        mTextureHolder.load("p_walk_sheet", pWalkPath);
        mPlayer->addSpriteSheetFrames(&mTextureHolder.get("p_walk_sheet"), "walk");
    }
    if (std::filesystem::exists(pAtkPath)) {
        mTextureHolder.load("p_atk_sheet", pAtkPath);
        mPlayer->addSpriteSheetFrames(&mTextureHolder.get("p_atk_sheet"), "attack");
    }
    if (std::filesystem::exists(pSkillQPath)) {
        mTextureHolder.load("p_skill_q_sheet", pSkillQPath);
        mPlayer->addSpriteSheetFrames(&mTextureHolder.get("p_skill_q_sheet"), "skill_q");
    }
    if (std::filesystem::exists(pSkillEPath)) {
        mTextureHolder.load("p_skill_e_sheet", pSkillEPath);
        mPlayer->addSpriteSheetFrames(&mTextureHolder.get("p_skill_e_sheet"), "skill_e");
    }
    if (std::filesystem::exists(pSkillRPath)) {
        mTextureHolder.load("p_skill_r_sheet", pSkillRPath);
        mPlayer->addSpriteSheetFrames(&mTextureHolder.get("p_skill_r_sheet"), "skill_r");
    }

    mPlayer->loadAnimations(
        mTextureHolder.has("p_idle_sheet") ? &mTextureHolder.get("p_idle_sheet") : nullptr,
        mTextureHolder.has("p_walk_sheet") ? &mTextureHolder.get("p_walk_sheet") : nullptr,
        mTextureHolder.has("p_atk_sheet") ? &mTextureHolder.get("p_atk_sheet") : nullptr,
        mTextureHolder.has("p_skill_q_sheet") ? &mTextureHolder.get("p_skill_q_sheet") : nullptr,
        mTextureHolder.has("p_skill_e_sheet") ? &mTextureHolder.get("p_skill_e_sheet") : nullptr,
        mTextureHolder.has("p_skill_r_sheet") ? &mTextureHolder.get("p_skill_r_sheet") : nullptr
    );

    for (int i = 0; i < 10; ++i) {
        std::string path = "assets/textures/player/" + pKey + "_idle_" + std::to_string(i) + ".png";
        if (std::filesystem::exists(path)) {
            mTextureHolder.load("p_idle_" + std::to_string(i), path);
            mPlayer->addIdleFrame(&mTextureHolder.get("p_idle_" + std::to_string(i)));
        }
    }
    for (int i = 0; i < 10; ++i) {
        std::string path = "assets/textures/player/" + pKey + "_walk_" + std::to_string(i) + ".png";
        if (std::filesystem::exists(path)) {
            mTextureHolder.load("p_walk_" + std::to_string(i), path);
            mPlayer->addWalkFrame(&mTextureHolder.get("p_walk_" + std::to_string(i)));
        }
    }
    for (int i = 0; i < 10; ++i) {
        std::string path = "assets/textures/player/" + pKey + "_attack_" + std::to_string(i) + ".png";
        if (std::filesystem::exists(path)) {
            mTextureHolder.load("p_atk_" + std::to_string(i), path);
            mPlayer->addAttackFrame(&mTextureHolder.get("p_atk_" + std::to_string(i)));
        }
    }

    // Load UI skill icons for current character
    std::string uiSkillDir = "assets/textures/ui/skills/";
    if (std::filesystem::exists(uiSkillDir + "dash_icon.png")) {
        mTextureHolder.load("ui_skill_dash", uiSkillDir + "dash_icon.png");
    }

    std::string cName = characterName(mPlayer->getType());
    std::transform(cName.begin(), cName.end(), cName.begin(), [](unsigned char c) { return std::tolower(c); });

    if (std::filesystem::exists(uiSkillDir + cName + "_icon_skill_Q.png"))
        mTextureHolder.load("ui_skill_q", uiSkillDir + cName + "_icon_skill_Q.png");
    if (std::filesystem::exists(uiSkillDir + cName + "_icon_skill_E.png"))
        mTextureHolder.load("ui_skill_e", uiSkillDir + cName + "_icon_skill_E.png");
    if (std::filesystem::exists(uiSkillDir + cName + "_icon_skill_R.png"))
        mTextureHolder.load("ui_skill_r", uiSkillDir + cName + "_icon_skill_R.png");

    spawnInitialEnemies();
    for (auto& enemy : mEnemies) setupEnemyAnimations(enemy.get());

    AudioManager::instance().playBattleMusic();
}

void PlayingState::handleEvent(const sf::Event& event) {
    // Window lost focus -> Auto-Pause
    if (event.is<sf::Event::FocusLost>()) {
        mPauseMenuOpen = true;
        mPauseSelection = 0;
        return;
    }

    // Camera Zooming via Mouse Wheel Scroll
    if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (wheel->wheel == sf::Mouse::Wheel::Vertical) {
            if (wheel->delta > 0) mZoomLevel = std::max(0.4f, mZoomLevel * 0.9f);
            else if (wheel->delta < 0) mZoomLevel = std::min(2.5f, mZoomLevel * 1.1f);
            mView.setSize({800.f * mZoomLevel, 600.f * mZoomLevel});
        }
    }

    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        // --- Pause Menu ---
        if (mPauseMenuOpen) {
            if (mPauseSettingsOpen) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) mPauseSettingsOpen = false;
                else if (keyPressed->code == sf::Keyboard::Key::Up || keyPressed->code == sf::Keyboard::Key::W) {
                    mPauseSettingsSelection = (mPauseSettingsSelection - 1 + 7) % 7;
                    mPauseSettingsHoveredIdx = -1;
                    mPauseSettingsSelectionActive = true;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Down || keyPressed->code == sf::Keyboard::Key::S) {
                    mPauseSettingsSelection = (mPauseSettingsSelection + 1) % 7;
                    mPauseSettingsHoveredIdx = -1;
                    mPauseSettingsSelectionActive = true;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Left || keyPressed->code == sf::Keyboard::Key::A) {
                    mPauseSettingsHoveredIdx = -1;
                    mPauseSettingsSelectionActive = true;
                    if (mPauseSettingsSelection == 0) mPauseSettingsMaster = std::max(0.f, mPauseSettingsMaster - 5.f);
                    else if (mPauseSettingsSelection == 1) mPauseSettingsMusic = std::max(0.f, mPauseSettingsMusic - 5.f);
                    else if (mPauseSettingsSelection == 2) mPauseSettingsSfx = std::max(0.f, mPauseSettingsSfx - 5.f);
                    else if (mPauseSettingsSelection == 3) mPauseSettingsFullscreen = !mPauseSettingsFullscreen;
                    else if (mPauseSettingsSelection == 4) mPauseSettingsDamageNumbers = !mPauseSettingsDamageNumbers;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Right || keyPressed->code == sf::Keyboard::Key::D) {
                    mPauseSettingsHoveredIdx = -1;
                    mPauseSettingsSelectionActive = true;
                    if (mPauseSettingsSelection == 0) mPauseSettingsMaster = std::min(100.f, mPauseSettingsMaster + 5.f);
                    else if (mPauseSettingsSelection == 1) mPauseSettingsMusic = std::min(100.f, mPauseSettingsMusic + 5.f);
                    else if (mPauseSettingsSelection == 2) mPauseSettingsSfx = std::min(100.f, mPauseSettingsSfx + 5.f);
                    else if (mPauseSettingsSelection == 3) mPauseSettingsFullscreen = !mPauseSettingsFullscreen;
                    else if (mPauseSettingsSelection == 4) mPauseSettingsDamageNumbers = !mPauseSettingsDamageNumbers;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                    AudioManager::instance().playSfx("button");
                    mPauseSettingsHoveredIdx = -1;
                    mPauseSettingsSelectionActive = true;
                    if (mPauseSettingsSelection == 3) mPauseSettingsFullscreen = !mPauseSettingsFullscreen;
                    else if (mPauseSettingsSelection == 4) mPauseSettingsDamageNumbers = !mPauseSettingsDamageNumbers;
                    else if (mPauseSettingsSelection == 5) {
                        AudioManager::instance().setMasterVolume(mPauseSettingsMaster);
                        AudioManager::instance().setMusicVolume(mPauseSettingsMusic);
                        AudioManager::instance().setSfxVolume(mPauseSettingsSfx);
                        AudioManager::instance().setDamageNumbersEnabled(mPauseSettingsDamageNumbers);
                        AudioManager::instance().setFullscreen(mPauseSettingsFullscreen);

                        bool currentlyFullscreen = (mWindow.getSize().x > 800 || mWindow.getSize().y > 600);
                        if (mPauseSettingsFullscreen != currentlyFullscreen) {
                            if (mPauseSettingsFullscreen) {
                                mWindow.create(sf::VideoMode::getDesktopMode(), "Legend of the Realm", sf::State::Fullscreen);
                            } else {
                                mWindow.create(sf::VideoMode({800, 600}), "Legend of the Realm", sf::State::Windowed);
                            }
                            mWindow.setFramerateLimit(60);
                            sf::View view({400.f, 300.f}, {800.f, 600.f});
                            mWindow.setView(view);
                        }
                    }
                    else if (mPauseSettingsSelection == 6) {
                        mPauseSettingsOpen = false;
                    }
                }
                return;
            }
            if (mPauseControlsOpen) {
                if (keyPressed->code == sf::Keyboard::Key::Escape || keyPressed->code == sf::Keyboard::Key::Enter || keyPressed->code == sf::Keyboard::Key::Space) {
                    AudioManager::instance().playSfx("button");
                    mPauseControlsOpen = false;
                }
                return;
            }

            if (keyPressed->code == sf::Keyboard::Key::Escape) mPauseMenuOpen = false;
            else if (keyPressed->code == sf::Keyboard::Key::Up || keyPressed->code == sf::Keyboard::Key::W) {
                mPauseSelection = (mPauseSelection - 1 + 5) % 5;
                mPauseHoveredIdx = -1;
                mPauseSelectionActive = true;
            }
            else if (keyPressed->code == sf::Keyboard::Key::Down || keyPressed->code == sf::Keyboard::Key::S) {
                mPauseSelection = (mPauseSelection + 1) % 5;
                mPauseHoveredIdx = -1;
                mPauseSelectionActive = true;
            }
            else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                AudioManager::instance().playSfx("button");
                if (mPauseSelection == 0) mPauseMenuOpen = false;
                else if (mPauseSelection == 1) {
                    mPauseSettingsOpen = true;
                    mPauseSettingsSelection = 0;
                    mPauseSettingsMaster = AudioManager::instance().getMasterVolume();
                    mPauseSettingsMusic = AudioManager::instance().getMusicVolume();
                    mPauseSettingsSfx = AudioManager::instance().getSfxVolume();
                    mPauseSettingsFullscreen = AudioManager::instance().isFullscreen();
                    mPauseSettingsDamageNumbers = AudioManager::instance().isDamageNumbersEnabled();
                }
                else if (mPauseSelection == 2) mPauseControlsOpen = true;
                else if (mPauseSelection == 3) {
                    saveCurrentGame(0);
                    addFloatingText("Saved Game!", mPlayer->getPosition(), sf::Color::Green);
                    mPauseMenuOpen = false;
                }
                else if (mPauseSelection == 4) mStateManager.changeState(std::make_unique<MainMenuState>(mStateManager, mWindow, mFont, mGameData), StateID::MainMenu);
            }
            return;
        }

        // --- Shop Menu ---
        if (mShopOpen) {
            if (keyPressed->code == sf::Keyboard::Key::Escape || keyPressed->code == sf::Keyboard::Key::Tab) mShopOpen = false;
            else if (keyPressed->code == sf::Keyboard::Key::Num1 || keyPressed->code == sf::Keyboard::Key::Numpad1) {
                int currentPrice = mGameData.getCurrentShopPrice("potion", mPlayer->getLevel());
                if (mPlayer->getGold() >= currentPrice) {
                    mPlayer->buyPotion();
                    AudioManager::instance().playSfx("purchase_success");
                    addFloatingText("+1 Potion", mPlayer->getPosition(), sf::Color::Red);
                } else {
                    mShopErrorMessage = "Not enough gold!";
                    mShopErrorTimer = 1.5f;
                }
            }
            else if (keyPressed->code == sf::Keyboard::Key::Num2 || keyPressed->code == sf::Keyboard::Key::Numpad2) {
                const ItemData& hpScroll = mGameData.getItem("scroll_hp");
                int currentPrice = mGameData.getCurrentShopPrice("scroll_hp", mPlayer->getLevel());
                if (mPlayer->getGold() >= currentPrice) {
                    mPlayer->gainGold(-currentPrice);
                    mPlayer->addHPScrollCount(1);
                    AudioManager::instance().playSfx("purchase_success");
                    addFloatingText("+" + std::to_string(hpScroll.effectValue) + "% Max HP!", mPlayer->getPosition(), sf::Color::Green);
                } else {
                    mShopErrorMessage = "Not enough gold!";
                    mShopErrorTimer = 1.5f;
                }
            }
            else if (keyPressed->code == sf::Keyboard::Key::Num3 || keyPressed->code == sf::Keyboard::Key::Numpad3) {
                const ItemData& atkScroll = mGameData.getItem("scroll_attack");
                int currentPrice = mGameData.getCurrentShopPrice("scroll_attack", mPlayer->getLevel());
                if (mPlayer->getGold() >= currentPrice) {
                    mPlayer->gainGold(-currentPrice);
                    mPlayer->addDamageScrollCount(1);
                    AudioManager::instance().playSfx("purchase_success");
                    addFloatingText("+" + std::to_string(atkScroll.effectValue) + "% Damage Buff!", mPlayer->getPosition(), sf::Color(255, 165, 0));
                } else {
                    mShopErrorMessage = "Not enough gold!";
                    mShopErrorTimer = 1.5f;
                }
            }
            else if (keyPressed->code == sf::Keyboard::Key::Num4 || keyPressed->code == sf::Keyboard::Key::Numpad4) {
                const ItemData& defScroll = mGameData.getItem("scroll_defense");
                int currentPrice = mGameData.getCurrentShopPrice("scroll_defense", mPlayer->getLevel());
                if (mPlayer->getGold() >= currentPrice) {
                    mPlayer->gainGold(-currentPrice);
                    mPlayer->addDefenseScrollCount(1);
                    AudioManager::instance().playSfx("purchase_success");
                    addFloatingText("+" + std::to_string(defScroll.effectValue) + "% Defense Buff!", mPlayer->getPosition(), sf::Color(100, 200, 255));
                } else {
                    mShopErrorMessage = "Not enough gold!";
                    mShopErrorTimer = 1.5f;
                }
            }
            return;
        }

        // --- Normal Gameplay Toggles ---
        if (keyPressed->code == sf::Keyboard::Key::Escape) {
            mPauseMenuOpen = true;
            mPauseSelection = 0;
        }
        else if (keyPressed->code == sf::Keyboard::Key::Tab) {
            // Shop opens/closes with Tab key, only near shop zone
            sf::Vector2f centerPos = mMap.getCenter();
            float dx = mPlayer->getPosition().x - centerPos.x;
            float dy = mPlayer->getPosition().y - centerPos.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < 180.f) mShopOpen = !mShopOpen;
        }
    }
    else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::View uiView({400.f, 300.f}, {800.f, 600.f});
        sf::Vector2f mousePos = mWindow.mapPixelToCoords(mouseMoved->position, uiView);
        mPauseSelectionActive = false;

        if (!mPauseMenuOpen && !mShopOpen && mHasPauseBtn && mPauseBtnSprite) {
            mPauseBtnHovered = mPauseBtnSprite->getGlobalBounds().contains(mousePos);
        } else {
            mPauseBtnHovered = false;
        }

        if (mPauseMenuOpen) {
            if (mPauseSettingsOpen) {
                mPauseSettingsHoveredIdx = -1;
                for (int i = 0; i < 7; ++i) {
                    float itemY = 160.f + i * 55.f;
                    sf::FloatRect bounds({200.f, itemY - 15.f}, {400.f, 40.f});
                    if (bounds.contains(mousePos)) {
                        mPauseSettingsHoveredIdx = i;
                    }
                }
                mPauseSettingsSelectionActive = false;

                if (mPauseSettingsDragging && mPauseSettingsDragIdx >= 0) {
                    float sliderLeft = 410.f;
                    float sliderRight = 580.f;
                    float val = (mousePos.x - sliderLeft) / (sliderRight - sliderLeft) * 100.f;
                    val = std::clamp(val, 0.f, 100.f);
                    if (mPauseSettingsDragIdx == 0) mPauseSettingsMaster = val;
                    else if (mPauseSettingsDragIdx == 1) mPauseSettingsMusic = val;
                    else if (mPauseSettingsDragIdx == 2) mPauseSettingsSfx = val;
                }
                return;
            }
            if (mPauseControlsOpen) {
                sf::FloatRect backBounds({320.f, 430.f}, {160.f, 40.f});
                mPauseControlsBackHovered = backBounds.contains(mousePos);
                return;
            }

            int hovered = -1;
            for (int i = 0; i < 5; ++i) {
                float yVal = 170.f + i * 70.f;
                sf::FloatRect bounds({400.f - 120.f, yVal - 25.f}, {240.f, 50.f});
                if (bounds.contains(mousePos)) {
                    hovered = i;
                }
            }
            mPauseHoveredIdx = hovered;
        }
        else if (mShopOpen) {
            mShopHoveredIdx = -1;
            for (int i = 0; i < 4; ++i) {
                sf::FloatRect itemBounds({400.f - 160.f, 195.f + i * 68.f - 25.f}, {320.f, 50.f});
                if (itemBounds.contains(mousePos)) {
                    mShopHoveredIdx = i;
                    break;
                }
            }
        }
    }
    else if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Left) {
            sf::View uiView({400.f, 300.f}, {800.f, 600.f});
            sf::Vector2f mousePos = mWindow.mapPixelToCoords(mouseBtn->position, uiView);

            if (!mPauseMenuOpen && !mShopOpen && mHasPauseBtn && mPauseBtnSprite) {
                if (mPauseBtnSprite->getGlobalBounds().contains(mousePos)) {
                    AudioManager::instance().playSfx("button");
                    mPauseMenuOpen = true;
                    mPauseSelection = 0;
                    return;
                }
            }

            if (mPauseMenuOpen) {
                if (mPauseSettingsOpen) {
                    for (int i = 0; i < 3; ++i) {
                        float itemY = 160.f + i * 55.f;
                        sf::FloatRect sliderBounds({405.f, itemY - 8.f}, {180.f, 20.f});
                        if (sliderBounds.contains(mousePos)) {
                            mPauseSettingsDragging = true;
                            mPauseSettingsDragIdx = i;
                            mPauseSettingsSelection = i;
                            mPauseSettingsSelectionActive = false;
                            float sliderLeft = 410.f;
                            float sliderRight = 580.f;
                            float val = (mousePos.x - sliderLeft) / (sliderRight - sliderLeft) * 100.f;
                            val = std::clamp(val, 0.f, 100.f);
                            if (i == 0) mPauseSettingsMaster = val;
                            else if (i == 1) mPauseSettingsMusic = val;
                            else if (i == 2) mPauseSettingsSfx = val;
                        }
                    }
                    float fsY = 160.f + 3 * 55.f;
                    sf::FloatRect fsBounds({200.f, fsY - 15.f}, {400.f, 40.f});
                    if (fsBounds.contains(mousePos)) {
                        mPauseSettingsFullscreen = !mPauseSettingsFullscreen;
                        mPauseSettingsSelection = 3;
                        mPauseSettingsSelectionActive = false;
                    }
                    
                    float dnY = 160.f + 4 * 55.f;
                    sf::FloatRect dnBounds({200.f, dnY - 15.f}, {400.f, 40.f});
                    if (dnBounds.contains(mousePos)) {
                        mPauseSettingsDamageNumbers = !mPauseSettingsDamageNumbers;
                        mPauseSettingsSelection = 4;
                        mPauseSettingsSelectionActive = false;
                    }
                    
                    float applyY = 160.f + 5 * 55.f;
                    sf::FloatRect applyBounds({300.f, applyY - 15.f}, {200.f, 40.f});
                    if (applyBounds.contains(mousePos)) {
                        AudioManager::instance().playSfx("button");
                        AudioManager::instance().setMasterVolume(mPauseSettingsMaster);
                        AudioManager::instance().setMusicVolume(mPauseSettingsMusic);
                        AudioManager::instance().setSfxVolume(mPauseSettingsSfx);
                        AudioManager::instance().setDamageNumbersEnabled(mPauseSettingsDamageNumbers);

                        bool currentlyFullscreen = AudioManager::instance().isFullscreen();
                        AudioManager::instance().setFullscreen(mPauseSettingsFullscreen);

                        if (mPauseSettingsFullscreen != currentlyFullscreen) {
                            if (mPauseSettingsFullscreen) {
                                mWindow.create(sf::VideoMode::getDesktopMode(), "Legend of the Realm", sf::State::Fullscreen);
                            } else {
                                mWindow.create(sf::VideoMode({800, 600}), "Legend of the Realm", sf::State::Windowed);
                            }
                            mWindow.setFramerateLimit(60);
                            sf::View view({400.f, 300.f}, {800.f, 600.f});
                            mWindow.setView(view);
                        }
                    }
                    float backY = 160.f + 6 * 55.f;
                    sf::FloatRect backBounds({300.f, backY - 15.f}, {200.f, 40.f});
                    if (backBounds.contains(mousePos)) {
                        AudioManager::instance().playSfx("button");
                        mPauseSettingsOpen = false;
                    }
                    return;
                }
                if (mPauseControlsOpen) {
                    sf::FloatRect backBounds({320.f, 430.f}, {160.f, 40.f});
                    if (backBounds.contains(mousePos)) {
                        AudioManager::instance().playSfx("button");
                        mPauseControlsOpen = false;
                    }
                    return;
                }

                for (int i = 0; i < 5; ++i) {
                    sf::FloatRect btnBounds({400.f - 120.f, 170.f + i * 70.f - 25.f}, {240.f, 50.f});
                    if (btnBounds.contains(mousePos)) {
                        AudioManager::instance().playSfx("button");
                        mPauseSelection = i;
                        if (i == 0) mPauseMenuOpen = false;
                        else if (i == 1) {
                            mPauseSettingsOpen = true;
                            mPauseSettingsSelection = 0;
                            mPauseSettingsMaster = AudioManager::instance().getMasterVolume();
                            mPauseSettingsMusic = AudioManager::instance().getMusicVolume();
                            mPauseSettingsSfx = AudioManager::instance().getSfxVolume();
                            mPauseSettingsFullscreen = AudioManager::instance().isFullscreen();
                            mPauseSettingsDamageNumbers = AudioManager::instance().isDamageNumbersEnabled();
                        }
                        else if (i == 2) mPauseControlsOpen = true;
                        else if (i == 3) {
                            saveCurrentGame(0);
                            addFloatingText("Saved Game!", mPlayer->getPosition(), sf::Color::Green);
                            mPauseMenuOpen = false;
                        } else if (i == 4) mStateManager.changeState(std::make_unique<MainMenuState>(mStateManager, mWindow, mFont, mGameData), StateID::MainMenu);
                    }
                }
            }
            else if (mShopOpen) {
                for (int i = 0; i < 4; ++i) {
                    sf::FloatRect itemBounds({400.f - 160.f, 195.f + i * 68.f - 25.f}, {320.f, 50.f});
                    if (itemBounds.contains(mousePos)) {
                        if (i == 0) {
                            int currentPrice = mGameData.getCurrentShopPrice("potion", mPlayer->getLevel());
                            if (mPlayer->getGold() >= currentPrice) {
                                mPlayer->buyPotion();
                                AudioManager::instance().playSfx("purchase_success");
                                addFloatingText("+1 Potion", mPlayer->getPosition(), sf::Color::Red);
                            } else {
                                mShopErrorMessage = "Not enough gold!";
                                mShopErrorTimer = 1.5f;
                            }
                        } else if (i == 1) {
                            const ItemData& hpScroll = mGameData.getItem("scroll_hp");
                            int currentPrice = mGameData.getCurrentShopPrice("scroll_hp", mPlayer->getLevel());
                            if (mPlayer->getGold() >= currentPrice) {
                                mPlayer->gainGold(-currentPrice);
                                mPlayer->addHPScrollCount(1);
                                AudioManager::instance().playSfx("purchase_success");
                                addFloatingText("+" + std::to_string(hpScroll.effectValue) + "% Max HP!", mPlayer->getPosition(), sf::Color::Green);
                            } else {
                                mShopErrorMessage = "Not enough gold!";
                                mShopErrorTimer = 1.5f;
                            }
                        } else if (i == 2) {
                            const ItemData& atkScroll = mGameData.getItem("scroll_attack");
                            int currentPrice = mGameData.getCurrentShopPrice("scroll_attack", mPlayer->getLevel());
                            if (mPlayer->getGold() >= currentPrice) {
                                mPlayer->gainGold(-currentPrice);
                                mPlayer->addDamageScrollCount(1);
                                AudioManager::instance().playSfx("purchase_success");
                                addFloatingText("+" + std::to_string(atkScroll.effectValue) + "% Damage Buff!", mPlayer->getPosition(), sf::Color(255, 165, 0));
                            } else {
                                mShopErrorMessage = "Not enough gold!";
                                mShopErrorTimer = 1.5f;
                            }
                        } else if (i == 3) {
                            const ItemData& defScroll = mGameData.getItem("scroll_defense");
                            int currentPrice = mGameData.getCurrentShopPrice("scroll_defense", mPlayer->getLevel());
                            if (mPlayer->getGold() >= currentPrice) {
                                mPlayer->gainGold(-currentPrice);
                                mPlayer->addDefenseScrollCount(1);
                                AudioManager::instance().playSfx("purchase_success");
                                addFloatingText("+" + std::to_string(defScroll.effectValue) + "% Defense Buff!", mPlayer->getPosition(), sf::Color(100, 200, 255));
                            } else {
                                mShopErrorMessage = "Not enough gold!";
                                mShopErrorTimer = 1.5f;
                            }
                        }
                    }
                }
            }
        }
    }
    else if (const auto* mouseRel = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseRel->button == sf::Mouse::Button::Left) {
            mPauseSettingsDragging = false;
            mPauseSettingsDragIdx = -1;
        }
    }
}

void PlayingState::update(float dt) {
    updateCamera();
    mWindow.setView(mView);

    // Floating texts always animate
    for (auto& ft : mFloatingTexts) {
        ft.text.move(ft.velocity * dt);
        ft.lifetime -= dt;
    }
    std::erase_if(mFloatingTexts, [](const FloatingText& ft) { return ft.lifetime <= 0.f; });

    if (mShopErrorTimer > 0.f) {
        mShopErrorTimer -= dt;
    }

    if (!mShopOpen) {
        mShopHoveredIdx = -1;
    }

    if (mShopOpen || mPauseMenuOpen) return;

    mPlayTimer += dt;

    // Update Player
    mPlayer->update(dt, mProjectiles, mWindow, mEnemies);

    // Update enemy hover state based on mouse position
    sf::Vector2f mouseWorldPos = mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow), mView);
    for (auto& enemy : mEnemies) {
        if (enemy->isDead()) {
            enemy->setHovered(false);
            continue;
        }
        float dx = mouseWorldPos.x - enemy->getPosition().x;
        float dy = mouseWorldPos.y - enemy->getPosition().y;
        float dist = std::sqrt(dx * dx + dy * dy);
        // Boss uses larger effective radius for full-body targeting
        float effectiveRadius = enemy->getIsBoss() ? enemy->getRadius() * 3.0f : enemy->getRadius() + 15.f;
        enemy->setHovered(dist <= effectiveRadius);
    }

    // Update Projectiles
    for (auto& proj : mProjectiles) {
        proj.update(dt);
    }
    std::erase_if(mProjectiles, [](const Projectile& p) { return p.isExpired(); });

    // Update Enemies & AI
    int playerHP = mPlayer->getHP();
    int originalHP = playerHP;
    for (auto& enemy : mEnemies) {
        enemy->update(dt);
        enemy->updateAI(dt, mPlayer->getPosition(), playerHP, mProjectiles);
    }

    if (playerHP < originalHP) {
        int damageTaken = originalHP - playerHP;
        mPlayer->takeDamage(damageTaken);
        addFloatingText("-" + std::to_string(damageTaken), mPlayer->getPosition() - sf::Vector2f(0.f, 30.f), sf::Color::Red);
    }

    // Process collisions (Melee hits, projectile hits, loot collection, enemy death cleanup)
    handleCollisions(dt);

    if (mBossGoblinDead && mBossDemonDead && mBossBringerDead) {
        return; // Stop update loop completely to prevent any further logic (like SpawnSystem) during the victory frame
    }

    // Check if we need to resume battle music after boss death
    if (mWaitingForBossDeathSound) {
        if (!AudioManager::instance().isSfxPlaying("boss_death")) {
            mWaitingForBossDeathSound = false;
            AudioManager::instance().playBattleMusic();
        }
    }

    // Check if we need to respawn normal enemies after boss death
    if (mNeedsEnemyRespawn) {
        mNeedsEnemyRespawn = false;
        SpawnSystem::spawnInitialEnemies(mEnemies, mGameData, *this);
    }

    if (mPlayer->isDead()) {
        handlePlayerDeath();
    }

    SpawnSystem::update(dt, mPlayer->getLevel(), mEnemies, mRespawnQueue, mGameData, *this, mBossGoblinDead, mBossDemonDead, mBossBringerDead);

    updateCamera();
}

void PlayingState::draw(sf::RenderWindow& window) {
    sf::View oldView = window.getView();

    // === WORLD LAYER ===
    window.setView(mView);
    mMap.draw(window);

    // Shop zone indicator at center
    sf::CircleShape shopZone(60.f);
    shopZone.setOrigin({60.f, 60.f});
    shopZone.setPosition(mMap.getCenter());
    shopZone.setFillColor(sf::Color(255, 215, 0, 30));
    shopZone.setOutlineThickness(2.f);
    shopZone.setOutlineColor(sf::Color(255, 215, 0, 80));
    window.draw(shopZone);

    for (const auto& item : mLootItems) {
        window.draw(item.sprite);
    }
    for (auto& enemy : mEnemies) {
        enemy->draw(window);
    }
    mPlayer->draw(window);

    // Draw attack range indicator: thin white circle - ONLY on click (temporary)
    if (mPlayer->getAttackRangePreviewTimer() > 0.f) {
        float range = mPlayer->getAttackRange();
        sf::CircleShape rangeCircle(range);
        rangeCircle.setOrigin({range, range});
        rangeCircle.setPosition(mPlayer->getPosition());
        rangeCircle.setFillColor(sf::Color::Transparent);
        rangeCircle.setOutlineThickness(1.f);
        // Fade out as timer decreases
        float alpha = std::min(1.f, mPlayer->getAttackRangePreviewTimer() / 0.3f) * 80.f;
        rangeCircle.setOutlineColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha)));
        window.draw(rangeCircle);
    }

    for (auto& proj : mProjectiles) {
        proj.draw(window);
    }
    for (const auto& ft : mFloatingTexts) {
        window.draw(ft.text);
    }

    // === UI HUD LAYER ===
    sf::View uiView({400.f, 300.f}, {800.f, 600.f});
    window.setView(uiView);

    // HUD background
    sf::RectangleShape hudBg({800.f, 76.f});
    hudBg.setFillColor(sf::Color(10, 10, 15, 230));
    hudBg.setPosition({0.f, 0.f});
    window.draw(hudBg);

    // Boss HUD bar
    Enemy* activeBoss = nullptr;
    float closestBossDist = 99999.f;
    for (auto& enemy : mEnemies) {
        if (enemy->getIsBoss()) {
            float dx = enemy->getPosition().x - mPlayer->getPosition().x;
            float dy = enemy->getPosition().y - mPlayer->getPosition().y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < 500.f && dist < closestBossDist) {
                closestBossDist = dist;
                activeBoss = enemy.get();
            }
        }
    }

    if (activeBoss) {
        // Draw boss bar background
        sf::RectangleShape bossBg({400.f, 20.f});
        bossBg.setFillColor(sf::Color(30, 10, 10, 200));
        bossBg.setOutlineThickness(1.5f);
        bossBg.setOutlineColor(sf::Color::Red);
        bossBg.setOrigin({200.f, 10.f});
        bossBg.setPosition({400.f, 100.f});
        window.draw(bossBg);

        float bossHpPercent = std::max(0.f, static_cast<float>(activeBoss->getHP()) / activeBoss->getMaxHP());
        sf::RectangleShape bossFg({400.f * bossHpPercent, 20.f});
        bossFg.setFillColor(sf::Color(220, 0, 0));
        bossFg.setOrigin({200.f, 10.f});
        bossFg.setPosition({400.f, 100.f});
        window.draw(bossFg);

        sf::Text bossNameText(mFont, activeBoss->getName() + " [HP " + std::to_string(activeBoss->getHP()) + "/" + std::to_string(activeBoss->getMaxHP()) + "]", 13);
        bossNameText.setFillColor(sf::Color::White);
        auto bnb = bossNameText.getLocalBounds();
        bossNameText.setOrigin({bnb.size.x / 2.f, bnb.size.y / 2.f});
        bossNameText.setPosition({400.f, 100.f});
        window.draw(bossNameText);
    }

    // HP Bar
    float hpPercent = static_cast<float>(mPlayer->getHP()) / mPlayer->getMaxHP();
    sf::RectangleShape hpBarBg({220.f, 16.f});
    hpBarBg.setFillColor(sf::Color(50, 50, 50));
    hpBarBg.setPosition({10.f, 8.f});
    window.draw(hpBarBg);
    sf::RectangleShape hpBarFg({220.f * hpPercent, 16.f});
    hpBarFg.setFillColor(sf::Color(220, 30, 30));
    hpBarFg.setPosition({10.f, 8.f});
    window.draw(hpBarFg);

    sf::Text hpText(mFont, "HP " + std::to_string(mPlayer->getHP()) + "/" + std::to_string(mPlayer->getMaxHP()), 13);
    hpText.setFillColor(sf::Color::White);
    hpText.setPosition({15.f, 6.f});
    window.draw(hpText);

    // EXP Bar
    float expPercent = static_cast<float>(mPlayer->getEXP()) / (mPlayer->getLevel() * 100);
    sf::RectangleShape expBarBg({220.f, 8.f});
    expBarBg.setFillColor(sf::Color(30, 30, 60));
    expBarBg.setPosition({10.f, 26.f});
    window.draw(expBarBg);
    sf::RectangleShape expBarFg({220.f * expPercent, 8.f});
    expBarFg.setFillColor(sf::Color(80, 180, 255));
    expBarFg.setPosition({10.f, 26.f});
    window.draw(expBarFg);

    // Stats text line (LV / Lives / Gold / Potion / Scroll)
    sf::Text statsText(mFont, "LVL " + std::to_string(mPlayer->getLevel()) +
        "  Lives: " + std::to_string(mLives) +
        "  Gold: " + std::to_string(mPlayer->getGold()) +
        "  Potions[H]: " + std::to_string(mPlayer->getPotions()) +
        "  Scrolls: " + std::to_string(mInventorySkills), 13);
    statsText.setFillColor(sf::Color(220, 220, 220));
    statsText.setPosition({10.f, 37.f});
    window.draw(statsText);

    int dmgPercent = mPlayer->getDamageScrollCount() * 5;
    int defPercent = mPlayer->getDefenseScrollCount() * 12;
    sf::Text buffText(mFont, "DMG+ " + std::to_string(dmgPercent) + "%   DEF+ " + std::to_string(defPercent) + "%", 13);
    buffText.setFillColor(sf::Color(180, 230, 180));
    buffText.setPosition({10.f, 52.f});
    window.draw(buffText);

    // Skill cooldown indicators at bottom
    auto drawCooldownBox = [&](float x, const std::string& label, float cd, float cdMax, const std::string& texKey) {
        float size = 64.f;
        float topY = 500.f;

        if (mTextureHolder.has(texKey)) {
            sf::Sprite icon(mTextureHolder.get(texKey));
            auto b = icon.getLocalBounds();
            // Scale icon to fit 64x64 while preserving aspect ratio
            float scale = std::min(size / b.size.x, size / b.size.y);
            icon.setScale({scale, scale});
            icon.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
            icon.setPosition({x + 32.f, topY + 32.f}); // center of the 64x64 box
            window.draw(icon);
        }

        if (cd > 0.f) {
            // Dark circular overlay for cooldown
            float radius = size * 0.5f;
            sf::CircleShape overlay(radius);
            overlay.setOrigin({radius, radius});
            overlay.setPosition({x + 32.f, topY + 32.f}); // same center as icon
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(overlay);

            std::string txt = std::to_string((int)cd + 1) + "s";
            sf::Text text(mFont, txt, 18);
            text.setFillColor(sf::Color::White);
            text.setOutlineThickness(1.5f);
            text.setOutlineColor(sf::Color::Black);
            auto b = text.getLocalBounds();
            text.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
            text.setPosition({x + 32.f, topY + 32.f});
            window.draw(text);
        }

        // Label text (Q, E, R, DASH)
        sf::Text text(mFont, label, 12);
        text.setFillColor(sf::Color::White);
        text.setOutlineThickness(1.f);
        text.setOutlineColor(sf::Color::Black);
        auto b = text.getLocalBounds();
        text.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
        text.setPosition({x + 32.f, topY + size + 8.f});
        window.draw(text);
    };

    drawCooldownBox(190.f, "DASH", mPlayer->getDashCooldown(), mPlayer->getDashCooldownMax(), "ui_skill_dash");
    drawCooldownBox(260.f, "Q", mPlayer->getQCooldown(), mPlayer->getQCooldownMax(), "ui_skill_q");
    drawCooldownBox(330.f, "E", mPlayer->getECooldown(), mPlayer->getECooldownMax(), "ui_skill_e");
    drawCooldownBox(400.f, "R", mPlayer->getRCooldown(), mPlayer->getRCooldownMax(), "ui_skill_r");

    // Play Timer at top right
    int totalSec = static_cast<int>(mPlayTimer);
    int mins = totalSec / 60;
    int secs = totalSec % 60;
    std::string timerStr = (mins < 10 ? "0" : "") + std::to_string(mins) + ":" + (secs < 10 ? "0" : "") + std::to_string(secs);
    sf::Text timerText(mFont, timerStr, 16);
    timerText.setFillColor(sf::Color(230, 230, 230));
    auto tb = timerText.getLocalBounds();
    timerText.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
    timerText.setPosition({750.f, 38.f});
    window.draw(timerText);

    // Draw pause button on the left of the timer
    if (mHasPauseBtn && mPauseBtnSprite) {
        auto pSize = mPauseBtnTex.getSize();
        float targetSize = 24.f;
        mPauseBtnSprite->setScale({targetSize / pSize.x, targetSize / pSize.y});
        float btnCenterX = 750.f - tb.size.x / 2.f - 20.f;
        mPauseBtnSprite->setPosition({btnCenterX, 38.f});
        mPauseBtnSprite->setColor(mPauseBtnHovered ? sf::Color(255, 255, 180) : sf::Color::White);
        window.draw(*mPauseBtnSprite);
    }

    // Shop prompt
    sf::Vector2f centerPos = mMap.getCenter();
    float dx = mPlayer->getPosition().x - centerPos.x;
    float dy = mPlayer->getPosition().y - centerPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 180.f && !mShopOpen && !mPauseMenuOpen) {
        sf::Text shopPrompt(mFont, "Press [TAB] to open Shop", 20);
        shopPrompt.setFillColor(sf::Color::Yellow);
        shopPrompt.setOutlineThickness(1.5f);
        shopPrompt.setOutlineColor(sf::Color::Black);
        auto bounds = shopPrompt.getLocalBounds();
        shopPrompt.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
        shopPrompt.setPosition({400.f, 450.f});
        window.draw(shopPrompt);
    }

    // === MINIMAP OVERLAY (Bottom-Left Corner) ===
    float miniSize = 150.f;

    sf::FloatRect miniPort({20.f / 800.f, (600.f - miniSize - 20.f) / 600.f}, {miniSize / 800.f, miniSize / 600.f});
    mMinimapView.setViewport(miniPort);
    mMinimapView.setCenter(mPlayer->getPosition());
    mMinimapView.setSize({2200.f, 2200.f});
    window.setView(mMinimapView);

    mMap.draw(window);

    // Draw player dot on minimap
    sf::CircleShape playerDot(50.f);
    playerDot.setFillColor(sf::Color::Cyan);
    playerDot.setOrigin({50.f, 50.f});
    playerDot.setPosition(mPlayer->getPosition());
    window.draw(playerDot);

    // Draw enemy dots on minimap
    for (const auto& enemy : mEnemies) {
        sf::CircleShape icon(enemy->getIsBoss() ? 90.f : 40.f);
        icon.setFillColor(enemy->getIsBoss() ? sf::Color::Yellow : sf::Color::Red);
        icon.setOrigin({icon.getRadius(), icon.getRadius()});
        icon.setPosition(enemy->getPosition());
        window.draw(icon);
    }

    // Minimap border frame
    window.setView(uiView);
    sf::RectangleShape miniBorder({miniSize, miniSize});
    miniBorder.setPosition({20.f, 600.f - miniSize - 20.f});
    miniBorder.setFillColor(sf::Color(10, 10, 20, 180));
    miniBorder.setOutlineThickness(3.f);
    miniBorder.setOutlineColor(sf::Color(255, 215, 0));
    window.draw(miniBorder);

    // === SHOP OVERLAY ===
    if (mShopOpen) {
        sf::RectangleShape shopOverlay({800.f, 600.f});
        shopOverlay.setFillColor(sf::Color(0, 0, 0, 170));
        window.draw(shopOverlay);

        sf::RectangleShape shopPanel({480.f, 440.f});
        shopPanel.setFillColor(sf::Color(25, 25, 35, 240));
        shopPanel.setOutlineThickness(3.f);
        shopPanel.setOutlineColor(sf::Color::Yellow);
        shopPanel.setOrigin({240.f, 220.f});
        shopPanel.setPosition({400.f, 300.f});
        window.draw(shopPanel);

        sf::Text shopTitle(mFont, "BASE SHOP", 30);
        shopTitle.setFillColor(sf::Color(255, 215, 0));
        auto stb = shopTitle.getLocalBounds();
        shopTitle.setOrigin({stb.size.x / 2.f, stb.size.y / 2.f});
        shopTitle.setPosition({400.f, 110.f});
        window.draw(shopTitle);

        sf::Text goldInfo(mFont, "Gold: " + std::to_string(mPlayer->getGold()) + "g   Potions: " + std::to_string(mPlayer->getPotions()), 18);
        goldInfo.setFillColor(sf::Color::White);
        auto gib = goldInfo.getLocalBounds();
        goldInfo.setOrigin({gib.size.x / 2.f, gib.size.y / 2.f});
        goldInfo.setPosition({400.f, 145.f});
        window.draw(goldInfo);


        int pLevel = mPlayer->getLevel();
        std::vector<std::string> shopItems = {
            "Buy Health Potion (+50 HP) - " + std::to_string(mGameData.getCurrentShopPrice("potion", pLevel)) + " Gold",
            "Buy HP Scroll (+8% Max HP) - " + std::to_string(mGameData.getCurrentShopPrice("scroll_hp", pLevel)) + " Gold",
            "Buy Damage Scroll (+5% DMG) - " + std::to_string(mGameData.getCurrentShopPrice("scroll_attack", pLevel)) + " Gold",
            "Buy Defense Scroll (+12% DEF) - " + std::to_string(mGameData.getCurrentShopPrice("scroll_defense", pLevel)) + " Gold"
        };

        for (int i = 0; i < 4; ++i) {
            float yPos = 195.f + i * 68.f;
            sf::Text itemText(mFont, shopItems[i], 16);
            
            bool isHovered = (i == mShopHoveredIdx);
            if (isHovered) {
                itemText.setFillColor(sf::Color::Yellow);
                itemText.setScale({1.05f, 1.05f});
            } else {
                itemText.setFillColor(sf::Color::White);
                itemText.setScale({1.f, 1.f});
            }
            auto ib = itemText.getLocalBounds();
            itemText.setOrigin({ib.size.x / 2.f, ib.size.y / 2.f});
            itemText.setPosition({400.f, yPos});

            if (mHasBtnTex) {
                sf::Sprite btnSprite(mBtnTexture);
                auto btnSize = mBtnTexture.getSize();
                btnSprite.setOrigin({btnSize.x / 2.f, btnSize.y / 2.f});
                if (isHovered) {
                    btnSprite.setScale({357.f / btnSize.x, 57.8f / btnSize.y});
                    btnSprite.setPosition({400.f, yPos});
                    btnSprite.setColor(sf::Color(255, 255, 180));
                } else {
                    btnSprite.setScale({340.f / btnSize.x, 55.f / btnSize.y});
                    btnSprite.setPosition({400.f, yPos});
                    btnSprite.setColor(sf::Color(180, 180, 220));
                }
                window.draw(btnSprite);
            }

            std::string iconKey = (i == 0) ? "potion" : (i == 1) ? "scroll_hp" : (i == 2) ? "scroll_attack" : "scroll_defense";
            if (mTextureHolder.has(iconKey)) {
                sf::Sprite itemIcon(mTextureHolder.get(iconKey));
                auto isz = itemIcon.getTexture().getSize();
                itemIcon.setOrigin({isz.x / 2.f, isz.y / 2.f});
                itemIcon.setPosition({250.f, yPos});
                if (isHovered) {
                    itemIcon.setScale({25.2f / isz.x, 25.2f / isz.y});
                } else {
                    itemIcon.setScale({24.f / isz.x, 24.f / isz.y});
                }
                window.draw(itemIcon);
            }

            window.draw(itemText);
        }

        if (mShopErrorTimer > 0.f) {
            sf::Text errorText(mFont, mShopErrorMessage, 16);
            errorText.setFillColor(sf::Color::Red);
            errorText.setOutlineThickness(1.2f);
            errorText.setOutlineColor(sf::Color::Black);
            auto eb = errorText.getLocalBounds();
            errorText.setOrigin({eb.size.x / 2.f, eb.size.y / 2.f});
            errorText.setPosition({400.f, 445.f});
            window.draw(errorText);
        }

        sf::Text shopExit(mFont, "Press [TAB] or [ESC] to Exit", 14);
        shopExit.setFillColor(sf::Color(180, 180, 180));
        auto seb = shopExit.getLocalBounds();
        shopExit.setOrigin({seb.size.x / 2.f, seb.size.y / 2.f});
        shopExit.setPosition({400.f, 485.f});
        window.draw(shopExit);
    }

    // === PAUSE MENU OVERLAY ===
    if (mPauseMenuOpen) {
        sf::RectangleShape pauseOverlay({800.f, 600.f});
        pauseOverlay.setFillColor(sf::Color(0, 0, 0, 190));
        window.draw(pauseOverlay);

        if (mPauseSettingsOpen) {
            sf::RectangleShape panelBg({500.f, 450.f});
            panelBg.setFillColor(sf::Color(10, 10, 20, 230));
            panelBg.setOutlineThickness(2.f);
            panelBg.setOutlineColor(sf::Color(255, 215, 0, 180));
            panelBg.setPosition({150.f, 75.f});
            window.draw(panelBg);

            // Title
            sf::Text titleText(mFont, "SETTING", 32);
            titleText.setFillColor(sf::Color(255, 215, 0));
            titleText.setOutlineThickness(2.f);
            titleText.setOutlineColor(sf::Color::Black);
            auto tb = titleText.getLocalBounds();
            titleText.setOrigin({tb.size.x / 2.f, tb.size.y / 2.f});
            titleText.setPosition({400.f, 110.f});
            window.draw(titleText);

            std::string labels[] = {"MASTER VOLUME", "MUSIC VOLUME", "SFX VOLUME", "FULLSCREEN", "DAMAGE NUMBERS", "APPLY", "BACK"};
            for (int i = 0; i < 7; ++i) {
                float itemY = 160.f + i * 55.f;
                bool highlight = false;
                if (mPauseSettingsHoveredIdx != -1) {
                    highlight = (i == mPauseSettingsHoveredIdx);
                } else if (mPauseSettingsSelectionActive) {
                    highlight = (i == mPauseSettingsSelection);
                }

                sf::Text label(mFont, labels[i], 18);
                label.setFillColor(highlight ? sf::Color::Yellow : sf::Color(200, 200, 200));
                label.setPosition({220.f, itemY - 8.f});

                if (i < 3) {
                    window.draw(label);
                    float val = (i == 0) ? mPauseSettingsMaster : (i == 1) ? mPauseSettingsMusic : mPauseSettingsSfx;
                    float sliderLeft = 410.f;
                    float sliderWidth = 170.f;

                    sf::RectangleShape track({sliderWidth, 6.f});
                    track.setFillColor(sf::Color(50, 50, 60));
                    track.setPosition({sliderLeft, itemY});
                    window.draw(track);

                    float fillWidth = (val / 100.f) * sliderWidth;
                    sf::RectangleShape fill({fillWidth, 6.f});
                    fill.setFillColor(highlight ? sf::Color(255, 215, 0) : sf::Color(180, 140, 50));
                    fill.setPosition({sliderLeft, itemY});
                    window.draw(fill);

                    sf::CircleShape knob(7.f);
                    knob.setOrigin({7.f, 7.f});
                    knob.setPosition({sliderLeft + fillWidth, itemY + 3.f});
                    knob.setFillColor(highlight ? sf::Color::Yellow : sf::Color::White);
                    window.draw(knob);

                    sf::Text valText(mFont, std::to_string(static_cast<int>(val)), 14);
                    valText.setFillColor(sf::Color::White);
                    valText.setPosition({sliderLeft + sliderWidth + 10.f, itemY - 6.f});
                    window.draw(valText);
                } else if (i == 3 || i == 4) {
                    window.draw(label);
                    bool val = (i == 3) ? mPauseSettingsFullscreen : mPauseSettingsDamageNumbers;
                    std::string toggleStr = val ? "ON" : "OFF";

                    sf::Text toggleText(mFont, toggleStr, 18);
                    toggleText.setFillColor(val ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
                    toggleText.setPosition({530.f, itemY - 8.f});
                    window.draw(toggleText);

                    sf::Text openBracket(mFont, "[", 18);
                    openBracket.setFillColor(highlight ? sf::Color::Yellow : sf::Color(150, 150, 150));
                    openBracket.setPosition({510.f, itemY - 8.f});
                    window.draw(openBracket);

                    sf::Text closeBracket(mFont, "]", 18);
                    closeBracket.setFillColor(highlight ? sf::Color::Yellow : sf::Color(150, 150, 150));
                    float closeX = 530.f + (toggleText.findCharacterPos(toggleStr.size()).x - toggleText.findCharacterPos(0).x) + 8.f;
                    closeBracket.setPosition({closeX, itemY - 8.f});
                    window.draw(closeBracket);
                } else {
                    if (mHasBtnTex) {
                        sf::Sprite btnSprite(mBtnTexture);
                        auto btnSize = mBtnTexture.getSize();
                        btnSprite.setOrigin({btnSize.x / 2.f, btnSize.y / 2.f});
                        btnSprite.setScale({180.f / btnSize.x, 45.f / btnSize.y});
                        btnSprite.setPosition({400.f, itemY});
                        btnSprite.setColor(highlight ? sf::Color(255, 255, 180) : sf::Color(150, 150, 150));
                        window.draw(btnSprite);
                    }
                    label.setCharacterSize(20);
                    auto lb = label.getLocalBounds();
                    label.setOrigin({lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f});
                    label.setPosition({400.f, itemY});
                    label.setFillColor(highlight ? sf::Color::Yellow : sf::Color::White);
                    window.draw(label);
                }
            }
        }
        else if (mPauseControlsOpen) {
            sf::RectangleShape panelBg({500.f, 400.f});
            panelBg.setFillColor(sf::Color(10, 10, 20, 230));
            panelBg.setOutlineThickness(2.f);
            panelBg.setOutlineColor(sf::Color(255, 215, 0, 180));
            panelBg.setPosition({150.f, 100.f});
            window.draw(panelBg);
            
            sf::Text cTitle(mFont, "CONTROL", 24);
            cTitle.setFillColor(sf::Color(255, 215, 0));
            auto ctb = cTitle.getLocalBounds();
            cTitle.setOrigin({ctb.size.x / 2.f, ctb.size.y / 2.f});
            cTitle.setPosition({400.f, 140.f});
            window.draw(cTitle);

            struct ControlBind {
                std::string key;
                std::string desc;
            };
            std::vector<ControlBind> binds = {
                {"W A S D", " - Move"},
                {"SPACE", " - Dash"},
                {"LMB", " - Attack"},
                {"Q", " - Skill Q"},
                {"E", " - Skill E"},
                {"R", " - Skill R"},
                {"H", " - Use Potion"},
                {"TAB", " - Open Shop"},
                {"ESC", " - Pause"}
            };

            for (int i = 0; i < 9; ++i) {
                float startX = (i < 5) ? 200.f : 430.f;
                float descX = (i < 5) ? 285.f : 515.f;
                float yVal = 180.f + ((i < 5) ? i : (i - 5)) * 26.f;

                // Key (Bold & Yellow)
                sf::Text keyText(mFont, binds[i].key, 16);
                keyText.setFillColor(sf::Color::Yellow);
                keyText.setStyle(sf::Text::Bold);
                keyText.setPosition({startX, yVal});
                window.draw(keyText);

                // Description (Normal & Gray)
                sf::Text descText(mFont, binds[i].desc, 16);
                descText.setFillColor(sf::Color(220, 220, 220));
                descText.setStyle(sf::Text::Regular);
                descText.setPosition({descX, yVal});
                window.draw(descText);
            }

            if (mHasBtnTex) {
                sf::Sprite btnSprite(mBtnTexture);
                auto btnSize = mBtnTexture.getSize();
                btnSprite.setOrigin({btnSize.x / 2.f, btnSize.y / 2.f});
                btnSprite.setScale({160.f / btnSize.x, 40.f / btnSize.y});
                btnSprite.setPosition({400.f, 450.f});
                btnSprite.setColor(mPauseControlsBackHovered ? sf::Color(255, 255, 180) : sf::Color(150, 150, 150));
                window.draw(btnSprite);
            }
            sf::Text bText(mFont, "BACK", 18);
            bText.setFillColor(mPauseControlsBackHovered ? sf::Color::Yellow : sf::Color::White);
            auto bb = bText.getLocalBounds();
            bText.setOrigin({bb.position.x + bb.size.x / 2.f, bb.position.y + bb.size.y / 2.f});
            bText.setPosition({400.f, 450.f});
            window.draw(bText);
        }
        else {
            sf::Text pauseTitle(mFont, "GAME PAUSED", 36);
            pauseTitle.setFillColor(sf::Color(255, 215, 0));
            auto pt = pauseTitle.getLocalBounds();
            pauseTitle.setOrigin({pt.size.x / 2.f, pt.size.y / 2.f});
            pauseTitle.setPosition({400.f, 90.f});
            window.draw(pauseTitle);

            std::string pauseOptions[5] = {
                "Resume Game",
                "Setting",
                "Control",
                "Save Game",
                "Quit to Main Menu"
            };

            for (int i = 0; i < 5; i++) {
                bool highlight = false;
                if (mPauseHoveredIdx != -1) {
                    highlight = (i == mPauseHoveredIdx);
                } else if (mPauseSelectionActive) {
                    highlight = (i == mPauseSelection);
                }

                sf::Text item(mFont, pauseOptions[i], 22);
                item.setFillColor(highlight ? sf::Color::Yellow : sf::Color::White);
                auto ib = item.getLocalBounds();
                item.setOrigin({ ib.position.x + ib.size.x / 2.f, ib.position.y + ib.size.y / 2.f });
                item.setPosition({ 400.f, 170.f + i * 70.f });

                if (mHasBtnTex) {
                    sf::Sprite btnSprite(mBtnTexture);
                    auto btnSize = mBtnTexture.getSize();
                    btnSprite.setOrigin({btnSize.x / 2.f, btnSize.y / 2.f});
                    btnSprite.setScale({240.f / btnSize.x, 50.f / btnSize.y});
                    btnSprite.setPosition({ 400.f, 170.f + i * 70.f });
                    btnSprite.setColor(highlight ? sf::Color(255, 255, 180) : sf::Color(180, 180, 180));
                    window.draw(btnSprite);
                }
                window.draw(item);
            }
        }
    }
}

void PlayingState::updateCamera() {
    sf::Vector2f playerPos = mPlayer->getPosition();
    float viewX = std::max(400.f, std::min(3600.f, playerPos.x));
    float viewY = std::max(300.f, std::min(3700.f, playerPos.y));
    mView.setCenter({viewX, viewY});
}

void PlayingState::spawnInitialEnemies() {
    SpawnSystem::spawnInitialEnemies(mEnemies, mGameData, *this);
}

std::string PlayingState::selectRandomMonsterForLevel(int playerLevel) {
    std::vector<std::string> allowed;

    // Level 1 is always allowed for levels < 29
    if (playerLevel < 29) {
        allowed.push_back("Skeleton Archer");
        allowed.push_back("Bat");
        allowed.push_back("Skeleton");
    }

    // Level 2 allowed if playerLevel >= 5 AND Goblin Giant is dead
    if (playerLevel >= 5 && mBossGoblinDead && playerLevel < 29) {
        allowed.push_back("Werewolf");
        allowed.push_back("Slime");
        allowed.push_back("Armored Skeleton");
        allowed.push_back("Orc");
    }

    // Level 3 allowed if playerLevel >= 13 AND Demon is dead
    if (playerLevel >= 13 && mBossDemonDead && playerLevel < 29) {
        allowed.push_back("Elite Orc");
        allowed.push_back("Orc rider");
        allowed.push_back("Greatsword Skeleton");
        allowed.push_back("Necromancer");
        allowed.push_back("Werebear");
    }

    if (allowed.empty()) {
        allowed.push_back("Skeleton");
        allowed.push_back("Bat");
        allowed.push_back("Slime");
        allowed.push_back("Orc");
        allowed.push_back("Elite Orc");
    }
    return allowed[randomInt(0, static_cast<int>(allowed.size()) - 1)];
}

void PlayingState::addFloatingText(std::string textStr, sf::Vector2f position, sf::Color color) {
    if (!AudioManager::instance().isDamageNumbersEnabled()) {
        bool isDamage = false;
        if (textStr.find("CRIT ") == 0) isDamage = true;
        else if (!textStr.empty() && std::isdigit(textStr[0])) isDamage = true;
        else if (textStr.length() > 1 && textStr[0] == '-' && std::isdigit(textStr[1]) && textStr.find("%") == std::string::npos) isDamage = true;
        
        if (isDamage) return;
    }
    mFloatingTexts.push_back(FloatingText(mFont, textStr, position, color, {0.f, -70.f}, 0.8f));
}

void PlayingState::spawnLoot(LootType type, sf::Vector2f position, int value) {
    LootSystem::spawnLoot(type, position, value, *this);
}

void PlayingState::setupEnemyAnimations(Enemy* enemy) {
    if (!enemy) return;
    // eKey dạng "Monster/Skeleton_Level_1/skeleton" (quái thường) hoặc
    // "Boss/Bringer Of Death/bringerOfDeath" (boss).
    std::string eKey = enemyKeyFromName(enemy->getName());

    // Tách thư mục và tiền tố tên file (đoạn cuối cùng sau dấu '/').
    std::string dir = eKey;
    std::string prefix = eKey;
    std::size_t slashPos = eKey.find_last_of('/');
    if (slashPos != std::string::npos) {
        dir = eKey.substr(0, slashPos);
        prefix = eKey.substr(slashPos + 1);
    }
    auto tryLoadState = [&](const std::string& stateFolder, const std::string& stateLower) {
        // 1) Quái thường: asset là 1 sprite sheet gộp duy nhất, vd
        //    "assets/textures/enemy/Monster/Skeleton_Level_1/skeleton_idle.png"
        std::string sheetPath = "assets/textures/enemy/" + eKey + "_" + stateLower + ".png";
        if (!std::filesystem::exists(sheetPath) && (stateLower == "idle" || stateLower == "walk")) {
            std::string flyPath = "assets/textures/enemy/" + eKey + "_flying.png";
            if (std::filesystem::exists(flyPath)) sheetPath = flyPath;
        }

        std::string sheetId = "e_" + eKey + "_" + stateLower + "_sheet";
        if (std::filesystem::exists(sheetPath)) {
            if (!mTextureHolder.has(sheetId)) {
                mTextureHolder.load(sheetId, sheetPath);
            }
            enemy->addSpriteSheetFrames(&mTextureHolder.get(sheetId), stateLower);
            return;
        }

        // 2) Boss: nhiều file riêng lẻ, nằm trong thư mục con theo trạng thái
        for (int i = 0; i < 30; ++i) { // Increase max frame count to 30 for Goblin Giant
            std::string path = "assets/textures/enemy/" + dir + "/" + stateFolder + "/" +
                                prefix + "_" + stateLower + "_" + std::to_string(i) + ".png";
            if (std::filesystem::exists(path)) {
                std::string tId = "e_" + eKey + "_" + stateLower + "_" + std::to_string(i);
                if (!mTextureHolder.has(tId)) {
                    mTextureHolder.load(tId, path);
                }
                if (stateLower == "idle") enemy->addIdleFrame(&mTextureHolder.get(tId));
                else if (stateLower == "walk") enemy->addWalkFrame(&mTextureHolder.get(tId));
                else if (stateLower == "attack") enemy->addAttackFrame(&mTextureHolder.get(tId));
                else if (stateLower == "hurt") enemy->addHurtFrame(&mTextureHolder.get(tId));
                else if (stateLower == "death") enemy->addDeathFrame(&mTextureHolder.get(tId));
                else if (stateLower == "skill_1") enemy->addSkill1Frame(&mTextureHolder.get(tId));
                else if (stateLower == "skill_2") enemy->addSkill2Frame(&mTextureHolder.get(tId));
                else if (stateLower == "skill") enemy->addSkill1Frame(&mTextureHolder.get(tId));
            } else {
                break;
            }
        }
    };

    tryLoadState("Idle", "idle");
    tryLoadState("Walk", "walk");
    tryLoadState("Attack", "attack");
    tryLoadState("Hurt", "hurt");
    tryLoadState("Death", "death");
    tryLoadState("Skill", "skill");
    tryLoadState("Skill_1", "skill_1");
    tryLoadState("Skill_2", "skill_2");
}

void PlayingState::applyDeathPenalty() {
    if (!mPlayer) return;
    int currentGold = mPlayer->getGold();
    int currentEXP = mPlayer->getEXP();

    int goldLost = static_cast<int>(currentGold * mGameData.getDeathGoldPenalty());
    int expLost = static_cast<int>(currentEXP * mGameData.getDeathExpPenalty());

    mPlayer->setGold(std::max(0, currentGold - goldLost));
    mPlayer->setEXP(std::max(0, currentEXP - expLost));

    if (goldLost > 0 || expLost > 0) {
        addFloatingText("-15% Gold & EXP Penalty!", mPlayer->getPosition() - sf::Vector2f(0.f, 50.f), sf::Color::Yellow);
    }
}

void PlayingState::handlePlayerDeath() {
    if (!mPlayer || mIsProcessingDeath) return;
    mIsProcessingDeath = true;

    AudioManager::instance().playSfx("player_death");

    mLives--;
    applyDeathPenalty();

    if (mLives <= 0) {
        mLives = 0;
        AudioManager::instance().stopMusic();
        mStateManager.changeState(std::make_unique<GameOverState>(mStateManager, mWindow, mFont, mGameData), StateID::GameOver);
        return;
    }

    // Respawn with lives remaining
    sf::Vector2f respawnPos = mMap.getCenter();
    if (mBossEncounterActive) {
        respawnPos = mBossCheckpointPos;
        // Reset active boss HP and AI state
        for (auto& enemy : mEnemies) {
            if (enemy->getIsBoss() && !enemy->isDead()) {
                enemy->resetBossState();
            }
        }
    }

    // Clear active projectiles to avoid immediate hit loop
    mProjectiles.clear();

    // Reset player HP, position, and combat flags
    mPlayer->setHP(mPlayer->getMaxHP());
    mPlayer->setPosition(respawnPos);
    mPlayer->setAttackTimer(0.f);
    mPlayer->setShieldActiveTimer(0.f);
    mPlayer->setWhirlwindTimer(0.f);
    mPlayer->setPendingSkillEProjectile(false);
    mPlayer->setPendingSkillRProjectile(false);
    mPlayer->setInvulnerable(1.5f);

    updateCamera();
    addFloatingText("RESPAWNED! Lives Remaining: " + std::to_string(mLives), respawnPos - sf::Vector2f(0.f, 40.f), sf::Color::Green);

    mIsProcessingDeath = false;
}

void PlayingState::saveCurrentGame(int slot) {
    SaveData data;
    data.isValid = true;
    data.characterType = mPlayer->getType();
    data.hp = mPlayer->getHP();
    data.maxHP = mPlayer->getMaxHP();
    data.level = mPlayer->getLevel();
    data.exp = mPlayer->getEXP();
    data.gold = mPlayer->getGold();
    data.potions = mPlayer->getPotions();
    data.inventorySkills = mInventorySkills;
    data.posX = mPlayer->getPosition().x;
    data.posY = mPlayer->getPosition().y;
    data.bossGoblinDead = mBossGoblinDead;
    data.bossDemonDead = mBossDemonDead;
    data.bossBringerDead = mBossBringerDead;
    data.hpScrollCount = mPlayer->getHPScrollCount();
    data.damageScrollCount = mPlayer->getDamageScrollCount();
    data.defenseScrollCount = mPlayer->getDefenseScrollCount();
    SaveManager::saveGame(slot, data);
}

bool PlayingState::checkVictory() {
    if (mBossGoblinDead && mBossDemonDead && mBossBringerDead) {
        mStateManager.changeState(std::make_unique<VictoryState>(mStateManager, mWindow, mFont, mGameData), StateID::Victory);
        return true;
    }
    return false;
}

void PlayingState::handleCollisions(float dt) {
    if (mPlayer->isDead()) return;

    // 1-3. Collision detection and damage application
    CollisionSystem::update(dt, *this);

    // 4. Loot Item Gathering
    LootSystem::update(dt, *this);

    // 5. Enemy Death, Loot Drop, Boss Kill Tracking
    auto it = mEnemies.begin();
    while (it != mEnemies.end()) {
        if ((*it)->isDead()) {
            std::cout << (*it)->getName() << " defeated!" << std::endl;
            int exp = (*it)->getEXPValue();
            mPlayer->gainEXP(exp);
            addFloatingText("+" + std::to_string(exp) + "xp", mPlayer->getPosition() - sf::Vector2f(0.f, 35.f), sf::Color(100, 255, 100));

            int goldVal = (*it)->getGoldValue();
            spawnLoot(LootType::Gold, (*it)->getPosition(), goldVal);

            const auto& table = mGameData.getLootTable((*it)->getIsBoss() ? "boss" : "default");
            for (const auto& entry : table) {
                if (entry.itemId == "gold") continue;
                if (randomChance(entry.dropChance)) {
                    if (entry.itemId == "potion") {
                        spawnLoot(LootType::Potion, (*it)->getPosition() + sf::Vector2f(15.f, 15.f));
                    } else if (entry.itemId == "scroll") {
                        spawnLoot(LootType::SkillScroll, (*it)->getPosition() - sf::Vector2f(15.f, 15.f));
                    }
                }
            }

            // Track Boss Kills
            if ((*it)->getIsBoss()) {
                AudioManager::instance().playSfx("boss_death");
                std::string bossName = (*it)->getName();
                if (bossName == "Goblin Giant") mBossGoblinDead = true;
                else if (bossName == "Demon") mBossDemonDead = true;
                else if (bossName == "Bringer Of Death") mBossBringerDead = true;

                addFloatingText("BOSS DEFEATED!", (*it)->getPosition(), sf::Color(255, 215, 0));
                if (checkVictory()) {
                    return; // Stop processing collisions and exit frame immediately
                }

                mBossEncounterActive = false;
                mWaitingForBossDeathSound = true;
                if (!mPlayer->isDead() && !mBossBringerDead) {
                    mNeedsEnemyRespawn = true;
                }
            } else {
                AudioManager::instance().playSfx("enemy_death");
                mRespawnQueue.push_back({(*it)->getName(), (*it)->getRegion(), 1, (*it)->getPosition(), mGameData.getRespawnTime()});
            }

            it = mEnemies.erase(it);
        } else {
            ++it;
        }
    }
}

int PlayingState::randomInt(int min, int max) const {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(mRng);
}

bool PlayingState::randomChance(int percent) const {
    if (percent <= 0) return false;
    if (percent >= 100) return true;
    std::uniform_int_distribution<int> dist(0, 99);
    return dist(mRng) < percent;
}
