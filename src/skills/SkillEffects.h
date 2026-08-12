#pragma once
#include "skills/ISkillEffect.h"
#include "entities/Player.h"
#include "entities/Projectile.h"

class ShieldBlockSkill : public ISkillEffect {
public:
    void execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) override;
};

class WhirlwindSkill : public ISkillEffect {
public:
    void execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) override;
};

class DivineChargeSkill : public ISkillEffect {
public:
    void execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) override;
};

class RocketLauncherSkill : public ISkillEffect {
public:
    void execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) override;
};

class OverchargeSkill : public ISkillEffect {
public:
    void execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) override;
};

class SpreadConeSkill : public ISkillEffect {
public:
    void execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) override;
};

class LancerSpeedBuffSkill : public ISkillEffect {
public:
    void execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) override;
};

class LancerMovingAttackSkill : public ISkillEffect {
public:
    void execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) override;
};

class LancerChargeSkill : public ISkillEffect {
public:
    void execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) override;
};

class SwordsmanSpeedBuffSkill : public ISkillEffect {
public:
    void execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) override;
};

class SwordsmanMultiSlashSkill : public ISkillEffect {
public:
    void execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) override;
};

class SwordsmanUltimateSlashSkill : public ISkillEffect {
public:
    void execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) override;
};
