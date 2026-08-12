#include "entities/Player.h"
#include "entities/Enemy.h"
#include "character/CharacterUtil.h"
#include "skills/SkillRegistry.h"
#include "core/AudioManager.h"

// Lấy dữ liệu cấu hình từ GameData dựa theo loại nhân vật (mType). Nạp tốc độ di chuyển, máu tối đa và thời gian hồi chiêu gốc của các kỹ năng Q, E, R
Player::Player(CharacterType type, const GameData& gameData) : mGameData(gameData), mType(type) {
    const CharacterData& data = mGameData.getCharacter(mType);
    mSpeed = data.baseSpeed;
    mMaxHP = data.baseHp;
    mQCooldownMax = data.qCooldown;
    mECooldownMax = data.eCooldown;
    mRCooldownMax = data.rCooldown;
    mHP = mMaxHP;

    // Khởi tạo hình tròn mặc định (mShape) đại diện cho Player (dùng khi chưa nạp Sprite) với bán kính 20px
    float radius = 20.f;
    mShape.setRadius(radius);
    mShape.setOrigin({radius, radius});
    sf::Color primaryColor = sf::Color::Blue;
    switch (mType) {
        case CharacterType::Knight: primaryColor = sf::Color::Blue; break;
        case CharacterType::Archer: primaryColor = sf::Color(50, 180, 50); break;
        case CharacterType::Lancer:   primaryColor = sf::Color(120, 120, 120); break;
        case CharacterType::Swordsman: primaryColor = sf::Color(220, 50, 50); break;
    }
    mShape.setFillColor(primaryColor);

    // Thiết lập các hình ảnh đồ họa bổ trợ
    mMeleeSlashShape.setSize({50.f, 30.f});
    mMeleeSlashShape.setFillColor(sf::Color(255, 255, 255, 150));
    mMeleeSlashShape.setOrigin({0.f, 15.f});

    mShieldShape.setRadius(32.f);
    mShieldShape.setOrigin({32.f, 32.f});
    mShieldShape.setFillColor(sf::Color::Transparent);
    mShieldShape.setOutlineThickness(3.f);
    mShieldShape.setOutlineColor(primaryColor);

    mWhirlwindShape.setRadius(75.f);
    mWhirlwindShape.setOrigin({75.f, 75.f});
    mWhirlwindShape.setFillColor(sf::Color(primaryColor.r, primaryColor.g, primaryColor.b, 50));
    mWhirlwindShape.setOutlineThickness(2.f);
    mWhirlwindShape.setOutlineColor(primaryColor);
}

void Player::update(float dt) { (void)dt; } // Hàm ghi đè từ Entity, không dùng đến

// Vòng lặp cập nhật chính của Player chạy mỗi khung hình
void Player::update(float dt, std::vector<Projectile>& projectiles, sf::RenderWindow& window, const std::vector<std::unique_ptr<Enemy>>& enemies) {
    // Đếm ngược toàn bộ bộ đếm thời gian (Timers & Cooldowns)
    if (mDashTimer > 0.f) mDashTimer -= dt;
    if (mDashCooldown > 0.f) mDashCooldown -= dt;
    if (mAttackTimer > 0.f) mAttackTimer -= dt;
    if (mAttackCooldown > 0.f) mAttackCooldown -= dt;
    if (mQCooldown > 0.f) mQCooldown -= dt;
    if (mECooldown > 0.f) mECooldown -= dt;
    if (mRCooldown > 0.f) mRCooldown -= dt;
    if (mShieldActiveTimer > 0.f) mShieldActiveTimer -= dt;
    if (mWhirlwindTimer > 0.f) mWhirlwindTimer -= dt;
    if (mHitFlashTimer > 0.f) mHitFlashTimer -= dt;
    if (mInvulnerableTimer > 0.f) mInvulnerableTimer -= dt;
    if (mSkillAnimTimer > 0.f) mSkillAnimTimer -= dt;
    if (mArcherAtkSpeedBuffTimer > 0.f) mArcherAtkSpeedBuffTimer -= dt;
    if (mSwordsmanAtkSpeedBuffTimer > 0.f) mSwordsmanAtkSpeedBuffTimer -= dt;
    if (mLancerSpeedBuffTimer > 0.f) mLancerSpeedBuffTimer -= dt;
    if (mAttackRangePreviewTimer > 0.f) mAttackRangePreviewTimer -= dt;

    // Tính toán Vận tốc và Giới hạn Bản đồ
    if (isDashing()) mVelocity = mDashDirection * (mSpeed * mGameData.getDashSpeedMult());
    else if (isShieldActive()) mVelocity = sf::Vector2f(0.f, 0.f);
    else handleInput(window, projectiles, enemies);

    mPosition += mVelocity * dt;

    if (mPosition.x < 20.f) mPosition.x = 20.f;
    if (mPosition.x > 3980.f) mPosition.x = 3980.f;
    if (mPosition.y < 20.f) mPosition.y = 20.f;
    if (mPosition.y > 3980.f) mPosition.y = 3980.f;

    mShape.setPosition(mPosition);
    mShieldShape.setPosition(mPosition);
    mWhirlwindShape.setPosition(mPosition);

    // Hệ thống Cập nhật Hoạt ảnh (Animation State Machine)
    const std::vector<Player::FrameInfo>* activeFrames = &mIdleFrames;
    float frameDuration = 0.12f;

    if (isPlayingSkillAnimation()) {
        if (mCurrentSkillAnim == "Skill_Q" && !mSkillQFrames.empty()) {
            activeFrames = &mSkillQFrames;
            frameDuration = mGameData.getAnimationClip(characterName(mType) + "_Skill_Q").frameDuration;
        } else if (mCurrentSkillAnim == "Skill_E" && !mSkillEFrames.empty()) {
            activeFrames = &mSkillEFrames;
            frameDuration = mGameData.getAnimationClip(characterName(mType) + "_Skill_E").frameDuration;
        } else if (mCurrentSkillAnim == "Skill_R" && !mSkillRFrames.empty()) {
            activeFrames = &mSkillRFrames;
            frameDuration = mGameData.getAnimationClip(characterName(mType) + "_Skill_R").frameDuration;
        } else if (mCurrentSkillAnim == "Attack" && !mAttackFrames.empty()) {
            activeFrames = &mAttackFrames;
            frameDuration = mGameData.getAnimationClip("Player_Attack").frameDuration;
        }
    }

    if (activeFrames == &mIdleFrames) {
        if (isAttacking() && !mAttackFrames.empty()) {
            activeFrames = &mAttackFrames;
            frameDuration = (mType == CharacterType::Archer && mArcherAtkSpeedBuffTimer > 0.f) ? 0.03f : 0.08f;
        } else if ((mVelocity.x != 0.f || mVelocity.y != 0.f) && !mWalkFrames.empty()) {
            if (mType == CharacterType::Lancer && mLancerSpeedBuffTimer > 0.f && !mSkillQFrames.empty()) {
                activeFrames = &mSkillQFrames;
                frameDuration = 0.08f;
            } else {
                activeFrames = &mWalkFrames;
                frameDuration = 0.10f;
            }
        }
    }

    if (!activeFrames->empty()) {
        mAnimTimer += dt;
        if (mAnimTimer >= frameDuration) {
            mAnimTimer = 0.f;
            mPrevAnimFrameIdx = mAnimFrameIdx;
            if (activeFrames == &mAttackFrames) {
                if (mAnimFrameIdx + 1 >= activeFrames->size()) {
                    mAttackTimer = 0.f;
                    mAnimFrameIdx = 0;
                } else mAnimFrameIdx++;
            } else mAnimFrameIdx = (mAnimFrameIdx + 1) % activeFrames->size();
        }
        if (mAnimFrameIdx >= activeFrames->size()) mAnimFrameIdx = 0;

        const FrameInfo& frame = (*activeFrames)[mAnimFrameIdx];
        if (frame.texture) {
            if (!mRenderSprite) mRenderSprite.emplace(*frame.texture);
            else mRenderSprite->setTexture(*frame.texture, false);
            mRenderSprite->setTextureRect(frame.rect);
            mRenderSprite->setOrigin({frame.rect.size.x / 2.f, frame.rect.size.y / 2.f});

            float scaleX = (mFacingDirection.x < 0.f) ? -1.4f : 1.4f;
            mRenderSprite->setScale({scaleX, 1.4f});
            mRenderSprite->setPosition(mPosition);

            if (mHitFlashTimer > 0.f) mRenderSprite->setColor(sf::Color(255, 120, 120));
            else mRenderSprite->setColor(sf::Color::White);
            mHasSprite = true;
        }
    } else if (mHasSprite && mAnimSprite) {
        if (isPlayingSkillAnimation()) mAnimSprite->playFlippable(mCurrentSkillAnim);
        else if (isAttacking()) mAnimSprite->playFlippable("Attack");
        else if (mVelocity.x != 0.f || mVelocity.y != 0.f) mAnimSprite->playFlippable("Walk");
        else mAnimSprite->playFlippable("Idle");

        mAnimSprite->setFlipX(mFacingDirection.x < 0.f);
        mAnimSprite->setPosition(mPosition);
        if (mHitFlashTimer > 0.f) mAnimSprite->setTint(sf::Color(255, 120, 120));
        else mAnimSprite->setTint(sf::Color::White);
        mAnimSprite->update(dt);
    }

    if (isMeleeClass() && isAttacking()) {
        mMeleeSlashShape.setPosition(mPosition);
        float angle = std::atan2(mFacingDirection.y, mFacingDirection.x) * 180.f / 3.14159f;
        mMeleeSlashShape.setRotation(sf::degrees(angle));
        mMeleeSlashShape.setScale({1.0f, 1.0f});
    }
}

void Player::draw(sf::RenderWindow& window) {
    if (mInvulnerableTimer > 0.f && mRenderSprite.has_value()) mRenderSprite->setColor(sf::Color(100, 255, 255, 200));
    if (mHasSprite && !mIdleFrames.empty() && mRenderSprite.has_value()) window.draw(*mRenderSprite);
    else if (mHasSprite && mAnimSprite) mAnimSprite->draw(window);
    else window.draw(mShape);
    if (mRenderSprite.has_value()) mRenderSprite->setColor(sf::Color::White);
}

// Xử lý Input từ Người dùng (handleInput)
void Player::handleInput(sf::RenderWindow& window, std::vector<Projectile>& projectiles, const std::vector<std::unique_ptr<Enemy>>& enemies) {
    mVelocity = sf::Vector2f(0.f, 0.f);

    // Di chuyển bằng WASD
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) mVelocity.y = -1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) mVelocity.y = 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) mVelocity.x = -1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) mVelocity.x = 1.f;

    float currentSpeed = mSpeed;
    if (mLancerSpeedBuffTimer > 0.f) currentSpeed *= 1.5f;
    if (mVelocity.x != 0.f || mVelocity.y != 0.f) {
        float length = std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);
        mVelocity = (mVelocity / length) * currentSpeed;
        if (!isDashing()) mFacingDirection = sf::Vector2f(mVelocity.x / currentSpeed, mVelocity.y / currentSpeed);
    }

    // Phím tắt dùng Bình máu(H) và Lướt(Space)
    static bool hPressedLastFrame = false;
    bool hPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H);
    if (hPressed && !hPressedLastFrame) usePotion();
    hPressedLastFrame = hPressed;

    static bool fPressedLastFrame = false;
    bool fPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    if (fPressed && !fPressedLastFrame && mDashCooldown <= 0.f && (mVelocity.x != 0.f || mVelocity.y != 0.f)) {
        AudioManager::instance().playSfx("dash");
        mDashTimer = 0.15f;
        mDashCooldown = 0.8f;
        mDashDirection = sf::Vector2f(mVelocity.x / currentSpeed, mVelocity.y / currentSpeed);
    }
    fPressedLastFrame = fPressed;

    // Đánh thường (Chuột trái)
    static bool leftMousePressedLastFrame = false;
    bool leftMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    if (leftMousePressed && !leftMousePressedLastFrame) mAttackRangePreviewTimer = 0.5f;
    leftMousePressedLastFrame = leftMousePressed;

    if (leftMousePressed && !isAttacking() && !isPlayingSkillAnimation() && mAttackCooldown <= 0.f) {
        AudioManager::instance().playSfx("attack");
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::Vector2f mouseDir = mouseWorldPos - mPosition;
        float mouseDist = std::hypot(mouseDir.x, mouseDir.y);
        if (mouseDist > 0.001f) mFacingDirection = mouseDir / mouseDist;

        if (isMeleeClass()) {
            float totalFrames = mAttackFrames.empty() ? 7.f : static_cast<float>(mAttackFrames.size());
            float animDuration = 0.08f * totalFrames;
            if (mType == CharacterType::Swordsman && mSwordsmanAtkSpeedBuffTimer > 0.f) animDuration *= 0.6f;
            mAttackTimer = animDuration;
            mAttackCooldown = animDuration;
            mAnimFrameIdx = 0;
            mAnimTimer = 0.f;
        } else {
            float perFrame = (mType == CharacterType::Archer && mArcherAtkSpeedBuffTimer > 0.f) ? 0.03f : 0.08f;
            float totalFrames = mAttackFrames.empty() ? 7.f : static_cast<float>(mAttackFrames.size());
            float fullAnimDuration = perFrame * totalFrames;
            float cooldown = fullAnimDuration;
            if (mType == CharacterType::Archer && mArcherAtkSpeedBuffTimer > 0.f) cooldown = 0.08f;

            mAttackCooldown = cooldown;
            mAttackTimer = fullAnimDuration;
            mPendingBasicProjectile = true;
            mAnimFrameIdx = 0;
            mAnimTimer = 0.f;
        }
    }

    // Sinh đạn Đánh thường (Frame-accurate Projectile Spawn)
    if (mPendingBasicProjectile && isAttacking()) {
        std::size_t releaseFrame = 6;
        if (mAnimFrameIdx == releaseFrame && mPrevAnimFrameIdx != releaseFrame) {
            mPendingBasicProjectile = false;
            sf::Vector2f projDir = mFacingDirection;
            if (mType == CharacterType::Archer) {
                float arrowSpeed = 650.f;
                float arrowLifetime = getAttackRange() / arrowSpeed;
                sf::Vector2f projVelocity = projDir * arrowSpeed;
                sf::Vector2f spawnPos = mPosition + projDir * 25.f;
                Projectile arrow(spawnPos, projVelocity, getDamage(), arrowLifetime, 6.f, sf::Color::White);
                std::string path = mGameData.getAssetPath("arrow_attack");
                static sf::Texture arrowTex;
                static bool arrowTexLoaded = false;
                if (!arrowTexLoaded && !path.empty() && std::filesystem::exists(path))
                    if (arrowTex.loadFromFile(path)) arrowTexLoaded = true;
                if (arrowTexLoaded) arrow.setTexture(&arrowTex);
                projectiles.push_back(arrow);
            }
        }
    }
    if (mPendingBasicProjectile && !isAttacking()) mPendingBasicProjectile = false;

    // Kích hoạt Kỹ năng Q, E, R
    static bool qPressedLastFrame = false;
    bool qPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q);
    if (qPressed && !qPressedLastFrame && mQCooldown <= 0.f) {
        std::string skillId = characterName(mType) + "_Q";
        if (auto* skill = SkillRegistry::getInstance().getSkill(skillId)) {
            AudioManager::instance().playSfx("skill_generic");
            skill->execute(*this, projectiles, window);
        }
    }
    qPressedLastFrame = qPressed;

    static bool ePressedLastFrame = false;
    bool ePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E);
    if (ePressed && !ePressedLastFrame && mECooldown <= 0.f) {
        std::string skillId = characterName(mType) + "_E";
        if (auto* skill = SkillRegistry::getInstance().getSkill(skillId)) {
            AudioManager::instance().playSfx("skill_generic");
            skill->execute(*this, projectiles, window);
        }
    }
    ePressedLastFrame = ePressed;

    static bool rPressedLastFrame = false;
    bool rPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R);
    if (rPressed && !rPressedLastFrame && mRCooldown <= 0.f) {
        std::string skillId = characterName(mType) + "_R";
        if (auto* skill = SkillRegistry::getInstance().getSkill(skillId)) {
            AudioManager::instance().playSfx("skill_generic");
            skill->execute(*this, projectiles, window);
        }
    }
    rPressedLastFrame = rPressed;

    // Sinh đạn đặc biệt cho Kỹ năng E và R của Archer (Mưa tên / Tên khổng lồ)
    if (mPendingSkillEProjectile && mType == CharacterType::Archer) {
        if (mAnimFrameIdx == 6 && mPrevAnimFrameIdx != 6) {
            mPendingSkillEProjectile = false;
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            sf::Vector2f fireDir = mouseWorldPos - mPosition;
            float dist = std::sqrt(fireDir.x * fireDir.x + fireDir.y * fireDir.y);
            if (dist > 0.f) fireDir /= dist;
            else fireDir = mFacingDirection;
            mFacingDirection = fireDir;

            float baseAngle = std::atan2(fireDir.y, fireDir.x);
            float angles[] = {-0.3f, -0.15f, 0.f, 0.15f, 0.3f};

            std::string path = mGameData.getAssetPath("arrow_attack");
            static sf::Texture arrowTex;
            static bool arrowTexLoaded = false;
            if (!arrowTexLoaded && !path.empty() && std::filesystem::exists(path))
                if (arrowTex.loadFromFile(path)) arrowTexLoaded = true;

            for (float offset : angles) {
                float angle = baseAngle + offset;
                sf::Vector2f dir = {std::cos(angle), std::sin(angle)};
                sf::Vector2f projVelocity = dir * 650.f;
                sf::Vector2f spawnPos = mPosition + dir * 25.f;
                Projectile arrow(spawnPos, projVelocity, getDamage() * 1.2f, 1.2f, 6.f, sf::Color::White);
                if (arrowTexLoaded) arrow.setTexture(&arrowTex);
                projectiles.push_back(arrow);
            }
        }
        if (!isAttacking() && !isPlayingSkillAnimation()) mPendingSkillEProjectile = false;

    }

    if (mPendingSkillRProjectile && mType == CharacterType::Archer) {
        std::size_t releaseFrame = 9;
        if (mAnimFrameIdx == releaseFrame && mPrevAnimFrameIdx != releaseFrame) {
            mPendingSkillRProjectile = false;
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            sf::Vector2f fireDir = mouseWorldPos - mPosition;
            float dist = std::hypot(fireDir.x, fireDir.y);
            if (dist > 0.001f) fireDir /= dist;
            else fireDir = mFacingDirection;
            mFacingDirection = fireDir;

            sf::Vector2f projVelocity = fireDir * 700.f;
            sf::Vector2f spawnPos = mPosition + fireDir * 25.f;
            Projectile bigArrow(spawnPos, projVelocity, getDamage() * 3.5f, 1.5f, 18.f, sf::Color::White);

            std::string path = mGameData.getAssetPath("arrow_skill");
            static sf::Texture skillArrowTex;
            static bool loaded = false;
            if (!loaded && !path.empty() && std::filesystem::exists(path))
                if (skillArrowTex.loadFromFile(path)) loaded = true;
            if (loaded) bigArrow.setTexture(&skillArrowTex);
            projectiles.push_back(bigArrow);
        }
        if (!isAttacking() && !isPlayingSkillAnimation()) mPendingSkillRProjectile = false;

    }
}

// Logic Va chạm, Chỉ số và Hệ thống RPG

// Kiểm tra Kẻ địch trong Tầm chém
bool Player::isEnemyInMeleeRange(const Enemy& enemy, float reach, float width) const {
    sf::Vector2f v = enemy.getPosition() - mPosition;
    float dist = std::hypot(v.x, v.y);
    float r = enemy.getIsBoss() ? enemy.getRadius() * 3.0f : enemy.getRadius();
    if (dist > reach + r) return false;

    sf::Vector2f dir = mFacingDirection;
    if (std::hypot(dir.x, dir.y) < 0.001f) dir = {1.f, 0.f};

    float dPar = v.x * dir.x + v.y * dir.y;
    float dPerp = v.x * (-dir.y) + v.y * dir.x;

    if (dPar < -15.f) return false;
    if (std::abs(dPerp) > (width / 2.f) + r) return false;

    return true;
}

float Player::getAttackRange() const {
    if (isMeleeClass()) return 50.f;
    if (mType == CharacterType::Archer) return 300.f;
    return 200.f;
}

// Nhận sát thương
void Player::takeDamage(int damage) {
    if (isDashing() || isShieldActive() || mInvulnerableTimer > 0.f) return;
    float finalDamage = static_cast<float>(damage) * 100.0f / (100.0f + getDefense());
    int actualDmg = static_cast<int>(std::max(1.0f, finalDamage));
    int oldHP = mHP;
    mHP = std::max(0, mHP - actualDmg);
    static sf::Clock hurtAudioClock;
    if (mHP < oldHP && hurtAudioClock.getElapsedTime().asSeconds() > 0.25f) {
        AudioManager::instance().playSfx("player_hurt");
        hurtAudioClock.restart();
    }
    mHitFlashTimer = 0.12f;
}

// Nhận Kinh nghiệm và Lên cấp
void Player::gainEXP(int amount) {
    if (mLevel >= 29) return;
    mEXP += amount;
    if (mEXP < 0) mEXP = 0;
    const CharacterData& data = mGameData.getCharacter(mType);
    while (true) {
        int expNeeded = mLevel * 100;
        if (mEXP >= expNeeded) {
            mEXP -= expNeeded;
            mLevel++;
            AudioManager::instance().playSfx("levelup");
            recalculateStats();
            if (mLevel >= 29) {
                mEXP = 0;
                break;
            }
        } else break;
    }
}

// Nhận vàng
void Player::gainGold(int amount) {
    mGold += amount;
    if (mGold < 0) mGold = 0;
}

// Tính Sát thương đầu ra
float Player::getDamage() const {
    const CharacterData& data = mGameData.getCharacter(mType);
    float baseDamage = isMeleeClass() ? data.baseMeleeDamage : data.baseRangedDamage;
    float mult = 1.0f;
    float levelDamage = baseDamage + (mLevel - 1) * data.damagePerLevel;
    float finalDamage = levelDamage * (1.0f + mDamageScrollCount * 0.05f);
    return finalDamage * mult;
}

float Player::getDefense() const {
    const CharacterData& data = mGameData.getCharacter(mType);
    float levelDefense = 5.0f + (mLevel - 1) * data.defensePerLevel;
    return levelDefense * (1.0f + mDefenseScrollCount * 0.12f);
}

void Player::recalculateStats() {
    const CharacterData& data = mGameData.getCharacter(mType);
    float levelHP = data.baseHp + (mLevel - 1) * data.hpPerLevel;
    mMaxHP = static_cast<int>(levelHP * (1.0f + mHPScrollCount * 0.08f));
}

// Mua và Sử dụng Bình Máu
void Player::buyPotion() {
    int currentPrice = mGameData.getCurrentShopPrice("potion", mLevel);
    if (mGold >= currentPrice) {
        mGold -= currentPrice;
        mPotions++;
    }
}

void Player::usePotion() {
    if (mPotions > 0 && mHP < mMaxHP) {
        mPotions--;
        AudioManager::instance().playSfx("potion_use");
        mHP = std::min(mMaxHP, mHP + mGameData.getPotionHealAmount());
        mHitFlashTimer = 0.2f;
    }
}

// Khởi tạo Animation Clips
void Player::loadAnimations(const sf::Texture* idleTex, const sf::Texture* walkTex, const sf::Texture* attackTex, const sf::Texture* qTex, const sf::Texture* eTex, const sf::Texture* rTex) {
    if (!idleTex) return;
    mAnimSprite = std::make_unique<AnimatedSprite>(*idleTex);

    auto sizeIdle = idleTex->getSize();
    unsigned int frameW_idle = sizeIdle.x / 4;
    if (frameW_idle == 0) frameW_idle = sizeIdle.x;
    AnimClip idleClip;
    for (unsigned int i = 0; i < 4; ++i) idleClip.frames.push_back(sf::IntRect({static_cast<int>(i * frameW_idle), 0}, {static_cast<int>(frameW_idle), static_cast<int>(sizeIdle.y)}));
    const auto& idleData = mGameData.getAnimationClip("Player_Idle");
    idleClip.frameDuration = idleData.frameDuration;
    idleClip.loop = idleData.loop;
    mAnimSprite->addClipFlippable("Idle", idleClip);

    if (walkTex) {
        auto sizeWalk = walkTex->getSize();
        unsigned int frameW_walk = sizeWalk.x / 6;
        if (frameW_walk == 0) frameW_walk = sizeWalk.x / 4;
        if (frameW_walk == 0) frameW_walk = sizeWalk.x;
        unsigned int frameCount = sizeWalk.x / frameW_walk;
        AnimClip walkClip;
        for (unsigned int i = 0; i < frameCount; ++i) walkClip.frames.push_back(sf::IntRect({static_cast<int>(i * frameW_walk), 0}, {static_cast<int>(frameW_walk), static_cast<int>(sizeWalk.y)}));
        const auto& walkData = mGameData.getAnimationClip("Player_Walk");
        walkClip.frameDuration = walkData.frameDuration;
        walkClip.loop = walkData.loop;
        mAnimSprite->addClipFlippable("Walk", walkClip);
    }

    if (attackTex) {
        auto sizeAtk = attackTex->getSize();
        unsigned int frameW_atk = sizeAtk.x / 6;
        if (frameW_atk == 0) frameW_atk = sizeAtk.x / 4;
        if (frameW_atk == 0) frameW_atk = sizeAtk.x;
        unsigned int frameCount = sizeAtk.x / frameW_atk;
        AnimClip atkClip;
        for (unsigned int i = 0; i < frameCount; ++i) atkClip.frames.push_back(sf::IntRect({static_cast<int>(i * frameW_atk), 0}, {static_cast<int>(frameW_atk), static_cast<int>(sizeAtk.y)}));
        const auto& atkData = mGameData.getAnimationClip("Player_Attack");
        atkClip.frameDuration = atkData.frameDuration;
        atkClip.loop = atkData.loop;
        mAnimSprite->addClipFlippable("Attack", atkClip);
    }

    if (qTex) {
        auto sz = qTex->getSize();
        unsigned int count = sz.x / 100;
        if (count == 0) count = 1;
        AnimClip clip;
        for (unsigned int i = 0; i < count; ++i) clip.frames.push_back(sf::IntRect({static_cast<int>(i * 100), 0}, {100, static_cast<int>(sz.y)}));
        const auto& cData = mGameData.getAnimationClip(characterName(mType) + "_Skill_Q");
        clip.frameDuration = cData.frameDuration;
        clip.loop = cData.loop;
        mAnimSprite->addClipFlippable("Skill_Q", clip);
    }

    if (eTex) {
        auto sz = eTex->getSize();
        unsigned int count = sz.x / 100;
        if (count == 0) count = 1;
        AnimClip clip;
        for (unsigned int i = 0; i < count; ++i) clip.frames.push_back(sf::IntRect({static_cast<int>(i * 100), 0}, {100, static_cast<int>(sz.y)}));
        const auto& cData = mGameData.getAnimationClip(characterName(mType) + "_Skill_E");
        clip.frameDuration = cData.frameDuration;
        clip.loop = cData.loop;
        mAnimSprite->addClipFlippable("Skill_E", clip);
    }

    if (rTex) {
        auto sz = rTex->getSize();
        unsigned int count = sz.x / 100;
        if (count == 0) count = 1;
        AnimClip clip;
        for (unsigned int i = 0; i < count; ++i) clip.frames.push_back(sf::IntRect({static_cast<int>(i * 100), 0}, {100, static_cast<int>(sz.y)}));
        const auto& cData = mGameData.getAnimationClip(characterName(mType) + "_Skill_R");
        clip.frameDuration = cData.frameDuration;
        clip.loop = cData.loop;
        mAnimSprite->addClipFlippable("Skill_R", clip);
    }

    mAnimSprite->setScale({1.2f, 1.2f});
    mHasSprite = true;
}
