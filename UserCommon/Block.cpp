#include "Block.h"

namespace UserCommon_211645361_000000000 {
// Constructors:
    Block::Block(int x, int y) {
        this->x = x;
        this->y = y;
        this->entities[Entity::State::STATIC] = nullptr; // used for a mine/wall/tank
        this->entities[Entity::State::DYNAMIC] = nullptr; // used for shells only
    }


// Get/Set:
    int Block::getX() const {
        return this->x;
    }

    int Block::getY() const {
        return this->y;
    }

    std::pair<int, int> Block::getPosition() {
        return {this->x,this->y};
    }

    void Block::place(std::unique_ptr<Entity> entity) {
        if(entity->getType() != Entity::Type::SHELL)
            this->entities[0] = std::move(entity); // Tank, Wall or Mine
        else
            this->entities[1] = std::move(entity); // Shell
    }

    void Block::pickAndPlace(Block *from, Entity::State state) {
        this->entities[state] = std::move(from->entities[state]);
    }

    Entity* Block::getEntity(Entity::State state) {
        return entities[state].get();
    }

    Tank *Block::getTank() {
        return dynamic_cast<Tank*>(getEntity(Entity::STATIC));
    }

    void Block::destroy(Entity::State state) {
        this->entities[state] = nullptr;
    }

    std::pair<int, int> Block::getEntityTargetPosition(Entity::State state){
        Enums::Direction aim;
        switch(state){
            case Entity::STATIC: aim = dynamic_cast<Tank*>(getEntity(state))->getCannonDirection();break;
            case Entity::DYNAMIC: aim = dynamic_cast<Shell*>(getEntity(state))->getDirection(); break;
            default:
                throw std::invalid_argument("Unhandled state value");
        }
        return Utils::getTargetPosition(this->getPosition(), aim);
    }

    std::pair<int, int> Block::getTankBackwardTargetPosition(){
        Enums::Direction aim = dynamic_cast<Tank*>(getEntity(Entity::STATIC))->getCannonDirection();
        return Utils::getTargetPosition(this->getPosition(), Utils::getBackwardDirection(aim));
    }

    char Block::getSymbol() const{
        if(isShellOnMine())  // {Mine, Shell}
            return Symbols::shellOnMine;
        if(hasEntity(Entity::STATIC))  // {Tank/Mine/Wall, - }
            return this->entities[Entity::STATIC]->getSymbol();
        if(hasEntity(Entity::DYNAMIC))  // { - , Shell}
            return this->entities[Entity::DYNAMIC]->getSymbol();
        return Symbols::empty;
    }


// Predicates:
    bool Block::hasEntity(Entity::State state) const {
        return this->entities[state] != nullptr;
    }

    bool Block::isEmpty() const {
        return !hasEntity(Entity::STATIC) && !hasEntity(Entity::DYNAMIC);
    }

    bool Block::isStaticType(Entity::Type type) const {
        return this->entities[Entity::STATIC] && this->entities[Entity::STATIC]->getType() == type;
    }

    bool Block::isDirectLine(Block *target, Enums::Direction aimingDirection) const {
        if(!Utils::isOneOfDirections(this->vectorTo(target)))
            return false;
        if(Utils::deltaToDirection(this->vectorTo(target)) == aimingDirection)
            return true;
        return false;
    }

    bool Block::isShellOnMine() const {
        return hasEntity(Entity::DYNAMIC) && isStaticType(Entity::Type::MINE);
    }


// Printers:
    std::string Block::toString() const {
        return "["+std::to_string(this->x) + "]["+std::to_string(this->y)+"]";
    }


// Override:
    bool Block::operator==(const Block& other) const {
        return this->x == other.x && this->y == other.y;
    }


// Calculations:
    std::pair<int, int> Block::vectorTo(Block *other) const {
        return {other->getX() - x, other->getY() - y};
    }
}

