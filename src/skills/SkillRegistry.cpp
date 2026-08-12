#include "skills/SkillRegistry.h"
#include "skills/SkillEffects.h"

SkillRegistry& SkillRegistry::getInstance() {
    static SkillRegistry instance;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        instance.init();
    }
    return instance;
}

void SkillRegistry::registerSkill(const std::string& skillId, std::unique_ptr<ISkillEffect> skill) { mRegistry[skillId] = std::move(skill); }

ISkillEffect* SkillRegistry::getSkill(const std::string& skillId) const {
    auto it = mRegistry.find(skillId);
    if (it != mRegistry.end()) return it->second.get();
    return nullptr;
}

void SkillRegistry::init() {
    registerSkill("Knight_Q", std::make_unique<ShieldBlockSkill>());
    registerSkill("Knight_E", std::make_unique<WhirlwindSkill>());
    registerSkill("Knight_R", std::make_unique<DivineChargeSkill>());

    registerSkill("Lancer_Q", std::make_unique<LancerSpeedBuffSkill>());
    registerSkill("Lancer_E", std::make_unique<LancerMovingAttackSkill>());
    registerSkill("Lancer_R", std::make_unique<LancerChargeSkill>());

    registerSkill("Archer_Q", std::make_unique<RocketLauncherSkill>());
    registerSkill("Archer_E", std::make_unique<OverchargeSkill>());
    registerSkill("Archer_R", std::make_unique<SpreadConeSkill>());

    registerSkill("Swordsman_Q", std::make_unique<SwordsmanSpeedBuffSkill>());
    registerSkill("Swordsman_E", std::make_unique<SwordsmanMultiSlashSkill>());
    registerSkill("Swordsman_R", std::make_unique<SwordsmanUltimateSlashSkill>());
}
