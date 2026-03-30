#include "Wall.h"

namespace UserCommon_211645361_000000000 {
// Constructors:
    Wall::Wall() {
        this->hitCount = 0;
        this->symbol = Symbols::wall; // this symbol can change if the wall is cracked
    }

// Get/Set:
    Entity::Type Wall::getType() const {
        return Entity::Type::WALL;
    }

    void Wall::hit() {
        this->hitCount++;
    }

    char Wall::getSymbol() const {
        return this->symbol;
    }


// Predicates:
    bool Wall::isDestroyed() const {
        return this->hitCount >= Constants::hitsToBreakWall;
    }
}



