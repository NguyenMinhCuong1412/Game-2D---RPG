#include "entities/Enemy.h"
#include "core/AudioManager.h"

Enemy::Enemy(std::string name, Region region, int level, sf::Vector2f spawnPoint, bool isBoss, const GameData& gameData)
    : mGameData(gameData), mName(name), mRegion(region), mLevel(level), mSpawnPoint(spawnPoint), mIsBoss(isBoss)
{
    mPosition = mSpawnPoint;

    const EnemyData& data = mGameData.getEnemy(mName);
    int effectiveLevel = mLevel;
    if (mIsBoss) {
        int baseBossLevel = 5;
        if (mName == "Demon") baseBossLevel = 13;
        else if (mName == "Bringer Of Death") baseBossLevel = 29;
        effectiveLevel = baseBossLevel;
    }
    
    int levelScaling = effectiveLevel - 1;
    mMaxHP = data.baseHp + levelScaling * data.hpPerLevel;
    mDamage = data.baseDamage + levelScaling * data.damagePerLevel;
    mDefense = data.baseDefense + levelScaling * data.defensePerLevel;
    mSpeed = data.baseSpeed + levelScaling * data.speedPerLevel;
    mEXPValue = data.baseExp + levelScaling * data.expPerLevel;
    mGoldValue = data.baseGold + levelScaling * data.goldPerLevel;
    mRadius = data.radius;
    mDetectionRange = data.detectionRange;
    mAttackRange = data.attackRange;
    mLeashRange = data.leashRange;

    mHP = mMaxHP;

    mShape.setRadius(mRadius);
    mShape.setOrigin({mRadius, mRadius});
    mShape.setPosition(mPosition);

    if (mIsBoss) {
        mShape.setFillColor(sf::Color(180, 0, 0));
        mShape.setOutlineThickness(4.f);
        mShape.setOutlineColor(sf::Color::Yellow);
    } else {
        sf::Color enemyColor = sf::Color::Red;
        switch (mRegion) {
            case Region::NorthWest: enemyColor = sf::Color(150, 60, 60); break;
            case Region::NorthEast: enemyColor = sf::Color(160, 70, 70); break;
            case Region::SouthWest: enemyColor = sf::Color(170, 80, 80); break;
            case Region::SouthEast: enemyColor = sf::Color(180, 90, 90); break;
            default: break;
        }
        mShape.setFillColor(enemyColor);
    }

    float barWidth = mIsBoss ? 80.f : 40.f;
    float barHeight = mIsBoss ? 8.f : 5.f;

    mHPBarBg.setSize({barWidth, barHeight});
    mHPBarBg.setFillColor(sf::Color(50, 50, 50, 200));
    mHPBarBg.setOrigin({barWidth / 2.f, barHeight / 2.f});

    mHPBarFg.setSize({barWidth, barHeight});
    mHPBarFg.setFillColor(mIsBoss ? sf::Color::Red : sf::Color::Green);
    mHPBarFg.setOrigin({barWidth / 2.f, barHeight / 2.f});
}

void Enemy::update(float dt) {
    if (mAttackCooldown > 0.f) mAttackCooldown -= dt;
    if (mHitFlashTimer > 0.f) mHitFlashTimer -= dt;

    mShape.setPosition(mPosition);

    float barYOffset = mIsBoss ? -60.f : -35.f;
    mHPBarBg.setPosition(mPosition + sf::Vector2f(0.f, barYOffset));
    mHPBarFg.setPosition(mPosition + sf::Vector2f(0.f, barYOffset));

    float hpRatio = std::max(0.f, static_cast<float>(mHP) / static_cast<float>(mMaxHP));
    float barWidth = mIsBoss ? 80.f : 40.f;
    mHPBarFg.setSize({barWidth * hpRatio, mHPBarFg.getSize().y});

    const std::vector<Enemy::FrameInfo>* activeFrames = &mIdleFrames;
    float frameDuration = 0.15f;

    if (mHitFlashTimer > 0.f && !mHurtFrames.empty() && mAIState != AIState::Attack) {
        activeFrames = &mHurtFrames;
        frameDuration = 0.08f;
    } else if (mAIState == AIState::Attack && !mAttackFrames.empty()) {
        if (mAttackAnimFinished && !mIdleFrames.empty()) {
            activeFrames = &mIdleFrames;
            frameDuration = 0.15f;
        } else {
            activeFrames = &mAttackFrames;
            frameDuration = 0.10f;
        }
    } else if (mAIState == AIState::Skill1 && !mSkill1Frames.empty()) {
        activeFrames = &mSkill1Frames;
        frameDuration = 0.10f;
    } else if (mAIState == AIState::Skill2 && !mSkill2Frames.empty()) {
        activeFrames = &mSkill2Frames;
        frameDuration = 0.10f;
    } else if (mAIState == AIState::Chase && !mWalkFrames.empty()) {
        activeFrames = &mWalkFrames;
        frameDuration = 0.12f;
    }

    if (!activeFrames->empty()) {
        mAnimTimer += dt;
        if (mAnimTimer >= frameDuration) {
            mAnimTimer = 0.f;
            mPrevAnimFrameIdx = mAnimFrameIdx;
            if (activeFrames == &mAttackFrames || activeFrames == &mSkill1Frames || activeFrames == &mSkill2Frames) {
                if (mAnimFrameIdx + 1 >= activeFrames->size()) {
                    if (!mAttackAnimFinished) {
                        mAttackAnimFinished = true;
                        mAttackCooldown = mGameData.getEnemy(mName).attackCooldown;
                    }
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

            float scaleVal = mIsBoss ? 2.5f : 1.3f;
            float scaleX = mFlipX ? -scaleVal : scaleVal;
            mRenderSprite->setScale({scaleX, scaleVal});
            mRenderSprite->setPosition(mPosition);

            if (mHitFlashTimer > 0.f) mRenderSprite->setColor(sf::Color(255, 100, 100));
            else mRenderSprite->setColor(sf::Color::White);
            mHasSprite = true;
        }
    } else if (mHasSprite && mAnimSprite) {
        if (mAIState == AIState::Attack) mAnimSprite->playFlippable("Attack");
        else if (mAIState == AIState::Skill1) mAnimSprite->playFlippable("Skill1");
        else if (mAIState == AIState::Skill2) mAnimSprite->playFlippable("Skill2");
        else if (mAIState == AIState::Chase) mAnimSprite->playFlippable("Walk");
        else mAnimSprite->playFlippable("Idle");

        mAnimSprite->setFlipX(mFlipX);
        mAnimSprite->setPosition(mPosition);

        if (mHitFlashTimer > 0.f) mAnimSprite->setTint(sf::Color(255, 100, 100));
        else mAnimSprite->setTint(sf::Color::White);

        mAnimSprite->update(dt);
    }
}

void Enemy::updateAI(float dt, sf::Vector2f playerPos, int& playerHP, std::vector<Projectile>& projectiles) {
    if (mHP <= 0) return;

    sf::Vector2f dirToPlayer = playerPos - mPosition;
    float distToPlayer = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.y * dirToPlayer.y);

    sf::Vector2f dirToSpawn = mSpawnPoint - mPosition;
    float distToSpawn = std::sqrt(dirToSpawn.x * dirToSpawn.x + dirToSpawn.y * dirToSpawn.y);

    if (distToPlayer > 0.001f && (mAIState == AIState::Chase || mAIState == AIState::Attack || mAIState == AIState::Skill1 || mAIState == AIState::Skill2)) mFlipX = (dirToPlayer.x < 0.f);

        switch (mAIState) {
        case AIState::Idle:
            if (distToPlayer <= mDetectionRange) mAIState = AIState::Chase;
            break;
        case AIState::Chase:
            if (distToSpawn > mLeashRange && mName != "Bringer Of Death") mAIState = AIState::Return;
            else if (distToPlayer <= mAttackRange) {
                int options = 1;
                if (!mSkill1Frames.empty()) options++;
                if (!mSkill2Frames.empty()) options++;
                int choice = rand() % options;
                if (choice == 0) mAIState = AIState::Attack;
                else if (choice == 1) mAIState = AIState::Skill1;
                else mAIState = AIState::Skill2;

                mAttackHasHit = false;
                mAttackAnimFinished = false;
                mAttackCooldown = 0.f;
                mAnimTimer = 0.f;
                mAnimFrameIdx = 0;
                mAttackPhase = 0;
            } else {
                sf::Vector2f moveDir = dirToPlayer / distToPlayer;
                mPosition += moveDir * mSpeed * dt;
                mFlipX = (moveDir.x < 0.f);
            }
            break;

        case AIState::Attack:
        case AIState::Skill1:
        case AIState::Skill2:
            if (mAttackAnimFinished && mAttackCooldown <= 0.f) {
                mAIState = AIState::Chase;
                mAnimFrameIdx = 0;
                mAnimTimer = 0.f;
                break;
            }

            {
                bool isSkill1 = (mAIState == AIState::Skill1);
                bool isSkill2 = (mAIState == AIState::Skill2);
                bool isAttack = (mAIState == AIState::Attack);

                if (mName == "Armored Skeleton") {
                    if (isAttack && mAnimFrameIdx == 4 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                    }
                    if (isSkill1 && mAnimFrameIdx >= 1 && mAnimFrameIdx <= 8 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.5f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage * 1.2f));
                    }
                }
                else if (mName == "Bat") {
                    if (isAttack && mAnimFrameIdx == 3 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                    }
                }
                else if (mName == "Elite Orc") {
                    if (isAttack && mAnimFrameIdx == 3 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                    }
                    if (isSkill1 && mAnimFrameIdx >= 1 && mAnimFrameIdx <= 9 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.5f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage * 1.2f));
                    }
                    if (isSkill2 && mAnimFrameIdx == 4 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage * 1.5f));
                    }
                }
                else if (mName == "Greatsword Skeleton") {
                    if (isAttack && mAnimFrameIdx == 5 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                    }
                    if (isSkill1 && mAnimFrameIdx == 5 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage * 1.2f));
                    }
                    if (isSkill2 && mAnimFrameIdx == 7 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage * 1.5f));
                    }
                }
                else if (mName == "Necromancer") {
                    if (isAttack && mAnimFrameIdx == 5 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                    }
                    if (isSkill1 && mAnimFrameIdx == 4 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        Projectile spell(playerPos, {0.f, 0.f}, mDamage * 1.2f, 1.0f, 20.f, sf::Color::Magenta);
                        spell.setIsEnemy(true);
                        spell.setEffectOnly(true);

                        std::string path = mGameData.getAssetPath("necromancer_spell");
                        static sf::Texture tex1; static bool loaded1 = false;
                        if (!loaded1 && std::filesystem::exists(path)) { tex1.loadFromFile(path); loaded1 = true; }
                        if (loaded1) {
                            int w = tex1.getSize().y; int c = tex1.getSize().x / w;
                            for (int i=0; i<c; ++i) spell.addAnimFrame(&tex1, sf::IntRect({i*w, 0}, {w, w}));
                        }
                        projectiles.push_back(spell);
                    }
                    if (isSkill2 && mAnimFrameIdx == 3 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        Projectile summon(mPosition, {0.f, 0.f}, 0.f, 1.0f, 0.f, sf::Color::Transparent);
                        summon.setIsEnemy(true);
                        summon.setEffectOnly(true);

                        std::string path = mGameData.getAssetPath("necromancer_spell2");
                        static sf::Texture tex2; static bool loaded2 = false;
                        if (!loaded2 && std::filesystem::exists(path)) { tex2.loadFromFile(path); loaded2 = true; }
                        if (loaded2) {
                            int w = tex2.getSize().y; int c = tex2.getSize().x / w;
                            for (int i=0; i<c; ++i) summon.addAnimFrame(&tex2, sf::IntRect({i*w, 0}, {w, w}));
                        }
                        projectiles.push_back(summon);
                    }
                }
                else if (mName == "Orc Rider" || mName == "Orc rider") {
                    if (isAttack && mAnimFrameIdx == 5 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                    }
                    if (isSkill1 && mAnimFrameIdx == 5 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage * 1.2f));
                    }
                    if (isSkill2 && mAnimFrameIdx >= 5 && mAnimFrameIdx <= 9 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage * 1.5f));
                    }
                }
                else if (mName == "Orc") {
                    if ((isAttack || isSkill1) && mAnimFrameIdx == 3 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                    }
                }
                else if (mName == "Skeleton Archer") {
                    if (isAttack && mAnimFrameIdx == 6 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        sf::Vector2f moveDir = dirToPlayer / distToPlayer;
                        float skelArrowSpeed = 400.f;
                        float skelArrowLifetime = mAttackRange / skelArrowSpeed;
                        Projectile arrow(mPosition, moveDir * skelArrowSpeed, mDamage, skelArrowLifetime, 6.f, sf::Color::White);
                        arrow.setIsEnemy(true);

                        std::string arrowPath = mGameData.getAssetPath("skeleton_arrow");
                        static sf::Texture archerArrowTexture;
                        static bool textureLoaded = false;
                        if (!textureLoaded && !arrowPath.empty() && std::filesystem::exists(arrowPath))
                            if (archerArrowTexture.loadFromFile(arrowPath)) textureLoaded = true;
                        if (textureLoaded) arrow.setTexture(&archerArrowTexture);
                        projectiles.push_back(arrow);
                    }
                }
                else if (mName == "Skeleton" || mName == "Slime") {
                    if (isAttack && mAnimFrameIdx == 3 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                    }
                    if (mName == "Slime" && isSkill1 && mAnimFrameIdx == 8 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage * 1.5f));
                    }
                }
                else if (mName == "Werebear") {
                    if (isAttack && mAnimFrameIdx == 5 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                    }
                    if (isSkill1) {
                        if (mAnimFrameIdx == 4 && mAttackPhase == 0) {
                            mAttackPhase = 1;
                            if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                        }
                        if (mAnimFrameIdx == 9 && mAttackPhase == 1) {
                            mAttackPhase = 2;
                            if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                        }
                    }
                    if (isSkill2 && mAnimFrameIdx == 5 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage * 1.5f));
                    }
                }
                else if (mName == "Werewolf") {
                    if (isAttack && mAnimFrameIdx == 5 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                    }
                    if (isSkill1 && mAnimFrameIdx == 7 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage * 1.5f));
                    }
                }
                else if (mName == "Goblin Giant") {
                    if (isAttack) {
                        if (mAnimFrameIdx == 12 && mAttackPhase == 0) {
                            mAttackPhase = 1;
                            if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                        }
                        if (mAnimFrameIdx == 13 && mAttackPhase == 1) {
                            mAttackPhase = 2;
                            if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                        }
                    }
                }
                else if (mName == "Demon") {
                    if (isAttack && mAnimFrameIdx == 8 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                    }
                }
                else if (mName == "Bringer Of Death") {
                    if (isAttack && mAnimFrameIdx == 3 && !mAttackHasHit) {
                        mAttackHasHit = true;
                        if (distToPlayer <= mAttackRange * 1.3f) playerHP = std::max(0, playerHP - static_cast<int>(mDamage));
                    }
                    if (isSkill1) {
                        if (mAttackAnimFinished && !mAttackHasHit) {
                            mAttackHasHit = true;

                            Projectile spell(playerPos, {0.f, 0.f}, mDamage, 2.0f, 40.f, sf::Color::Magenta);
                            spell.setIsEnemy(true);
                            spell.setEffectOnly(true);
                            static std::vector<sf::Texture> spellTextures(16);
                            static bool spellLoaded = false;
                            if (!spellLoaded) {
                                bool allLoaded = true;
                                for (int i = 0; i < 16; ++i) {
                                    std::string p = "assets/textures/enemy/Boss/Bringer Of Death/Spell/bringerOfDeath_spell_" + std::to_string(i) + ".png";
                                    if (!std::filesystem::exists(p)) p = "assets/textures/projectiles/bringerOfDeath_spell/bringerOfDeath_spell_" + std::to_string(i) + ".png";
                                    if (std::filesystem::exists(p)) spellTextures[i].loadFromFile(p);
                                    else allLoaded = false;
                                }
                                spellLoaded = allLoaded;
                            }

                            if (spellLoaded) for (int i = 0; i < 16; ++i) spell.addAnimFrame(&spellTextures[i]);
                            projectiles.push_back(spell);
                        }
                    }
                }
            }
            break;

        case AIState::Return:
            if (distToSpawn <= 10.f) {
                mPosition = mSpawnPoint;
                if (!mIsBoss) mHP = mMaxHP;
                mAIState = AIState::Idle;
            } else {
                sf::Vector2f moveDir = dirToSpawn / distToSpawn;
                mPosition += moveDir * mSpeed * 1.5f * dt;
                mFlipX = (moveDir.x < 0.f);
            }
            break;
    }

    mPosition.x = std::max(mRadius, std::min(4000.f - mRadius, mPosition.x));
    mPosition.y = std::max(mRadius, std::min(4000.f - mRadius, mPosition.y));
}

void Enemy::takeDamage(float damage) {
    AudioManager::instance().playSfx("hit");
    float finalDamage = damage * 100.0f / (100.0f + mDefense);
    int actualDmg = static_cast<int>(std::max(1.0f, finalDamage));
    mHP = std::max(0, mHP - actualDmg);
    mHitFlashTimer = 0.12f;

    if (mAIState == AIState::Idle || mAIState == AIState::Return) mAIState = AIState::Chase;
}

void Enemy::draw(sf::RenderWindow& window) {
    if (mHasSprite && !mIdleFrames.empty() && mRenderSprite.has_value()) {

        if (mIsHovered && mHitFlashTimer <= 0.f) mRenderSprite->setColor(sf::Color(255, 80, 80));
        window.draw(*mRenderSprite);
        if (mIsHovered && mHitFlashTimer <= 0.f) mRenderSprite->setColor(sf::Color::White);
    } else if (mHasSprite && mAnimSprite) {
        if (mIsHovered && mHitFlashTimer <= 0.f) mAnimSprite->setTint(sf::Color(255, 80, 80));
        mAnimSprite->draw(window);
        if (mIsHovered && mHitFlashTimer <= 0.f) mAnimSprite->setTint(sf::Color::White);
    } else {
        if (mIsHovered) {
            mShape.setOutlineThickness(3.f);
            mShape.setOutlineColor(sf::Color::Red);
        } else if (!mIsBoss) mShape.setOutlineThickness(0.f);
        window.draw(mShape);
    }

    if (mHP < mMaxHP && mHP > 0) {
        window.draw(mHPBarBg);
        window.draw(mHPBarFg);
    }
}

void Enemy::loadAnimations(const sf::Texture* idleTex, const sf::Texture* walkTex, const sf::Texture* attackTex) {
    if (!idleTex) return;

    mAnimSprite = std::make_unique<AnimatedSprite>(*idleTex);

    auto sizeIdle = idleTex->getSize();
    unsigned int frameW_idle = sizeIdle.x / 4;
    if (frameW_idle == 0) frameW_idle = sizeIdle.x;
    AnimClip idleClip;
    for (unsigned int i = 0; i < 4; ++i)
        idleClip.frames.push_back(sf::IntRect({static_cast<int>(i * frameW_idle), 0}, {static_cast<int>(frameW_idle), static_cast<int>(sizeIdle.y)}));
    const auto& idleData = mGameData.getAnimationClip("Enemy_Idle");
    idleClip.frameDuration = idleData.frameDuration;
    idleClip.loop = idleData.loop;
    mAnimSprite->addClipFlippable("Idle", idleClip);

    if (walkTex) {
        auto sizeWalk = walkTex->getSize();
        unsigned int frameW_walk = sizeWalk.x / 4;
        if (frameW_walk == 0) frameW_walk = sizeWalk.x;
        AnimClip walkClip;
        for (unsigned int i = 0; i < 4; ++i)
            walkClip.frames.push_back(sf::IntRect({static_cast<int>(i * frameW_walk), 0}, {static_cast<int>(frameW_walk), static_cast<int>(sizeWalk.y)}));
        const auto& walkData = mGameData.getAnimationClip("Enemy_Walk");
        walkClip.frameDuration = walkData.frameDuration;
        walkClip.loop = walkData.loop;
        mAnimSprite->addClipFlippable("Walk", walkClip);
    }

    if (attackTex) {
        auto sizeAtk = attackTex->getSize();
        unsigned int frameW_atk = sizeAtk.x / 4;
        if (frameW_atk == 0) frameW_atk = sizeAtk.x;
        AnimClip atkClip;
        for (unsigned int i = 0; i < 4; ++i)
            atkClip.frames.push_back(sf::IntRect({static_cast<int>(i * frameW_atk), 0}, {static_cast<int>(frameW_atk), static_cast<int>(sizeAtk.y)}));
        const auto& atkData = mGameData.getAnimationClip("Enemy_Attack");
        atkClip.frameDuration = atkData.frameDuration;
        atkClip.loop = atkData.loop;
        mAnimSprite->addClipFlippable("Attack", atkClip);
    }

    float scaleVal = mIsBoss ? 2.5f : 1.3f;
    mAnimSprite->setScale({scaleVal, scaleVal});
    mHasSprite = true;
}