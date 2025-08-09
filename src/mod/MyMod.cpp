#include "mod/MyMod.h"

#include "MyMod.h"
#include "config.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDieEvent.h"
#include "ll/api/event/player/PlayerRespawnEvent.h"
#include "mc/world/actor/player/Player.h"

namespace my_mod {

MyMod& MyMod::getInstance() {
    static MyMod instance;
    return instance;
}

bool MyMod::load() {
    loadAndCreateDefaultConfig(getSelf());
    return true;
}

bool MyMod::enable() {
    auto& eventBus = ll::event::EventBus::getInstance();

    mPlayerDieListener = ll::event::Listener<ll::event::PlayerDieEvent>::create(
        [this](ll::event::PlayerDieEvent& event) {
            auto& player = event.self();
            
            if (gAllowedPlayers.count(player.getRealName())) {
                int totalXp = player.getTotalExperience();
                mStoredExperience[player.getUuid().asString()] = totalXp;
                player.addExperience(-totalXp);
                player.addLevels(-player.getPlayerLevel());
                player.setExperience(0);
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
                player.addExperience(it->second);
                mStoredExperience.erase(it);
            }
            return true;
        }
    );

    eventBus.addListener(mPlayerDieListener);
    eventBus.addListener(mPlayerRespawnListener);
    
    return true;
}

bool MyMod::disable() {
    auto& eventBus = ll::event::EventBus::getInstance();

    eventBus.removeListener(mPlayerDieListener);
    eventBus.removeListener(mPlayerRespawnListener);

    mStoredExperience.clear();

    return true;
}

}
