#pragma once
#include "common/lib.h"
#include "database/GameData.h"
#include "states/PlayingState.h"

class LootSystem {
public:
    static void spawnLoot(
        LootType type,
        sf::Vector2f position,
        int value,
        PlayingState& state
    );

    static void update(
        float dt,
        PlayingState& state
    );
};
