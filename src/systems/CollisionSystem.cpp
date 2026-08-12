#include "systems/CollisionSystem.h"
#include "states/PlayingState.h"
#include "core/AudioManager.h"

void CollisionSystem::update(
    float dt,
    PlayingState& state
) {
    if (state.mPlayer->isDead()) return;

    // 1. Basic Melee Attack (Knight, Lancer, Swordsman - Frame 4 / index 3)
    if (state.mPlayer->isAttacking() && !state.mPlayer->isPlayingSkillAnimation() && state.mPlayer->isMeleeClass()) {
        if (!state.mMeleeHitRegistered && state.mPlayer->getAnimFrameIndex() == 3) {
            float dmg = state.mPlayer->getDamage();
            for (auto& enemy : state.mEnemies) {
                if (enemy->isDead()) continue;
                if (state.mPlayer->isEnemyInMeleeRange(*enemy, 65.f, 60.f)) {
                    bool isCrit = state.randomChance(state.mGameData.getCritChance());
                    if (isCrit) dmg *= state.mGameData.getCritMultiplier();

                    enemy->takeDamage(dmg);
                    state.addFloatingText(isCrit ? "CRIT " + std::to_string((int)dmg) : std::to_string((int)dmg),
                                    enemy->getPosition() - sf::Vector2f(0.f, enemy->getRadius()),
                                    isCrit ? sf::Color::Yellow : sf::Color::White);
                }
            }
            state.mMeleeHitRegistered = true;
        }
    } else if (!state.mPlayer->isAttacking() && !state.mPlayer->isPlayingSkillAnimation()) {
        state.mMeleeHitRegistered = false;
    }

    // 2. Knight Skill E (Whirlwind) — 2 hits at frame 4 (idx 3) and frame 8 (idx 7)
    if (state.mPlayer->getType() == CharacterType::Knight && state.mPlayer->isWhirlwindActive()) {
        std::size_t frameIdx = state.mPlayer->getAnimFrameIndex();
        int phase = state.mPlayer->getWhirlwindDamagePhase();

        bool shouldDamage = false;
        if (phase == 0 && frameIdx >= 3) {
            shouldDamage = true;
            state.mPlayer->setWhirlwindDamagePhase(1);
        } else if (phase == 1 && frameIdx >= 7) {
            shouldDamage = true;
            state.mPlayer->setWhirlwindDamagePhase(2);
        }

        if (shouldDamage) {
            float dmg = state.mPlayer->getDamage() * state.mGameData.getWhirlwindDamageMult();
            for (auto& enemy : state.mEnemies) {
                if (enemy->isDead()) continue;
                if (state.mPlayer->isEnemyInMeleeRange(*enemy, 80.f, 160.f)) {
                    bool isCrit = state.randomChance(state.mGameData.getCritChance());
                    if (isCrit) dmg *= state.mGameData.getCritMultiplier();
                    enemy->takeDamage(dmg);
                    state.addFloatingText(isCrit ? "CRIT " + std::to_string((int)dmg) : std::to_string((int)dmg),
                                    enemy->getPosition() - sf::Vector2f(0.f, enemy->getRadius()),
                                    isCrit ? sf::Color::Yellow : sf::Color(0, 191, 255));
                }
            }
        }
    } else {
        if (state.mPlayer->getWhirlwindDamagePhase() != 0) {
            state.mPlayer->setWhirlwindDamagePhase(0);
        }
    }

    // 3. Knight Skill R (Divine Charge) — 3 hits at frame 8 (idx 7), frame 9 (idx 8), frame 10 (idx 9)
    if (state.mPlayer->getType() == CharacterType::Knight &&
        state.mPlayer->isPlayingSkillAnimation() &&
        state.mPlayer->getCurrentSkillAnim() == "Skill_R") {
        std::size_t frameIdx = state.mPlayer->getAnimFrameIndex();
        int phase = state.mPlayer->getKnightRDamagePhase();

        bool shouldDamage = false;
        if (phase == 0 && frameIdx >= 7) {
            shouldDamage = true;
            state.mPlayer->setKnightRDamagePhase(1);
        } else if (phase == 1 && frameIdx >= 8) {
            shouldDamage = true;
            state.mPlayer->setKnightRDamagePhase(2);
        } else if (phase == 2 && frameIdx >= 9) {
            shouldDamage = true;
            state.mPlayer->setKnightRDamagePhase(3);
        }

        if (shouldDamage) {
            float dmg = state.mPlayer->getDamage() * 1.5f;
            for (auto& enemy : state.mEnemies) {
                if (enemy->isDead()) continue;
                if (state.mPlayer->isEnemyInMeleeRange(*enemy, 75.f, 70.f)) {
                    bool isCrit = state.randomChance(state.mGameData.getCritChance());
                    if (isCrit) dmg *= state.mGameData.getCritMultiplier();
                    enemy->takeDamage(dmg);
                    state.addFloatingText(isCrit ? "CRIT " + std::to_string((int)dmg) : std::to_string((int)dmg),
                                    enemy->getPosition() - sf::Vector2f(0.f, enemy->getRadius()),
                                    isCrit ? sf::Color::Yellow : sf::Color(255, 140, 0));
                }
            }
        }
    } else {
        if (state.mPlayer->getKnightRDamagePhase() != 0) {
            state.mPlayer->setKnightRDamagePhase(0);
        }
    }

    // 4. Lancer Skill E (Moving Attack) — 3 hits at frame 3 (idx 2), frame 4 (idx 3), frame 5 (idx 4)
    if (state.mPlayer->getType() == CharacterType::Lancer &&
        state.mPlayer->isPlayingSkillAnimation() &&
        state.mPlayer->getCurrentSkillAnim() == "Skill_E") {
        std::size_t frameIdx = state.mPlayer->getAnimFrameIndex();
        int phase = state.mPlayer->getLancerEDamagePhase();

        bool shouldDamage = false;
        if (phase == 0 && frameIdx >= 2) {
            shouldDamage = true;
            state.mPlayer->setLancerEDamagePhase(1);
        } else if (phase == 1 && frameIdx >= 3) {
            shouldDamage = true;
            state.mPlayer->setLancerEDamagePhase(2);
        } else if (phase == 2 && frameIdx >= 4) {
            shouldDamage = true;
            state.mPlayer->setLancerEDamagePhase(3);
        }

        if (shouldDamage) {
            float dmg = state.mPlayer->getDamage() * 1.2f;
            for (auto& enemy : state.mEnemies) {
                if (enemy->isDead()) continue;
                if (state.mPlayer->isEnemyInMeleeRange(*enemy, 75.f, 65.f)) {
                    bool isCrit = state.randomChance(state.mGameData.getCritChance());
                    if (isCrit) dmg *= state.mGameData.getCritMultiplier();
                    enemy->takeDamage(dmg);
                    state.addFloatingText(isCrit ? "CRIT " + std::to_string((int)dmg) : std::to_string((int)dmg),
                                    enemy->getPosition() - sf::Vector2f(0.f, enemy->getRadius()),
                                    isCrit ? sf::Color::Yellow : sf::Color(180, 180, 255));
                }
            }
        }
    } else {
        if (state.mPlayer->getLancerEDamagePhase() != 0) {
            state.mPlayer->setLancerEDamagePhase(0);
        }
    }

    // 5. Lancer Skill R (Charge) — 2 hits at frame 1 (idx 0) and frame 5 (idx 4)
    if (state.mPlayer->getType() == CharacterType::Lancer &&
        state.mPlayer->isPlayingSkillAnimation() &&
        state.mPlayer->getCurrentSkillAnim() == "Skill_R") {
        std::size_t frameIdx = state.mPlayer->getAnimFrameIndex();
        int phase = state.mPlayer->getLancerRDamagePhase();

        bool shouldDamage = false;
        if (phase == 0 && frameIdx >= 0) {
            shouldDamage = true;
            state.mPlayer->setLancerRDamagePhase(1);
        } else if (phase == 1 && frameIdx >= 4) {
            shouldDamage = true;
            state.mPlayer->setLancerRDamagePhase(2);
        }

        if (shouldDamage) {
            float dmg = state.mPlayer->getDamage() * 1.8f;
            for (auto& enemy : state.mEnemies) {
                if (enemy->isDead()) continue;
                if (state.mPlayer->isEnemyInMeleeRange(*enemy, 85.f, 75.f)) {
                    bool isCrit = state.randomChance(state.mGameData.getCritChance());
                    if (isCrit) dmg *= state.mGameData.getCritMultiplier();
                    enemy->takeDamage(dmg);
                    state.addFloatingText(isCrit ? "CRIT " + std::to_string((int)dmg) : std::to_string((int)dmg),
                                    enemy->getPosition() - sf::Vector2f(0.f, enemy->getRadius()),
                                    isCrit ? sf::Color::Yellow : sf::Color(220, 220, 100));
                }
            }
        }
    } else {
        if (state.mPlayer->getLancerRDamagePhase() != 0) {
            state.mPlayer->setLancerRDamagePhase(0);
        }
    }

    // 6. Swordsman Skill E Multi Slash (5 hits: frames 4, 7, 8, 13, 14 -> zero-based 3, 6, 7, 12, 13)
    if (state.mPlayer->getType() == CharacterType::Swordsman &&
        state.mPlayer->isPlayingSkillAnimation() &&
        state.mPlayer->getCurrentSkillAnim() == "Skill_E") {
        std::size_t frameIdx = state.mPlayer->getAnimFrameIndex();
        int phase = state.mPlayer->getSwordsmanEDamagePhase();

        bool shouldDamage = false;
        if (phase == 0 && frameIdx >= 3) {
            shouldDamage = true;
            state.mPlayer->setSwordsmanEDamagePhase(1);
        } else if (phase == 1 && frameIdx >= 6) {
            shouldDamage = true;
            state.mPlayer->setSwordsmanEDamagePhase(2);
        } else if (phase == 2 && frameIdx >= 7) {
            shouldDamage = true;
            state.mPlayer->setSwordsmanEDamagePhase(3);
        } else if (phase == 3 && frameIdx >= 12) {
            shouldDamage = true;
            state.mPlayer->setSwordsmanEDamagePhase(4);
        } else if (phase == 4 && frameIdx >= 13) {
            shouldDamage = true;
            state.mPlayer->setSwordsmanEDamagePhase(5);
        }

        if (shouldDamage) {
            float dmg = state.mPlayer->getDamage();
            for (auto& enemy : state.mEnemies) {
                if (enemy->isDead()) continue;
                if (state.mPlayer->isEnemyInMeleeRange(*enemy, 70.f, 70.f)) {
                    bool isCrit = state.randomChance(state.mGameData.getCritChance());
                    if (isCrit) dmg *= state.mGameData.getCritMultiplier();
                    enemy->takeDamage(dmg);
                    state.addFloatingText(isCrit ? "CRIT " + std::to_string((int)dmg) : std::to_string((int)dmg),
                                    enemy->getPosition() - sf::Vector2f(0.f, enemy->getRadius()),
                                    isCrit ? sf::Color::Yellow : sf::Color(255, 100, 100));
                }
            }
        }
    } else {
        if (state.mPlayer->getSwordsmanEDamagePhase() != 0) {
            state.mPlayer->setSwordsmanEDamagePhase(0);
        }
    }

    // 7. Swordsman Skill R Ultimate Slash (Damage window: frame 6 to 12 -> zero-based 5 to 11)
    if (state.mPlayer->getType() == CharacterType::Swordsman &&
        state.mPlayer->isPlayingSkillAnimation() &&
        state.mPlayer->getCurrentSkillAnim() == "Skill_R") {
        std::size_t frameIdx = state.mPlayer->getAnimFrameIndex();
        if (!state.mMeleeHitRegistered && frameIdx >= 5 && frameIdx <= 11) {
            float dmg = state.mPlayer->getDamage() * 2.5f;
            for (auto& enemy : state.mEnemies) {
                if (enemy->isDead()) continue;
                if (state.mPlayer->isEnemyInMeleeRange(*enemy, 90.f, 80.f)) {
                    bool isCrit = state.randomChance(state.mGameData.getCritChance());
                    if (isCrit) dmg *= state.mGameData.getCritMultiplier();
                    enemy->takeDamage(dmg);
                    state.addFloatingText(isCrit ? "CRIT " + std::to_string((int)dmg) : std::to_string((int)dmg),
                                    enemy->getPosition() - sf::Vector2f(0.f, enemy->getRadius()),
                                    isCrit ? sf::Color::Yellow : sf::Color(255, 50, 50));
                }
            }
            state.mMeleeHitRegistered = true;
        }
    }

    // 3. Ranged Projectiles
    for (auto& proj : state.mProjectiles) {
        if (proj.isExpired()) continue;

        if (proj.isEnemy()) {
            if (state.mPlayer->isDead()) {
                proj = Projectile({0.f, 0.f}, {0.f, 0.f}, 0.f, 0.f);
                continue;
            }
            if (proj.getVelocity() == sf::Vector2f(0.f, 0.f)) {
                // Delayed explosion spell: trigger based on exact frame index
                if (state.mBossBringerDead && proj.getAnimFrameCount() >= 16) {
                    proj = Projectile({0.f, 0.f}, {0.f, 0.f}, 0.f, 0.f);
                    continue;
                }

                bool shouldDamage = false;
                if (proj.getAnimFrameCount() == 16) {
                    // Bringer of Death Spell: Damage at Frame 7 (Index 6)
                    if (proj.getAnimFrameIndex() == 6 && !proj.hasHit()) {
                        shouldDamage = true;
                    }
                } else if (proj.getAnimFrameCount() == 6 || proj.getAnimFrameCount() == 10) {
                    // Necromancer Spell: Damage at Frame 5 (Index 4)
                    // (Assuming 6 frames effect, or just trigger near end)
                    if (proj.getAnimFrameIndex() == 4 && !proj.hasHit()) {
                        shouldDamage = true;
                    }
                } else {
                    // Fallback
                    if (proj.getLifetime() <= 0.05f && !proj.hasHit()) shouldDamage = true;
                }

                if (shouldDamage) {
                    proj.setHasHit(true);
                    if (proj.getDamage() > 0.f) {
                        float pr = state.mPlayer->getRadius();
                        float dx = proj.getPosition().x - state.mPlayer->getPosition().x;
                        float dy = proj.getPosition().y - state.mPlayer->getPosition().y;
                        float dist = std::sqrt(dx * dx + dy * dy);
                        if (dist < proj.getRadius() + pr) {
                            float dmg = proj.getDamage();
                            state.mPlayer->takeDamage(dmg);
                            state.addFloatingText("-" + std::to_string((int)dmg), state.mPlayer->getPosition() - sf::Vector2f(0.f, pr), sf::Color::Red);
                        }
                        AudioManager::instance().playSfx("explosion");
                        state.addFloatingText("BOOM!", proj.getPosition(), sf::Color::Magenta);
                    } else {
                        // Necromancer Summon effect
                        state.mRespawnQueue.push_back({"Skeleton", Region::Center, 1, proj.getPosition(), 0.f});
                        state.addFloatingText("Summon!", proj.getPosition(), sf::Color::Magenta);
                    }
                }
            } else {
                // Moving projectile (arrow, etc)
                float pr = state.mPlayer->getRadius();
                float dx = proj.getPosition().x - state.mPlayer->getPosition().x;
                float dy = proj.getPosition().y - state.mPlayer->getPosition().y;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < proj.getRadius() + pr) {
                    float dmg = proj.getDamage();
                    state.mPlayer->takeDamage(dmg);
                    state.addFloatingText("-" + std::to_string((int)dmg), state.mPlayer->getPosition() - sf::Vector2f(0.f, pr), sf::Color::Red);
                    proj = Projectile({0.f, 0.f}, {0.f, 0.f}, 0.f, 0.f);
                }
            }
        } else {
            // Check collision with enemies
            for (auto& enemy : state.mEnemies) {
                if (enemy->isDead()) continue;
                float r = enemy->getRadius();
                float dx = proj.getPosition().x - enemy->getPosition().x;
                float dy = proj.getPosition().y - enemy->getPosition().y;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < proj.getRadius() + r) {
                    float dmg = proj.getDamage();
                    bool isCrit = state.randomChance(state.mGameData.getCritChance());
                    if (isCrit) dmg *= state.mGameData.getCritMultiplier();

                    if (proj.getRadius() >= 12.f) {
                        AudioManager::instance().playSfx("explosion");
                        state.addFloatingText("BOOM!", proj.getPosition(), sf::Color(255, 165, 0));
                        for (auto& e : state.mEnemies) {
                            if (e->isDead()) continue;
                            float ex = e->getPosition().x - proj.getPosition().x;
                            float ey = e->getPosition().y - proj.getPosition().y;
                            float edist = std::sqrt(ex * ex + ey * ey);
                            if (edist < 150.f) {
                                e->takeDamage(dmg);
                                state.addFloatingText(isCrit ? "CRIT " + std::to_string((int)dmg) : std::to_string((int)dmg),
                                                e->getPosition() - sf::Vector2f(0.f, e->getRadius()),
                                                isCrit ? sf::Color::Yellow : sf::Color::Red);
                            }
                        }
                    } else {
                        enemy->takeDamage(dmg);
                        state.addFloatingText(isCrit ? "CRIT " + std::to_string((int)dmg) : std::to_string((int)dmg),
                                        enemy->getPosition() - sf::Vector2f(0.f, r),
                                        isCrit ? sf::Color::Yellow : sf::Color::White);
                    }
                    proj = Projectile({0.f, 0.f}, {0.f, 0.f}, 0.f, 0.f);
                    break;
                }
            }
        }
    }
}
