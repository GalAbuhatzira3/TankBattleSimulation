#include "Shell.h"

namespace UserCommon_211645361_000000000 {
    // Constructors:
    Shell::Shell(int shooterID, Enums::Direction direction) {
        this->shooterID = shooterID;
        this->direction = direction;
        this->symbol = Symbols::shell;
    }

    // Get/Set:
    int Shell::getShooterID() const {
        return this->shooterID;
    }

    Enums::Direction Shell::getDirection() {
        return this->direction;
    }

    void Shell::setDirection(Enums::Direction newDirection) {
        this->direction = newDirection;
    }

    Entity::Type Shell::getType() const {
        return Entity::Type::SHELL;
    }

    char Shell::getSymbol() const {
        return this->symbol;
    }
}


