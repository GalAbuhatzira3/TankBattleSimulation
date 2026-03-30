#pragma once
#include <memory>
// Implementations:
#include "../UserCommon/MyBattleInfo.h"
// Game Objects:
#include "../UserCommon/Board.h"
// Interfaces:
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"
#include "../common/SatelliteView.h"
#include "../common/PlayerRegistration.h"
// Namespaces:
using namespace UserCommon_211645361_000000000;

namespace Algorithm_211645361_000000000 {
    class MyPlayer_211645361_000000000 : public Player {
    private:
        // each player has a VIRTUAL(!) board where he will keep track of his tank objects.
        // this is NOT(!) the board GM uses nor a copy of it.
        enum Report{Losing, Stable, Winning};

        // Attributes:
        Board demoBoard;
        size_t numShells;
        size_t maxSteps;
        int playerIndex;
        int maxActions = 0;
        int enemyPrevNumOfTanks = -1;
        int ourPrevNumOfTanks = -1;

        // Get/Set:
        Block *getCallerTankBlock(SatelliteView &view);
        Report getBattleReport(MyBattleInfo& myInfo);
        void updateStrategy(BattleInfo& info);

    public:
        // Constructor:
        MyPlayer_211645361_000000000(int playerIndex, size_t x, size_t y, size_t maxSteps, size_t numShells);

        // Override:
        void updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& view) override;
    };

}
