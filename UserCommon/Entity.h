#pragma once
#include "Utils.h"

namespace UserCommon_211645361_000000000 {
    class Entity {
    public:
        enum class Type {WALL,MINE,TANK,SHELL};
        enum State {STATIC=0, DYNAMIC=1};
        virtual ~Entity() = default;
        [[nodiscard]] virtual Type getType() const = 0;
        [[nodiscard]] virtual char getSymbol() const = 0;
    };
}

