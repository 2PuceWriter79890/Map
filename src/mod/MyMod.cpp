#include "mod/MyMod.h"

#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/player/PlayerDieEvent.h>
#include <ll/api/event/player/PlayerRespawnEvent.h>

#include <mc/world/actor/player/Player.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/nbt/CompoundTag.h>

#include <map>
#include <string>
#include <set>
#include <memory>

static const std::set<std::string> allowedPlayers = {
    "Steve",
    "Alex"
};

static std::map<std::string, std::unique_ptr<CompoundTag>> deathInfo;

void plugin_entry(ll::plugin::NativePlugin& self) {
    ll::event::EventBus::getInstance().subscribe<ll::event::PlayerDieEvent>(
        [&](ll::event::PlayerDieEvent& event) {
            auto& player = event.self();
            
            if (allowedPlayers.count(player.getRealName())) {
                auto uuidStr = player.getUuid().asString();
                auto position = player.getPosition();
                auto dimensionId = player.getDimensionId();

                auto tag = std::make_unique<CompoundTag>();
                tag->putFloat("x", position.x);
                tag->putFloat("y", position.y);
                tag->putFloat("z", position.z);
                tag->putInt("dim", dimensionId.id);

                deathInfo[uuidStr] = std::move(tag);
            }
            return true;
        }
    );

    ll::event::EventBus::getInstance().subscribe<ll::event::PlayerRespawnEvent>(
        [&](ll::event::PlayerRespawnEvent& event) {
            auto& player = event.self();
            auto uuidStr = player.getUuid().asString();

            auto it = deathInfo.find(uuidStr);
            if (it != deathInfo.end()) {
                auto const& tag = it->second;
                Vec3 savedPos(tag->getFloat("x"), tag->getFloat("y"), tag->getFloat("z"));
                DimensionId savedDim(tag->getInt("dim"));
                
                player.teleport(Vec3(savedPos.x, savedPos.y + 0.5, savedPos.z), savedDim);

                deathInfo.erase(it);
            }
            return true;
        }
    );
}
