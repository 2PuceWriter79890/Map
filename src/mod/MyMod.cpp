#include "ll/api/mod/NativeMod.h"
#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/Listener.h"
#include "ll/api/event/EventId.h"
#include "ll/api/event/player/PlayerDieEvent.h"
#include "ll/api/event/player/PlayerRespawnEvent.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/SharedAttributes.h"

#include <map>
#include <string>
#include <memory>
#include <utility>

namespace my_mod {

static ll::mod::NativeMod* gSelf = nullptr;
static std::map<std::string, int> gStoredTotalExperience;

int calculateXpForLevel(int level) {
    if (level >= 32) {
        return static_cast<int>(4.5 * level * level - 162.5 * level + 2220);
    }
    if (level >= 17) {
        return static_cast<int>(2.5 * level * level - 40.5 * level + 360);
    }
    return level * level + 6 * level;
}

void onPlayerDie(ll::event::PlayerDieEvent& event) {
    if (!gSelf) return;

    auto& player = event.self();
    
    auto const& levelAttribute = player.getAttribute(Player::LEVEL());
    auto const& expAttribute = player.getAttribute(Player::EXPERIENCE());

    int level = static_cast<int>(levelAttribute.mCurrentValue);
    float progress = expAttribute.mCurrentValue;

    int xpFromLevels = calculateXpForLevel(level);
    int xpInBar = static_cast<int>(progress * player.getXpNeededForNextLevel());
    int totalXp = xpFromLevels + xpInBar;

    gStoredTotalExperience[player.getUuid().asString()] = totalXp;

    const_cast<AttributeInstance&>(levelAttribute).mCurrentValue = 0;
    const_cast<AttributeInstance&>(expAttribute).mCurrentValue = 0.0f;

    gSelf->getLogger().info("Stored {} total XP for player {}.", totalXp, player.getRealName());
}

void onPlayerRespawn(ll::event::PlayerRespawnEvent& event) {
    if (!gSelf) return;

    auto& player = event.self();
    auto uuidStr = player.getUuid().asString();

    auto it = gStoredTotalExperience.find(uuidStr);
    if (it != gStoredTotalExperience.end()) {
        int totalXpToRestore = it->second;

        player.addExperience(totalXpToRestore);
        
        gSelf->getLogger().info("Restored {} total XP for player {}.", totalXpToRestore, player.getRealName());
        gStoredTotalExperience.erase(it);
    }
}

class MyMod {
public:
    static MyMod& getInstance() {
        static MyMod instance;
        return instance;
    }
    MyMod(const MyMod&) = delete;
    MyMod& operator=(const MyMod&) = delete;

    ll::mod::NativeMod& getSelf() { return mSelf; }
    ll::io::Logger& getLogger() { return mLogger; }

    bool load() {
        getLogger().info("RetainXP loading...");
        return true;
    }
    bool enable() {
        getLogger().info("RetainXP enabling...");
        auto& eventBus = ll::event::EventBus::getInstance();

        mPlayerDieListener = eventBus.emplaceListener<ll::event::PlayerDieEvent>(onPlayerDie);
        mPlayerRespawnListener = eventBus.emplaceListener<ll::event::PlayerRespawnEvent>(onPlayerRespawn);
        
        return true;
    }
    bool disable() {
        getLogger().info("RetainXP disabling...");
        auto& eventBus = ll::event::EventBus::getInstance();

        eventBus.removeListener(mPlayerDieListener);
        eventBus.removeListener(mPlayerRespawnListener);

        gStoredTotalExperience.clear();
        return true;
    }

private:
    MyMod() : mSelf(*ll::mod::NativeMod::current()), mLogger(mSelf.getLogger()) {}

    ll::mod::NativeMod& mSelf;
    ll::io::Logger&     mLogger;
    ll::event::ListenerPtr mPlayerDieListener;
    ll::event::ListenerPtr mPlayerRespawnListener;
};


} // namespace my_mod

LL_REGISTER_MOD(my_mod::MyMod, my_mod::MyMod::getInstance());