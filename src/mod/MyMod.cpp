#include "mod/MyMod.h"
#include "mod/config.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDieEvent.h"
#include "ll/api/event/player/PlayerRespawnEvent.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/SharedAttributes.h"

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
                auto& levelAttribute = player.getAttribute(Player::LEVEL);
                auto& expAttribute = player.getAttribute(Player::EXPERIENCE);

                int level = static_cast<int>(levelAttribute.mCurrentValue);
                float progress = expAttribute.mCurrentValue;

                mStoredExperience[player.getUuid().asString()] = {level, progress};

                levelAttribute.mCurrentValue = 0;
                expAttribute.mCurrentValue = 0.0f;
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
                const auto& [level, progress] = it->second;

                player.addLevels(level);
                player.getAttribute(Player::EXPERIENCE).mCurrentValue = progress;
                
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

} // namespace my_mod

extern "C" {
    _declspec(dllexport) bool ll_load() { 
        return my_mod::MyMod::getInstance().load();
    }
    _declspec(dllexport) bool ll_enable() { 
        return my_mod::MyMod::getInstance().enable();
    }
    _declspec(dllexport) bool ll_disable() { 
        return my_mod::MyMod::getInstance().disable();
    }
}