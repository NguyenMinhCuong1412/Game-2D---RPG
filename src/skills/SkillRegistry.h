#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "skills/ISkillEffect.h"

class SkillRegistry {

private:
    SkillRegistry() = default;
    ~SkillRegistry() = default;
    SkillRegistry(const SkillRegistry&) = delete;
    SkillRegistry& operator=(const SkillRegistry&) = delete;

    std::unordered_map<std::string, std::unique_ptr<ISkillEffect>> mRegistry;
public:
    static SkillRegistry& getInstance();

    void init();
    void registerSkill(const std::string& skillId, std::unique_ptr<ISkillEffect> skill);
    ISkillEffect* getSkill(const std::string& skillId) const;
};
