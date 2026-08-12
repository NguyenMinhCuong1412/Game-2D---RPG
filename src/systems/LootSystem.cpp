#include "systems/LootSystem.h"
#include "states/PlayingState.h"
#include "core/AudioManager.h"

void LootSystem::spawnLoot(
    LootType type,
    sf::Vector2f position,
    int value,
    PlayingState& state
) {
    const sf::Texture& tex = (type == LootType::Gold) ? state.mTextureHolder.get("gold") :
                             (type == LootType::Potion) ? state.mTextureHolder.get("potion") :
                                                          state.mTextureHolder.get("scroll");

    LootItem item(type, position, tex, value);

    auto size = item.sprite.getTexture().getSize();
    item.sprite.setOrigin(sf::Vector2f(size.x / 2.f, size.y / 2.f));
    item.sprite.setPosition(position);
    // Scale 32x32 textures to approx game size
    item.sprite.setScale(sf::Vector2f(0.75f, 0.75f));

    state.mLootItems.push_back(item);
}

void LootSystem::update(
    float dt,
    PlayingState& state
) {
    (void)dt;
    // 4. Loot Item Gathering
    auto lootIt = state.mLootItems.begin();
    while (lootIt != state.mLootItems.end()) {
        float dx = state.mPlayer->getPosition().x - lootIt->position.x;
        float dy = state.mPlayer->getPosition().y - lootIt->position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 30.f) {
            if (lootIt->type == LootType::Gold) {
                AudioManager::instance().playSfx("gold_pickup");
                state.mPlayer->gainGold(lootIt->value);
                state.addFloatingText("+" + std::to_string(lootIt->value) + " Gold", lootIt->position, sf::Color::Yellow);
            } else if (lootIt->type == LootType::Potion) {
                AudioManager::instance().playSfx("scroll_pickup");
                state.mPlayer->addPotion();
                state.addFloatingText("+1 Potion", lootIt->position, sf::Color::Red);
            } else if (lootIt->type == LootType::SkillScroll) {
                AudioManager::instance().playSfx("scroll_pickup");
                state.mInventorySkills++;
                if (state.randomChance(50)) {
                    const ItemData& atkScroll = state.mGameData.getItem("scroll_attack");
                    state.mPlayer->addDamageScrollCount(1);
                    state.addFloatingText("+" + std::to_string(atkScroll.effectValue) + "% DMG Scroll!",
                                    lootIt->position, sf::Color(255, 165, 0));
                } else {
                    const ItemData& defScroll = state.mGameData.getItem("scroll_defense");
                    state.mPlayer->addDefenseScrollCount(1);
                    state.addFloatingText("+" + std::to_string(defScroll.effectValue) + "% DEF Scroll!",
                                    lootIt->position, sf::Color(100, 200, 255));
                }
            }
            lootIt = state.mLootItems.erase(lootIt);
        } else {
            ++lootIt;
        }
    }
}
