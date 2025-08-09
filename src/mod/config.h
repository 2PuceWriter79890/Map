#pragma once

#include <string>
#include <set>

namespace ll::mod {
    class NativeMod;
}

extern std::set<std::string> gAllowedPlayers;

void loadAndCreateDefaultConfig(ll::mod::NativeMod& self);
