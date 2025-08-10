#pragma once

#include "ll/api/mod/NativeMod.h"
#include "ll/api/event/Listener.h"
#include "ll/api/io/Logger.h"

#include <map>
#include <string>
#include <memory>
#include <utility>

namespace my_mod {

class MyMod {
public:
    static MyMod& getInstance();

    MyMod(const MyMod&) = delete;
    MyMod(MyMod&&)      = delete;
    MyMod& operator=(const MyMod&) = delete;
    MyMod& operator=(MyMod&&) = delete;

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }
    [[nodiscard]] ll::io::Logger& getLogger() const { return mLogger; }

    bool load();
    bool enable();
    bool disable();

private:
    MyMod() : mSelf(*ll::mod::NativeMod::current()), mLogger(mSelf.getLogger()) {} 

    ll::mod::NativeMod& mSelf;
    ll::io::Logger&     mLogger;

    ll::event::ListenerPtr mPlayerDieListener;
    ll::event::ListenerPtr mPlayerRespawnListener;

    std::map<std::string, std::pair<int, float>> mStoredExperience;
};

}