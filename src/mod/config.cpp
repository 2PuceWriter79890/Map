#include "mod/config.h"

#include <ll/api/mod/NativeMod.h>
#include <fstream>
#include <iomanip>
#include "mod/json.hpp"

using json = nlohmann::json;

std::set<std::string> gAllowedPlayers;

void loadAndCreateDefaultConfig(ll::mod::NativeMod& self) {
    auto configPath = self.getModDir() / "config.json";
    
    std::ifstream file(configPath);
    if (!file.is_open()) {
        json defaultConfig = {
            {"playersWithKeepExp", {"Steve", "Alex"}}
        };
        std::ofstream outFile(configPath);
        outFile << std::setw(4) << defaultConfig << std::endl;
        
        gAllowedPlayers = {"Steve", "Alex"};
        return;
    }

    json configJson;
    file >> configJson;

    if (configJson.contains("playersWithKeepExp") && configJson["playersWithKeepExp"].is_array()) {
        for (const auto& name : configJson["playersWithKeepExp"]) {
            gAllowedPlayers.insert(name.get<std::string>());
        }
    }
}
