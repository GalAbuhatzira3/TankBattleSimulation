#include "Tank.h"

namespace UserCommon_211645361_000000000 {
// Constructors:
    Tank::Tank(int tankID, int ownerID, int artillery) {
        this->tankID = tankID;
        this->ownerID = ownerID;
        this->artillery = artillery;
        if(ownerID%2 == 1)
            this->cannonDirection = Enums::Direction::L;
        else
            this->cannonDirection = Enums::Direction::R;

        this->loadedShell = std::make_unique<Shell>(ownerID,cannonDirection);
        this->shootingCooldown = 0;
        this->backwardMoveCooldown = 0;
        this->requestedBackward = false;
    }


// Get/Set:
    Entity::Type Tank::getType() const {
        return Entity::Type::TANK;
    }

    char Tank::getSymbol() const {
        return static_cast<char>('0' + this->ownerID);
    }

    int Tank::getOwnerID() const {
        return this->ownerID;
    }

    int Tank::getTankID() const {
        return this->tankID;
    }

    void Tank::decArtillery() {
        this->artillery--;
    }

    Enums::Direction Tank::getCannonDirection() {
        return this->cannonDirection;
    }

    Enums::Direction Tank::getCannonBackwardDirection() {
        return Utils::getBackwardDirection(this->cannonDirection);
    }

    std::unique_ptr<Shell> Tank::getLoadedShell() {
        return std::move(this->loadedShell);
    }

    int Tank::getShootingCooldown() const {
        return this->shootingCooldown;
    }

    void Tank::setShootingCooldown() {
        this->shootingCooldown = Constants::shootingCooldown;
    }

    void Tank::decShootingCooldown() {
        if(this->shootingCooldown > 0)
            this->shootingCooldown--;
    }

    int Tank::getMovingCooldown() const {
        return backwardMoveCooldown;
    }

    void Tank::decMovingCooldown() {
        if(backwardMoveCooldown > 0)
            this->backwardMoveCooldown--;
    }

    void Tank::cancelBackwardRequest() {
        this->requestedBackward = false;
        this->backwardMoveCooldown = 0;
    }

    void Tank::startBackwardRequest() {
        this->requestedBackward = true; // signal that a backward request is pending
        this->backwardMoveCooldown = Constants::backwardMoveCooldown; // start the cooldown
    }

    void Tank::decCooldowns() {
        decShootingCooldown();
        decMovingCooldown();
    }


// Predicates:
    bool Tank::isOutOfAmmo() const {
        return this->artillery == 0;
    }

    bool Tank::canShoot() const {
        return !isOutOfAmmo() && this->shootingCooldown == 0;
    }

    bool Tank::canMoveBackward() const {
        return this->requestedBackward && this->backwardMoveCooldown == 0;
    }

    bool Tank::hasRequestedBackward() const {
        return this->requestedBackward;
    }


// Operations:
    void Tank::fire() {
        setShootingCooldown();
        decArtillery();
        if(!isOutOfAmmo()) // if there is enough artillery, load another shell
            this->loadedShell = std::make_unique<Shell>(this->ownerID,this->cannonDirection);
    }

    void Tank::rotateCannon(Enums::Rotation rotation) {
        Enums::Direction newDirection = Utils::rotationToDirection(rotation,this->cannonDirection);
        this->cannonDirection = newDirection;
        if(this->loadedShell != nullptr)
            this->loadedShell->setDirection(newDirection);
    }

    void Tank::cancelBackwardEligibility() {
        if(this->requestedBackward && this->backwardMoveCooldown == 0)
            this->requestedBackward = false;
    }
}







