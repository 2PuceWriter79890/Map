#include "mod/MyMod.h"

#include "ll/api/command/CommandRegistrar.h"
#include "mc/nbt/CompoundTag.h"
#include "gmlib/mc/world/Level.h"
#include "gmlib/mc/world/actor/OfflinePlayer.h"
#include "gmlib/mc/world/actor/Player.h"

namespace {

// 清理单个玩家的物品
void cleanPlayerItems(gmlib::GMPlayer& player, const std::string& itemId) {
    int removed = 0;
    
    // 获取玩家所有物品
    auto items = player.getItems(itemId);
    
    // 清理物品
    for (auto& itemRef : items) {
        if (itemRef && itemRef->getTypeName() == itemId) {
            removed += itemRef->getCount();
            itemRef->setCount(0);
        }
    }
    
    if (removed > 0) {
        player.sendText(fmt::format("Removed {} {} from your inventory", removed, itemId));
    }
}

// 清理所有玩家(在线和离线)
void cleanAllPlayersItems(const std::string& itemId) {
    int totalRemoved = 0;
    
    // 1. 处理在线玩家
    auto onlinePlayers = gmlib::GMLevel::getInstance()->getAllPlayers();
    for (auto player : onlinePlayers) {
        cleanPlayerItems(*player, itemId);
    }
    
    // 2. 处理离线玩家
    auto level = gmlib::GMLevel::getInstance();
    if (!level) return;
    
    // 获取所有玩家数据文件
    auto playerDataPath = "./world/playerdata/";
    auto files = ll::file::getAllFiles(playerDataPath, false);
    
    for (const auto& file : files) {
        if (file.ends_with(".dat")) {
            try {
                // 读取玩家数据
                auto uuid = file.substr(0, file.size() - 4);
                auto playerData = gmlib::GMPlayer::getOfflinePlayerData(uuid);
                
                if (!playerData) continue;
                
                // 处理背包物品
                if (playerData->contains("Inventory")) {
                    auto& inventory = playerData->getList("Inventory");
                    for (int i = inventory.size() - 1; i >= 0; --i) {
                        auto item = inventory.getCompound(i);
                        if (item.getString("id") == itemId) {
                            totalRemoved += item.getByte("Count", 1);
                            inventory.remove(i);
                        }
                    }
                }
                
                // 处理末影箱物品
                if (playerData->contains("EnderItems")) {
                    auto& enderItems = playerData->getList("EnderItems");
                    for (int i = enderItems.size() - 1; i >= 0; --i) {
                        auto item = enderItems.getCompound(i);
                        if (item.getString("id") == itemId) {
                            totalRemoved += item.getByte("Count", 1);
                            enderItems.remove(i);
                        }
                    }
                }
                
                // 保存修改后的数据
                gmlib::GMPlayer::saveOfflinePlayerData(uuid, *playerData);
                
            } catch (...) {
            }
        }
    }
    
    // 广播清理结果
    if (totalRemoved > 0) {
        auto msg = fmt::format("Cleaned total {} {} from all players", totalRemoved, itemId);
        level->broadcast(msg);
    }
}

// 注册命令
void registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "cleanitems",
        "Remove specified item from all players' inventories",
        CommandPermissionLevel::GameDirectors
    );
    
    cmd.overload<std::string>()
        .required("itemId")
        .execute([](CommandOrigin const& origin, CommandOutput& output, std::string const& itemId) {
            cleanAllPlayersItems(itemId);
            output.success(fmt::format("Started cleaning {} from all players", itemId));
        });
}

} // namespace

void PluginInit() {
    registerCommand();
}
