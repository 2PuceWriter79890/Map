#include "mod/MyMod.h"

#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/EventId.h"
#include "ll/api/event/player/PlayerDieEvent.h"
#include "ll/api/event/player/PlayerRespawnEvent.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/SharedAttributes.h"

namespace my_mod {

int calculateXpForLevel(int level) {
    if (level >= 32) {
        return static_cast<int>(4.5 * level * level - 162.5 * level + 2220);
    }
    if (level >= 17) {
        return static_cast<int>(2.5 * level * level - 40.5 * level + 360);
    }
    return level * level + 6 * level;
}

MyMod& MyMod::getInstance() {
    static MyMod instance;
    return instance;
}

bool MyMod::load() {
    getLogger().info("§eRetainXP 加载中...");
    return true;
}

bool MyMod::enable() {
    getLogger().info("§eRetainXP 启用中...");
    auto& eventBus = ll::event::EventBus::getInstance();

    mPlayerDieListener = ll::event::Listener<ll::event::PlayerDieEvent>::create(
        [this](ll::event::PlayerDieEvent& event) {
            auto& player = event.self();
            
            auto const& levelAttribute = player.getAttribute(Player::LEVEL());
            auto const& expAttribute = player.getAttribute(Player::EXPERIENCE());

            int level = static_cast<int>(levelAttribute.mCurrentValue);
            float progress = expAttribute.mCurrentValue;

            int xpFromLevels = calculateXpForLevel(level);
            int xpInBar = static_cast<int>(progress * player.getXpNeededForNextLevel());
            int totalXp = xpFromLevels + xpInBar;

            if (totalXp > 0) {
                mStoredExperience[player.getUuid().asString()] = totalXp;
                player.addExperience(-totalXp);
                getLogger().info("Stored and removed {} total XP for player {}.", totalXp, player.getRealName());
            }
            
            return true;
        }
    );

    mPlayerRespawnListener = ll::event::Listener<ll::event::PlayerRespawnEvent>::create(
        [this](ll::event::PlayerRespawnEvent& event) {
            auto& player = event.self();
            auto uuidStr = player.getUuid().asString();

            auto it = mStoredExperience.find(uuidStr);
            if (it != mStoredExperience.end()) {
                int totalXpToRestore = it->second;
                player.addExperience(totalXpToRestore);
                getLogger().info("Restored {} total XP for player {}.", totalXpToRestore, player.getRealName());
                mStoredExperience.erase(it);
            }
            return true;
        }
    );

    eventBus.addListener(mPlayerDieListener, ll::event::getEventId<ll::event::PlayerDieEvent>);
    eventBus.addListener(mPlayerRespawnListener, ll::event::getEventId<ll::event::PlayerRespawnEvent>);
    
    return true;
}

bool MyMod::disable() {
    getLogger().info("§eRetainXP 关闭中...");
    auto& eventBus = ll::event::EventBus::getInstance();

    eventBus.removeListener(mPlayerDieListener);
    eventBus.removeListener(mPlayerRespawnListener);

    mStoredExperience.clear();
    return true;
}

} // namespace my_mod

LL_REGISTER_MOD(my_mod::MyMod, my_mod::MyMod::getInstance());