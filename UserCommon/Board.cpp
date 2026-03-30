#include "Board.h"

namespace UserCommon_211645361_000000000 {
// Constructors:
    Board::Board(size_t width, size_t height){
        this->width = width;
        this->height = height;
        board.resize(height);
        for (size_t y = 0; y < height; ++y) {
            board[y].reserve(width);
            for (size_t x = 0; x < width; ++x)
                board[y].emplace_back(y,x);
        }
    }


// Get/Set:
    size_t Board::getWidth() const {
        return this->width;
    }

    size_t Board::getHeight() const {
        return this->height;
    }

    char Board::getSymbol(std::pair<int, int> position) const {
        int x = position.first, y = position.second;
        return board[x][y].getSymbol();
    }

    Block *Board::at(std::pair<int, int> position){
        return &board[Utils::mod(position.first, height)][Utils::mod(position.second, width)];
    }

    Block *Board::getNeighbor(Block *source, Enums::Direction direction) {
        auto [x,y] = source->getPosition();
        auto [offsetX, offsetY] = Utils::directionToVector(direction);
        return at({x+offsetX,y+offsetY});
    }

    std::vector<Block *> Board::getAllEntitiesBlocks(Entity::Type type) {
        std::vector<Block*> blocks = {};
        for (size_t y = 0; y < this->height; ++y){
            for (size_t x = 0; x < this->width; ++x){
                Block* block = this->at({y, x}); // this is on purpose
                if(type == Entity::Type::SHELL && block->hasEntity(Entity::DYNAMIC))
                    blocks.emplace_back(block);
                if(type != Entity::Type::SHELL && block->isStaticType(type))
                    blocks.emplace_back(block);
            }
        }
        return blocks;
    }


// Printers:
    std::string Board::getBoardText() {
        std::string boardString;
        for(size_t i = 0; i < height; i++){
            for(size_t j = 0; j < width; j++)
                boardString += at({i,j})->getSymbol();
            boardString += "\n";
        }
        return boardString;
    }

    std::vector<std::vector<char>> Board::getBoardMatrix() {
        std::vector<std::vector<char>> mat(height, std::vector<char>(width));
        for (size_t r = 0; r < height; ++r)
            for (size_t c = 0; c < width; ++c)
                mat[r][c] = board[r][c].getSymbol();
        return mat;
    }


// Calculations:
    Enums::Direction Board::directionToNeighbor(Block *source, Block *neighbor){
        for(int i = 0; i < 8; i++){
            auto direction = static_cast<Enums::Direction>(i);
            std::pair<int,int> offset = Utils::directionToVector(direction);
            if(this->at({source->getX()+offset.first,source->getY()+offset.second}) == neighbor)
                return direction;
        }
        throw std::invalid_argument("neighbor block isn't a neighbor of this block");
    }
}


















