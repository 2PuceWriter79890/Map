#include "mod/MyMod.h"

#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/item/MapItem.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/saveddata/maps/MapItemSavedData.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/world/actor/player/Player.h"

void registerMapInfoCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "getmapinfo",
        "Get held map info", 
        CommandPermissionLevel::Any
    );

    cmd.overload().execute([](CommandOrigin const& origin, CommandOutput& output) {
        if (auto* player = origin.getPlayer()) {
            auto& heldItem = player->getInventory().getSelectedItem();
            
            if (!heldItem.isMap()) {
                output.error("Please hold a map!");
                return;
            }

            if (auto* nbt = heldItem.getUserData()) {
                if (auto* level = ll::service::getLevel().get()) {
                    if (auto* mapData = level->getMapSavedData().fetchSavedData(MapItem::getMapId(nbt))) {
                        output.success("=== Map Info ===");
                        output.success("Map ID: " + std::to_string(mapData->getMapId().id));
                        output.success("Scale: " + std::to_string(mapData->getScale()));
                        output.success("Locked: " + std::string(heldItem.isLocked() ? "Yes" : "No"));
                        output.success("Center: X=" + std::to_string(mapData->getCenterX()) + 
                                     ", Z=" + std::to_string(mapData->getCenterZ()));
                        return;
                    }
                }
            }
            output.error("Failed to get map data!");
        } else {
            output.error("Only players can use this command");
        }
    });
}

void plugin_init() {
    registerMapInfoCommand(); 
}
