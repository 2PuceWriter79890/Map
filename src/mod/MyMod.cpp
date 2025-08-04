#include "mod/MyMod.h"

#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/io/Logger.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/item/MapItem.h"
#include "mc/world/level/saveddata/maps/MapItemSavedData.h"

void registerMapInfoCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "getmapinfo",
        "获取手持地图信息",
        CommandPermissionLevel::Any
    );

    cmd.overload().execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        if (auto* player = origin.getPlayer()) {
            ItemStack* heldItem = player->getHandSlot();
            
            if (!heldItem || !heldItem->isMap()) {
                output.error("请手持一张地图！");
                return 0;
            }

            auto mapId = MapItem::getMapId(*heldItem);
            
            auto* mapData = ll::service::getLevel()->getMapSavedData().fetchSavedData(mapId);
            
            if (!mapData) {
                output.error("无法获取地图数据！");
                return 0;
            }

            output.success("=== 地图信息 ===");
            output.success(fmt::format("地图ID: {}", mapId));
            output.success(fmt::format("地图等级(比例尺): {}", mapData->getScale()));
            output.success(fmt::format("是否上锁: {}", heldItem->isLocked() ? "是" : "否"));
            output.success(fmt::format("地图中心坐标: X={}, Z={}", mapData->getCenterX(), mapData->getCenterZ()));
            
        } else {
            output.error("只有玩家可以执行此命令");
        }
        return 0;
    }>();
}

void plugin_init() {
    static ll::io::Logger logger("MapInfo");
    registerMapInfoCommand();
    logger.info("地图信息命令已注册 - /getmapinfo");
}
