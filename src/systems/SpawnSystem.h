#pragma once
#include "common/lib.h"
#include "entities/Enemy.h"
#include "database/GameData.h"
#include "states/PlayingState.h"

class SpawnSystem {
public:
    static void spawnInitialEnemies(
        std::vector<std::unique_ptr<Enemy>>& enemies,
        const GameData& gameData,
        PlayingState& state
    );

    static void update(
        float dt,
        int playerLevel,
        std::vector<std::unique_ptr<Enemy>>& enemies,
        std::vector<RespawnRequest>& respawnQueue,
        const GameData& gameData,
        PlayingState& state,
        bool& bossGoblinDead,
        bool& bossDemonDead,
        bool& bossBringerDead
    );
};
