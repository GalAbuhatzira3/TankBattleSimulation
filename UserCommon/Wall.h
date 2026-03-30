#pragma once
#include "Entity.h"

namespace UserCommon_211645361_000000000 {
    class Wall : public Entity {
    private:
        // Attributes:
        int hitCount;
        char symbol;

    public:
        // Constructors:
        Wall();

        // Get/Set:
        [[nodiscard]] Type getType() const override;
        [[nodiscard]] char getSymbol() const override;
        void hit();

        // Predicates:
        [[nodiscard]] bool isDestroyed() const;
    };
}

