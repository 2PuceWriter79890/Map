#include "mod/MyMod.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/item/MapItem.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/saveddata/maps/MapItemSavedData.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/level/Level.h"

void registerMapInfoCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "getmapinfo",
        "Get held map info", 
        CommandPermissionLevel::Any
    );

    cmd.overload().execute([](CommandOrigin const& origin, CommandOutput& output) {
        auto* entity = origin.getEntity();
        if (!entity || !entity->isPlayer()) {
            output.error("Only players can use this command");
            return;
        }

        auto* player = static_cast<Player*>(entity);
        PlayerInventory& inventory = player->getInventory();
        ItemStack const& heldItem = inventory.getSelectedItem();
        
        if (!heldItem.isMap()) {
            output.error("Please hold a map!");
            return;
        }

        CompoundTag const* nbt = heldItem.getUserData();
        if (!nbt) {
            output.error("Failed to get map NBT data!");
            return;
        }

        ActorUniqueID mapId = MapItem::getMapId(nbt);
        Level* level = ll::service::getLevel().get();
        if (!level) {
            output.error("Failed to get level!");
            return;
        }

        MapItemSavedData* mapData = level->getMapSavedData().fetchSavedData(mapId);
        if (!mapData) {
            output.error("Failed to get map data!");
            return;
        }

        output.success("=== Map Info ===");
        output.success("Map ID: " + std::to_string(mapId.id));
        output.success("Scale: " + std::to_string(mapData->getScale()));
        output.success("Locked: " + std::string(heldItem.isLocked() ? "Yes" : "No"));
        output.success("Center: X=" + std::to_string(mapData->getCenterX()) + 
                      ", Z=" + std::to_string(mapData->getCenterZ()));
    });
}

void plugin_init() {
    registerMapInfoCommand();
}
