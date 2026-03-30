#pragma once
#include "Entity.h"

namespace UserCommon_211645361_000000000 {
    class Shell : public Entity {
    private:
        // Attributes:
        int shooterID;
        Enums::Direction direction;
        char symbol;

    public:
        // Constructors:
        Shell(int shooterID, Enums::Direction direction);

        // Get/Set:
        [[nodiscard]] int getShooterID() const;
        Enums::Direction getDirection();
        void setDirection(Enums::Direction direction);
        [[nodiscard]] Type getType() const override;
        [[nodiscard]] char getSymbol() const override;
    };
}

