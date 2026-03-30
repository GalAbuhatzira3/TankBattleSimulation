#include "MyBattleInfo.h"

namespace UserCommon_211645361_000000000 {
// Constructor:
    MyBattleInfo::MyBattleInfo(Board* board, Block* tankBlock, int initialNumShells, SatelliteView& view) {
        this->demoBoard = board;
        this->tankBlock = tankBlock;
        this->view = &view;
        this->initialNumShells = initialNumShells;
        this->protocol = ShootingProtocol::ShootIfPossible;
    }

    MyBattleInfo::~MyBattleInfo() = default;


// Get/Set:
    int MyBattleInfo::getInitialNumShells() const{
        return this->initialNumShells;
    }

    Board* MyBattleInfo::getDemoBoard() {
        return this->demoBoard;
    }

    Block* MyBattleInfo::getTankBlock() {
        return this->tankBlock;
    }

    void MyBattleInfo::updateTankBlock(Block* block){
        block->pickAndPlace(this->tankBlock,Entity::STATIC);
        this->tankBlock = block;
    }

    SatelliteView *MyBattleInfo::getView() {
        return this->view;
    }

    int MyBattleInfo::getMaxActions() const {
        return this->maxActions;
    }

    void MyBattleInfo::setMaxActions(int n) {
        this->maxActions = n;
    }

    MyBattleInfo::ShootingProtocol MyBattleInfo::getShootingProtocol() {
        return this->protocol;
    }

    void MyBattleInfo::setShootingProtocol(MyBattleInfo::ShootingProtocol shootingProtocol) {
        this->protocol = shootingProtocol;
    }


// Queries:
    std::vector<Block*> MyBattleInfo::getAllSymbolBlocks(char symbol){
        std::vector<Block*> blocks;
        for (size_t i = 0; i < this->demoBoard->getHeight(); ++i)
            for (size_t j = 0; j < this->demoBoard->getWidth(); ++j)
                if(view->getObjectAt(i,j) == symbol)
                    blocks.emplace_back(demoBoard->at({i,j}));
        return blocks;
    }

    std::vector<Block*> MyBattleInfo::getAllEnemyBlocks(int playerIndex){
        std::vector<Block*> blocks;
        char playerSymbol = Utils::getPlayerSymbol(playerIndex);
        char blockSymbol;
        for (size_t i = 0; i < this->demoBoard->getHeight(); ++i){
            for (size_t j = 0; j < this->demoBoard->getWidth(); ++j){
                blockSymbol = view->getObjectAt(i,j);
                // check if it's one of <playerIndex> enemies
                if(Utils::isPlayer(blockSymbol) && blockSymbol != playerSymbol)
                    blocks.emplace_back(demoBoard->at({i,j}));
            }
        }
        return blocks;
    }
}





