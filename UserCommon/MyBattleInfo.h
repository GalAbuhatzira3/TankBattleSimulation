#pragma once
#include <vector>
// Game Objects:
#include "Board.h"
#include "Block.h"
#include "Utils.h"
#include "MySatelliteView.h"
// Interfaces:
#include "../common/BattleInfo.h"

namespace UserCommon_211645361_000000000 {
    class MyBattleInfo : public BattleInfo {

    public:
        enum ShootingProtocol{ShootIfPossible, Reposition};

        // Constructor:
        MyBattleInfo(Board* board, Block* tankBlock, int initialNumShells,  SatelliteView& view);
        ~MyBattleInfo() override;

        // Get/Set:
        Board* getDemoBoard();
        Block* getTankBlock();
        void updateTankBlock(Block *block);
        SatelliteView* getView();
        [[nodiscard]] int getInitialNumShells() const;
        [[nodiscard]] int getMaxActions() const;
        void setMaxActions(int n);
        ShootingProtocol getShootingProtocol();
        void setShootingProtocol(ShootingProtocol protocol);

        // Queries:
        std::vector<Block *> getAllSymbolBlocks(char symbol);
        std::vector<Block *> getAllEnemyBlocks(int playerIndex);

    private:
        // Attributes:
        Board* demoBoard;
        Block* tankBlock;
        SatelliteView *view;
        int tankIndex = -1;
        int initialNumShells;
        int maxActions = -1;
        ShootingProtocol protocol;
    };
}
