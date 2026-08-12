#pragma once
#include "common/lib.h"

class PlayingState;

class CollisionSystem {
public:
    static void update(
        float dt,
        PlayingState& state
    );
};
