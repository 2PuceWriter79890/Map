#include "mod/MyMod.h"

#include "ll/api/io/Logger.h"
#include "ll/api/command/CommandRegistrar.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include "gmlib/mc/world/actor/OfflinePlayer.h"

namespace {

auto& logger = ll::Logger::createLogger("ItemCleaner");

// 清理单个离线玩家的指定物品
void cleanOfflinePlayer(gmlib::OfflinePlayer& offlinePlayer, std::string_view itemId, int& totalRemoved) {
    auto nbt = offlinePlayer.getNbt();
    if (!nbt) {
        logger.debug("Player {} has no NBT data", offlinePlayer.getServerId());
        return;
    }

    auto& playerData = nbt.value();
    int removed = 0;
    std::string playerName = playerData.getString("Name", offlinePlayer.getServerId());

    // 清理背包
    if (playerData.contains("Inventory")) {
        auto& inventoryList = playerData.getList("Inventory");
        for (int i = inventoryList.size() - 1; i >= 0; --i) {
            auto* itemTag = inventoryList.getCompound(i);
            if (itemTag && itemTag->getString("id") == itemId) {
                removed += itemTag->getByte("Count", 1);
                inventoryList.remove(i);
            }
        }
    }

    // 清理末影箱
    if (playerData.contains("EnderItems")) {
        auto& enderList = playerData.getList("EnderItems");
        for (int i = enderList.size() - 1; i >= 0; --i) {
            auto* itemTag = enderList.getCompound(i);
            if (itemTag && itemTag->getString("id") == itemId) {
                removed += itemTag->getByte("Count", 1);
                enderList.remove(i);
            }
        }
    }

    if (removed > 0) {
        if (!offlinePlayer.setNbt(playerData)) {
            logger.error("Failed to save NBT data for {}", playerName);
            return;
        }
        totalRemoved += removed;
        logger.info("Removed {} {} from {}", removed, itemId, playerName);
    }
}

// 清理单个在线玩家的指定物品
void cleanOnlinePlayer(Player* player, std::string_view itemId, int& totalRemoved) {
    int removed = 0;
    std::string playerName = player->getName();

    // 清理背包
    auto& inventory = player->getInventory();
    for (int i = 0; i < inventory.getSize(); ++i) {
        auto item = inventory.getItem(i);
        if (item && item->getTypeName() == itemId) {
            removed += item->getCount();
            inventory.removeItem(i, item->getCount());
        }
    }

    // 清理末影箱
    auto& enderChest = player->getEnderChestContainer();
    for (int i = 0; i < enderChest.getSize(); ++i) {
        auto item = enderChest.getItem(i);
        if (item && item->getTypeName() == itemId) {
            removed += item->getCount();
            enderChest.removeItem(i, item->getCount());
        }
    }

    if (removed > 0) {
        totalRemoved += removed;
        logger.info("Removed {} {} from online player {}", removed, itemId, playerName);
        player->sendMessage(fmt::format("§cRemoved {} {} from your inventory", removed, itemId));
    }
}

// 清理所有玩家的指定物品
void cleanAllPlayers(std::string_view itemId, CommandOutput& output) {
    logger.info("Starting cleanup of {} from all players...", itemId);
    output.success("Starting cleanup process...");

    int totalRemoved = 0;
    auto startTime = std::chrono::steady_clock::now();

    // 处理在线玩家
    auto onlinePlayers = Level::getAllPlayers();
    logger.debug("Processing {} online players", onlinePlayers.size());
    for (auto player : onlinePlayers) {
        cleanOnlinePlayer(player, itemId, totalRemoved);
    }

    // 处理离线玩家
    auto offlinePlayers = gmlib::OfflinePlayer::getAllOfflinePlayers();
    logger.debug("Processing {} offline players", offlinePlayers.size());
    for (auto& offlinePlayer : offlinePlayers) {
        cleanOfflinePlayer(offlinePlayer, itemId, totalRemoved);
    }

    auto duration = std::chrono::steady_clock::now() - startTime;
    double timeTaken = std::chrono::duration<double>(duration).count();
    
    logger.info("Cleanup completed. Total removed: {} {} (took {:.2f}s)",
               totalRemoved, itemId, timeTaken);
    
    output.success(fmt::format("Removed total {} {} from all players (took {:.2f}s)", 
                             totalRemoved, itemId, timeTaken));

    // 向在线玩家广播结果
    if (totalRemoved > 0) {
        auto msg = fmt::format("§aCleaned {} {} from all players", totalRemoved, itemId);
        for (auto player : Level::getAllPlayers()) {
            player->sendMessage(msg);
        }
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
            logger.info("Command executed by {} to clean {}", 
                      origin.getName(), itemId);
            
            // 直接同步执行清理操作
            cleanAllPlayers(itemId, output);
        });
}

} // namespace

// 插件入口
void PluginInit() {
    logger.setLevel(ll::LogLevel::Debug); // 设置日志级别
    logger.info("ItemCleaner plugin loaded");

    registerCommand();
}
