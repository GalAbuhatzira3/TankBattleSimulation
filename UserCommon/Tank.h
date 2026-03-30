#pragma once
#include "Shell.h"

namespace UserCommon_211645361_000000000 {
    class Tank : public Entity{
    private:
        // Attributes:
        int tankID;
        int ownerID;
        Enums::Direction cannonDirection;
        std::unique_ptr<Shell> loadedShell;
        int artillery;
        int shootingCooldown;
        int backwardMoveCooldown;
        bool requestedBackward;

    public:
        // Constructors:
        explicit Tank(int tankID, int ownerID, int artillery);

        // Get/Set:
        [[nodiscard]] int getOwnerID() const;
        [[nodiscard]] int getTankID() const;
        void decArtillery();
        Enums::Direction getCannonDirection();
        std::unique_ptr<Shell> getLoadedShell();
        [[nodiscard]] Type getType() const override;
        [[nodiscard]] char getSymbol() const override;
        [[nodiscard]] int getShootingCooldown() const;
        void setShootingCooldown();
        void decShootingCooldown();
        Enums::Direction getCannonBackwardDirection();
        [[nodiscard]] int getMovingCooldown() const;
        void decMovingCooldown();
        void cancelBackwardRequest();
        void startBackwardRequest();
        void decCooldowns();
        void cancelBackwardEligibility();

        // predicates:
        [[nodiscard]] bool isOutOfAmmo() const;
        [[nodiscard]] bool canShoot() const;
        [[nodiscard]] bool hasRequestedBackward() const;
        [[nodiscard]] bool canMoveBackward() const;

        // Operations:
        void fire();
        void rotateCannon(Enums::Rotation rotation);
    };
}

