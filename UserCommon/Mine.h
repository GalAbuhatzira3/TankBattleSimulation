#pragma once
#include "Entity.h"

namespace UserCommon_211645361_000000000 {
    class Mine : public Entity{
    public:
        // Constructors:
        Mine() = default;

        // Get/Set:
        [[nodiscard]] Type getType() const override;
        [[nodiscard]] char getSymbol() const override;
    };
}

