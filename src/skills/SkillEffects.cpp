#include "skills/SkillEffects.h"
#include <cmath>
#include <filesystem>

void ShieldBlockSkill::execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) {
    (void)projectiles;
    (void)window;
    if (owner.getType() == CharacterType::Knight) {
        owner.setShieldActiveTimer(2.0f);
        owner.playSkillAnimation("Skill_Q", 2.0f);
    }
    owner.setQCooldown(owner.getQCooldownMax());
}

void WhirlwindSkill::execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) {
    (void)projectiles;
    (void)window;
    if (owner.getType() == CharacterType::Knight) {
        owner.setWhirlwindTimer(0.6f);
        owner.setWhirlwindDamagePhase(0);
        owner.playSkillAnimation("Skill_E", 0.60f);
    }
    owner.setECooldown(owner.getECooldownMax());
}

void DivineChargeSkill::execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) {
    (void)projectiles;
    (void)window;
    if (owner.getType() == CharacterType::Knight) {
        owner.startDash(0.35f, 0.8f, owner.getFacingDirection());
        owner.setAttackTimer(0.35f);
        owner.playSkillAnimation("Skill_R", 0.66f);
    }
    owner.setRCooldown(owner.getRCooldownMax());
}

void RocketLauncherSkill::execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) {
    (void)projectiles;
    (void)window;
    if (owner.getType() == CharacterType::Archer) owner.setArcherAtkSpeedBuffTimer(4.0f);
    owner.setQCooldown(owner.getQCooldownMax());
}

void OverchargeSkill::execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) {
    if (owner.getType() == CharacterType::Archer) {
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::Vector2f fireDir = mouseWorldPos - owner.getPosition();
        float dist = std::sqrt(fireDir.x * fireDir.x + fireDir.y * fireDir.y);
        if (dist > 0.f) {
            fireDir /= dist;
            owner.setFacingDirection(fireDir);
            owner.playSkillAnimation("Attack", 0.72f);
            owner.setAttackTimer(0.72f);
            owner.setPendingSkillEProjectile(true);
            owner.setECooldown(owner.getECooldownMax());
        }
    }
}

void SpreadConeSkill::execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) {
    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    sf::Vector2f fireDir = mouseWorldPos - owner.getPosition();
    float dist = std::sqrt(fireDir.x * fireDir.x + fireDir.y * fireDir.y);

    if (owner.getType() == CharacterType::Archer) {
        if (dist > 0.f) {
            fireDir /= dist;
            owner.setFacingDirection(fireDir);
            owner.playSkillAnimation("Skill_R", 0.96f);
            owner.setAttackTimer(0.96f);
            owner.setPendingSkillRProjectile(true);
            owner.setRCooldown(owner.getRCooldownMax());
        }
    } else {
        if (dist > 0.f) {
            fireDir /= dist;
            owner.setFacingDirection(fireDir);
            float baseAngle = std::atan2(fireDir.y, fireDir.x);
            float angles[] = {-0.3f, -0.15f, 0.f, 0.15f, 0.3f};

            for (float offset : angles) {
                float angle = baseAngle + offset;
                sf::Vector2f dir = {std::cos(angle), std::sin(angle)};
                sf::Vector2f projVelocity = dir * 650.f;
                sf::Vector2f spawnPos = owner.getPosition() + dir * 25.f;
                projectiles.push_back(Projectile(spawnPos, projVelocity, owner.getDamage() * 1.5f, 1.2f, 8.f, sf::Color::Magenta));
            }
            owner.setRCooldown(owner.getRCooldownMax());
        }
    }
}

void LancerSpeedBuffSkill::execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) {
    (void)projectiles;
    (void)window;
    owner.setLancerSpeedBuffTimer(3.0f);
    owner.setQCooldown(owner.getQCooldownMax());
}

void LancerMovingAttackSkill::execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) {
    (void)projectiles;
    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    sf::Vector2f dashDir = mouseWorldPos - owner.getPosition();
    float dist = std::sqrt(dashDir.x * dashDir.x + dashDir.y * dashDir.y);
    if (dist > 0.f) dashDir /= dist;
    else dashDir = owner.getFacingDirection();

    owner.setFacingDirection(dashDir);
    owner.startDash(0.25f, 0.6f, dashDir);
    owner.setAttackTimer(0.25f);
    owner.playSkillAnimation("Skill_E", 0.72f);
    owner.setECooldown(owner.getECooldownMax());
}

void LancerChargeSkill::execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) {
    (void)projectiles;
    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    sf::Vector2f dashDir = mouseWorldPos - owner.getPosition();
    float dist = std::sqrt(dashDir.x * dashDir.x + dashDir.y * dashDir.y);
    if (dist > 0.f) dashDir /= dist;
    else dashDir = owner.getFacingDirection();

    owner.setFacingDirection(dashDir);
    owner.startDash(0.35f, 0.8f, dashDir);
    owner.setAttackTimer(0.35f);
    owner.playSkillAnimation("Skill_R", 0.64f);
    owner.setRCooldown(owner.getRCooldownMax());
}

void SwordsmanSpeedBuffSkill::execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) {
    (void)projectiles;
    (void)window;
    owner.setSwordsmanAtkSpeedBuffTimer(4.0f);
    owner.setQCooldown(owner.getQCooldownMax());
}

void SwordsmanMultiSlashSkill::execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) {
    (void)projectiles;
    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    sf::Vector2f dir = mouseWorldPos - owner.getPosition();
    float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (dist > 0.f) dir /= dist;
    else dir = owner.getFacingDirection();

    owner.setFacingDirection(dir);
    owner.setAttackTimer(0.75f);
    owner.playSkillAnimation("Skill_E", 0.75f);
    owner.setECooldown(owner.getECooldownMax());
}

void SwordsmanUltimateSlashSkill::execute(Player& owner, std::vector<Projectile>& projectiles, sf::RenderWindow& window) {
    (void)projectiles;
    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    sf::Vector2f dashDir = mouseWorldPos - owner.getPosition();
    float dist = std::sqrt(dashDir.x * dashDir.x + dashDir.y * dashDir.y);
    if (dist > 0.f) dashDir /= dist;
    else dashDir = owner.getFacingDirection();

    owner.setFacingDirection(dashDir);
    owner.startDash(0.35f, 0.8f, dashDir);
    owner.setAttackTimer(0.84f);
    owner.playSkillAnimation("Skill_R", 0.84f);
    owner.setRCooldown(owner.getRCooldownMax());
}
