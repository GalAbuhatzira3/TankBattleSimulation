#pragma once
#include "Block.h"

namespace UserCommon_211645361_000000000 {
    class Board {
    private:
        // Attributes:
        std::vector<std::vector<Block>> board;
        size_t width = 0;
        size_t height = 0;

    public:
        // Constructors:
        Board(size_t width, size_t height);
        Board() = default;

        // Get/Set:
        [[nodiscard]] size_t getWidth() const;
        [[nodiscard]] size_t getHeight() const;
        [[nodiscard]] char getSymbol(std::pair<int, int> position) const;
        Block* at(std::pair<int, int> position);
        Block* getNeighbor(Block* source, Enums::Direction direction);
        std::vector<Block*> getAllEntitiesBlocks(Entity::Type type);

        // Printers:
        std::string getBoardText();
        std::vector<std::vector<char>> getBoardMatrix();

        // Calculations:
        Enums::Direction directionToNeighbor(Block *source, Block *neighbor);
    };
}

