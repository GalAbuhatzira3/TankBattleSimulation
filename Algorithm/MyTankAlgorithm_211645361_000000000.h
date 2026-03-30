#pragma once
#include <memory>
#include <queue>
// Implementations:
#include "../UserCommon/MySatelliteView.h"
#include "../UserCommon/MyBattleInfo.h"
// Game Objects:
#include "../UserCommon/Node.h"
// Interfaces:
#include "../common/TankAlgorithm.h"
#include "../common/ActionRequest.h"
#include "../common/TankAlgorithmRegistration.h"
// Namespaces:
using namespace UserCommon_211645361_000000000;

namespace Algorithm_211645361_000000000 {
    class MyTankAlgorithm_211645361_000000000 : public TankAlgorithm {
    private:
        // Attributes:
        int playerIndex;
        int tankIndex;
        int queueMaxSize{};
        std::queue<ActionRequest> actionsQueue;  // the action queue for the tank, based on this Algorithm

        // Helping methods:
        bool isInDanger(MyBattleInfo *myInfo, Block* block);
        bool willLikelyHitMyself(MyBattleInfo *myInfo);
        bool tryToEvacuate(MyBattleInfo *myInfo);
        void setToShoot(MyBattleInfo *myInfo);
        bool isFriendlyFire(MyBattleInfo *myInfo, Block *shooterBlock, Enums::Direction aim) const;
        bool isAimingAtEnemyOnly(MyBattleInfo *myInfo, Block* shooterBlock, Enums::Direction aim) const;
        void placeDemoTank(MyBattleInfo *myInfo);
        void moveToNeighbor(MyBattleInfo *myInfo, Block* nextBlock, Enums::Direction targetDir, bool takeClearShot);
        void moveToShoot(MyBattleInfo *myInfo);
        bool rotateToShoot(MyBattleInfo *myInfo, Block* shooterBlock);
        std::vector<std::vector<Node>> getGraphFromBoard(Board *board) const;
        std::stack<Block*> BFS(MyBattleInfo *myInfo, Block* target) const;
        std::stack<Block*> getPathToClosestEnemy(MyBattleInfo *myInfo);
        void refillActionQueue(MyBattleInfo *myInfo);

        // Commands:
        bool moveForward(Tank* tank);
        bool moveBackward(Tank* tank);
        void shoot(Tank* tank);
        bool rotate(MyBattleInfo *myInfo, Enums::Direction targetDir, bool takeClearShot);
        void doNothing(Tank* tank);
        void getBattleInfo(Tank* tank);

        // Predicates:
        bool canAddAction();
        bool canAddBundleActions(size_t n);

    public:
        // Constructor:
        MyTankAlgorithm_211645361_000000000(int playerIndex, int tankIndex);

        // Override:
        ActionRequest getAction() override;
        void updateBattleInfo(BattleInfo& info) override;
    };

}
