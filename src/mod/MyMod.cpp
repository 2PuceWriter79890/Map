#include "mod/MyMod.h"

#include <ll/api/mod/NativeMod.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/Listener.h>
#include <ll/api/event/player/PlayerDieEvent.h>
#include <ll/api/event/player/PlayerRespawnEvent.h>

#include <mc/world/actor/player/Player.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/FloatTag.h>
#include <mc/nbt/IntTag.h>

#include <map>
#include <string>
#include <set>
#include <memory>

static const std::set<std::string> allowedPlayers = {
    "Steve",
    "Alex"
};

static std::map<std::string, std::unique_ptr<CompoundTag>> deathInfo;

void plugin_entry(ll::mod::NativeMod& self) {
    auto playerDieListener = ll::event::Listener<ll::event::PlayerDieEvent>::create(
        [&](ll::event::PlayerDieEvent& event) {
            auto& player = event.self();
            
            if (allowedPlayers.count(player.getRealName())) {
                auto uuidStr = player.getUuid().asString();
                auto position = player.getPosition();
                auto dimensionId = player.getDimensionId();

                auto tag = std::make_unique<CompoundTag>();
                
                tag->mTags.emplace("x", FloatTag::create(position.x));
                tag->mTags.emplace("y", FloatTag::create(position.y));
                tag->mTags.emplace("z", FloatTag::create(position.z));
                tag->mTags.emplace("dim", IntTag::create(dimensionId.id));

                deathInfo[uuidStr] = std::move(tag);
            }
            return true;
        }
    );
    ll::event::EventBus::getInstance().addListener(playerDieListener);

    auto playerRespawnListener = ll::event::Listener<ll::event::PlayerRespawnEvent>::create(
        [&](ll::event::PlayerRespawnEvent& event) {
            auto& player = event.self();
            auto uuidStr = player.getUuid().asString();

            auto it = deathInfo.find(uuidStr);
            if (it != deathInfo.end()) {
                auto const& tag = it->second;

                Vec3 savedPos(tag->getFloat("x"), tag->getFloat("y"), tag->getFloat("z"));
                DimensionId savedDim(tag->getInt("dim"));
                
                player.teleportTo(Vec3(savedPos.x, savedPos.y + 0.5, savedPos.z), true, 0, 0, false);

                deathInfo.erase(it);
            }
            return true;
        }
    );
    ll::event::EventBus::getInstance().addListener(playerRespawnListener);
}
