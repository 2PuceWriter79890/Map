#include "mod/MyMod.h"
#include "mod/config.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDieEvent.h"
#include "ll/api/event/player/PlayerRespawnEvent.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/attribute/AttributeInstance.h"
#include "mc/world/actor/attribute/SharedAttributes.h"

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
                auto& levelAttribute = player.getAttribute(SharedAttributes::PLAYER_LEVEL);
                auto& expAttribute = player.getAttribute(SharedAttributes::PLAYER_EXPERIENCE_AND_LEVEL);

                int level = static_cast<int>(levelAttribute.getCurrentValue());
                float progress = expAttribute.getCurrentValue();

                mStoredExperience[player.getUuid().asString()] = {level, progress};

                levelAttribute.setCurrentValue(0);
                expAttribute.setCurrentValue(0.0f);
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
                player.getAttribute(SharedAttributes::PLAYER_EXPERIENCE_AND_