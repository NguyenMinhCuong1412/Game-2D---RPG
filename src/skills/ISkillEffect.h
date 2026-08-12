#pragma once
#include <vector>

namespace sf {
    class RenderWindow;
}

class Player;
class Projectile;

struct ISkillEffect {
    virtual void execute(Player& owner,
                          std::vector<Projectile>& projectiles,
                          sf::RenderWindow& window) = 0;
    virtual ~ISkillEffect() = default;
};
