#pragma once
#include "Tank.h"

namespace UserCommon_211645361_000000000 {
    class Block {
    private:
        // Attributes:
        int x;
        int y;
        std::array<std::unique_ptr<Entity>,2> entities;

    public:
        // Constructors:
        Block(int x, int y);

        // Get/Set:
        [[nodiscard]] int getX() const;
        [[nodiscard]] int getY() const;
        std::pair<int, int> getPosition();
        void place(std::unique_ptr<Entity> entity);
        void pickAndPlace(Block* from, Entity::State state);
        void destroy(Entity::State state);
        Entity* getEntity(Entity::State state);
        Tank* getTank();
        std::pair<int,int> getEntityTargetPosition(Entity::State state);
        std::pair<int,int> getTankBackwardTargetPosition();
        [[nodiscard]] char getSymbol() const;

        // Predicates:
        [[nodiscard]] bool hasEntity(Entity::State state) const;
        [[nodiscard]] bool isEmpty() const;
        [[nodiscard]] bool isStaticType(Entity::Type type) const;
        bool isDirectLine(Block*target, Enums::Direction aimingDirection) const;
        [[nodiscard]] bool isShellOnMine() const;

        // Printers:
        [[nodiscard]] std::string toString() const;

        // Overrides:
        bool operator==(const Block& other) const;

        // Calculations:
        std::pair<int, int> vectorTo(Block *other) const;
    };
}

