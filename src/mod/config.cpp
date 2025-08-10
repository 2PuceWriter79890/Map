#include "mod/MyMod.h"
#include "mod/MyMod.h"

#include <ll/api/mod/NativeMod.h>
#include <ll/api/io/Logger.h>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::set<std::string> gAllowedPlayers;

void loadAndCreateDefaultConfig(ll::mod::NativeMod& self) {
    auto& logger = my_mod::MyMod::getInstance().getLogger();
    auto configPath = self.getModDir() / "config.json";
    
    logger.error("!!! DEBUG: Checking for config file at: \"{}\"", configPath.string());

    std::ifstream file(configPath);
    if (!file.is_open()) {
        logger.warn("Config file not found. Creating default config...");
        
        json defaultConfig = {
            {"playersWithKeepExp", {"Steve", "Alex"}}
        };
        std::ofstream outFile(configPath);
        if (outFile.is_open()) {
            outFile << std::setw(4) << defaultConfig << std::endl;
            logger.info("Default config file created successfully.");
        } else {
            logger.error("Failed to create config file! Please check folder permissions.");
        }
        
        gAllowedPlayers = {"Steve", "Alex"};
        return;
    }

    logger.info("Config file found, loading players...");
    json configJson;
    file >> configJson;

    if (configJson.contains("playersWithKeepExp") && configJson["playersWithKeepExp"].is_array()) {
        for (const auto& name : configJson["playersWithKeepExp"]) {
            gAllowedPlayers.insert(name.get<std::string>());
        }
    }
    logger.info("Loaded {} player(s) into the whitelist.", gAllowedPlayers.size());
}