#include "Mine.h"

namespace UserCommon_211645361_000000000 {
// Get/Set:
    Entity::Type Mine::getType() const {
        return Entity::Type::MINE;
    }

    char Mine::getSymbol() const {
        return Symbols::mine;
    }
}

