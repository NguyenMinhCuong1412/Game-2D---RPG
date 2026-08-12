#include "systems/SpawnSystem.h"
#include "core/AudioManager.h"
#include "states/PlayingState.h"

// Helper: generate a random position within map bounds, away from player and other enemies
static sf::Vector2f generateRandomSpawnPos(
    PlayingState& state,
    const std::vector<std::unique_ptr<Enemy>>& enemies,
    sf::Vector2f playerPos,
    float mapMin, float mapMax, float minPlayerDist, float minEnemyDist
) {
    const int MAX_ATTEMPTS = 30;

    for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        float x = static_cast<float>(state.randomInt(static_cast<int>(mapMin), static_cast<int>(mapMax)));
        float y = static_cast<float>(state.randomInt(static_cast<int>(mapMin), static_cast<int>(mapMax)));
        sf::Vector2f pos(x, y);

        // Check distance from player
        float dx = pos.x - playerPos.x;
        float dy = pos.y - playerPos.y;
        if (std::sqrt(dx * dx + dy * dy) < minPlayerDist) continue;

        // Check distance from existing enemies
        bool tooClose = false;
        for (const auto& e : enemies) {
            float ex = pos.x - e->getPosition().x;
            float ey = pos.y - e->getPosition().y;
            if (std::sqrt(ex * ex + ey * ey) < minEnemyDist) {
                tooClose = true;
                break;
            }
        }
        if (tooClose) continue;

        return pos;
    }

    // Fallback: return a random position even if constraints weren't fully met
    float x = static_cast<float>(state.randomInt(static_cast<int>(mapMin), static_cast<int>(mapMax)));
    float y = static_cast<float>(state.randomInt(static_cast<int>(mapMin), static_cast<int>(mapMax)));
    return {x, y};
}

void SpawnSystem::spawnInitialEnemies(
    std::vector<std::unique_ptr<Enemy>>& enemies,
    const GameData& gameData,
    PlayingState& state
) {
    int pLvl = state.getPlayer().getLevel();
    const auto& spawnList = gameData.getSpawnList();

    // If level >= 29 and Demon is dead, we clear normal monsters and only spawn final boss Bringer Of Death
    if (pLvl >= 29 && state.mBossDemonDead) {
        enemies.clear();
        state.mRespawnQueue.clear();
        if (!state.mBossBringerDead) {
            sf::Vector2f center = state.mMap.getCenter();
            auto boss = std::make_unique<Enemy>("Bringer Of Death", Region::Center, pLvl, center, true, gameData);
            state.setupEnemyAnimations(boss.get());
            enemies.push_back(std::move(boss));
        }
        return;
    }

    for (const auto& entry : spawnList) {
        if (entry.isBoss) {
            // Spawn bosses if player level meets the requirement exactly and they are not dead yet
            if (entry.enemyName == "Goblin Giant" && pLvl >= 5 && !state.mBossGoblinDead) {
                bool exists = false;
                for (const auto& e : enemies) { if (e->getName() == "Goblin Giant") { exists = true; break; } }
                if (!exists) {
                    // Clear normal enemies for boss encounter, but preserve them in respawn queue
                    for (const auto& e : enemies) {
                        if (!e->getIsBoss()) {
                            state.mRespawnQueue.push_back({e->getName(), e->getRegion(), e->getLevel(), e->getPosition(), gameData.getRespawnTime()});
                        }
                    }
                    enemies.clear();
                    sf::Vector2f center = state.mMap.getCenter();
                    auto newBoss = std::make_unique<Enemy>("Goblin Giant", Region::Center, pLvl, center, true, gameData);
                    state.setupEnemyAnimations(newBoss.get());
                    enemies.push_back(std::move(newBoss));
                    state.mBossEncounterActive = true;
                    state.mBossCheckpointPos = state.getPlayer().getPosition();
                    AudioManager::instance().stopMusic();
                    AudioManager::instance().playSfx("boss_encounter");
                    state.addFloatingText("BOSS GOBLIN GIANT HAS SPAWNED!", state.getPlayer().getPosition(), sf::Color::Red);
                    return; // Boss encounter: only boss on map
                }
            }
            else if (entry.enemyName == "Demon" && pLvl >= 13 && state.mBossGoblinDead && !state.mBossDemonDead) {
                bool exists = false;
                for (const auto& e : enemies) { if (e->getName() == "Demon") { exists = true; break; } }
                if (!exists) {
                    for (const auto& e : enemies) {
                        if (!e->getIsBoss()) {
                            state.mRespawnQueue.push_back({e->getName(), e->getRegion(), e->getLevel(), e->getPosition(), gameData.getRespawnTime()});
                        }
                    }
                    enemies.clear();
                    sf::Vector2f center = state.mMap.getCenter();
                    auto newBoss = std::make_unique<Enemy>("Demon", Region::Center, pLvl, center, true, gameData);
                    state.setupEnemyAnimations(newBoss.get());
                    enemies.push_back(std::move(newBoss));
                    state.mBossEncounterActive = true;
                    state.mBossCheckpointPos = state.getPlayer().getPosition();
                    AudioManager::instance().stopMusic();
                    AudioManager::instance().playSfx("boss_encounter");
                    state.addFloatingText("BOSS DEMON HAS SPAWNED!", state.getPlayer().getPosition(), sf::Color::Red);
                    return;
                }
            }
            continue;
        }

        // Spawn a random normal monster suitable for the current player level
        // Use random position instead of fixed spawn point
        std::string finalName = state.selectRandomMonsterForLevel(pLvl);
        sf::Vector2f randomPos = generateRandomSpawnPos(state, enemies, state.getPlayer().getPosition(), 200.f, 3800.f, 200.f, 100.f);
        auto newEnemy = std::make_unique<Enemy>(finalName, entry.region, pLvl, randomPos, false, gameData);
        state.setupEnemyAnimations(newEnemy.get());
        enemies.push_back(std::move(newEnemy));
    }
}

void SpawnSystem::update(
    float dt,
    int playerLevel,
    std::vector<std::unique_ptr<Enemy>>& enemies,
    std::vector<RespawnRequest>& respawnQueue,
    const GameData& gameData,
    PlayingState& state,
    bool& bossGoblinDead,
    bool& bossDemonDead,
    bool& bossBringerDead
) {
    // 1. Dynamic Boss Spawning — all bosses clear normal enemies and spawn at map center
    if (playerLevel >= 5 && !bossGoblinDead) {
        bool alive = false;
        for (const auto& e : enemies) { if (e->getName() == "Goblin Giant") { alive = true; break; } }
        if (!alive) {
            // Clear normal enemies for boss encounter, but preserve them in respawn queue
            for (const auto& e : enemies) {
                if (!e->getIsBoss()) {
                    respawnQueue.push_back({e->getName(), e->getRegion(), e->getLevel(), e->getPosition(), gameData.getRespawnTime()});
                }
            }
            enemies.clear();
            state.mProjectiles.clear();
            sf::Vector2f center = state.mMap.getCenter();
            auto boss = std::make_unique<Enemy>("Goblin Giant", Region::Center, playerLevel, center, true, gameData);
            state.setupEnemyAnimations(boss.get());
            enemies.push_back(std::move(boss));
            state.mBossEncounterActive = true;
            state.mBossCheckpointPos = state.getPlayer().getPosition();
            AudioManager::instance().stopMusic();
            AudioManager::instance().playSfx("boss_encounter");
            state.addFloatingText("BOSS GOBLIN GIANT HAS SPAWNED!", state.getPlayer().getPosition(), sf::Color::Red);
        }
    }
    if (playerLevel >= 13 && bossGoblinDead && !bossDemonDead) {
        bool alive = false;
        for (const auto& e : enemies) { if (e->getName() == "Demon") { alive = true; break; } }
        if (!alive) {
            for (const auto& e : enemies) {
                if (!e->getIsBoss()) {
                    respawnQueue.push_back({e->getName(), e->getRegion(), e->getLevel(), e->getPosition(), gameData.getRespawnTime()});
                }
            }
            enemies.clear();
            state.mProjectiles.clear();
            sf::Vector2f center = state.mMap.getCenter();
            auto boss = std::make_unique<Enemy>("Demon", Region::Center, playerLevel, center, true, gameData);
            state.setupEnemyAnimations(boss.get());
            enemies.push_back(std::move(boss));
            state.mBossEncounterActive = true;
            state.mBossCheckpointPos = state.getPlayer().getPosition();
            AudioManager::instance().stopMusic();
            AudioManager::instance().playSfx("boss_encounter");
            state.addFloatingText("BOSS DEMON HAS SPAWNED!", state.getPlayer().getPosition(), sf::Color::Red);
        }
    }
    if (playerLevel >= 29 && bossDemonDead && !bossBringerDead) {
        bool alive = false;
        for (const auto& e : enemies) { if (e->getName() == "Bringer Of Death") { alive = true; break; } }
        if (!alive) {
            for (const auto& e : enemies) {
                if (!e->getIsBoss()) {
                    respawnQueue.push_back({e->getName(), e->getRegion(), e->getLevel(), e->getPosition(), gameData.getRespawnTime()});
                }
            }
            enemies.clear();
            state.mProjectiles.clear();
            sf::Vector2f center = state.mMap.getCenter();
            auto boss = std::make_unique<Enemy>("Bringer Of Death", Region::Center, playerLevel, center, true, gameData);
            state.setupEnemyAnimations(boss.get());
            enemies.push_back(std::move(boss));
            state.mBossEncounterActive = true;
            state.mBossCheckpointPos = state.getPlayer().getPosition();
            AudioManager::instance().stopMusic();
            AudioManager::instance().playSfx("boss_encounter");
            state.addFloatingText("FINAL BOSS BRINGER OF DEATH HAS SPAWNED!", state.getPlayer().getPosition(), sf::Color::Red);
        }
        // No minion spawning during boss encounter
        return;
    }

    // If any boss is alive, don't process normal respawns
    for (const auto& e : enemies) {
        if (e->getIsBoss() && !e->isDead()) return;
    }

    // 2. Process Respawn Queue — random position for respawned enemies
    auto rit = respawnQueue.begin();
    while (rit != respawnQueue.end()) {
        rit->timer -= dt;
        if (rit->timer <= 0.f) {
            if (playerLevel < 29) {
                int scaledLvl = std::max(rit->level, playerLevel);
                std::string spawnName = rit->name;
                if (spawnName != "Goblin Giant" && spawnName != "Demon" && spawnName != "Bringer Of Death") {
                    spawnName = state.selectRandomMonsterForLevel(scaledLvl);
                }
                // Random position instead of reusing old position
                sf::Vector2f randomPos = generateRandomSpawnPos(state, enemies, state.getPlayer().getPosition(), 200.f, 3800.f, 200.f, 80.f);
                auto respawned = std::make_unique<Enemy>(spawnName, rit->region, scaledLvl, randomPos, false, gameData);
                state.setupEnemyAnimations(respawned.get());
                enemies.push_back(std::move(respawned));
            }
            rit = respawnQueue.erase(rit);
        } else {
            ++rit;
        }
    }
}
