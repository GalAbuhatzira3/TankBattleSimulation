#include "MyPlayer_211645361_000000000.h"

namespace Algorithm_211645361_000000000 {
    REGISTER_PLAYER(MyPlayer_211645361_000000000)

    // Get/Set:
    Block *MyPlayer_211645361_000000000::getCallerTankBlock(SatelliteView &view) {
        // getting the block the caller tank is currently on (identifying the '%' symbol)
        for (size_t i = 0; i < this->demoBoard.getHeight(); ++i)
            for (size_t j = 0; j < this->demoBoard.getWidth(); ++j)
                if(view.getObjectAt(i,j) == Symbols::thisTank)
                    return demoBoard.at({i,j}); // the tank has been located at (i,j)
        throw std::invalid_argument("game manager didn't mark tank's position");
    }

    MyPlayer_211645361_000000000::Report MyPlayer_211645361_000000000::getBattleReport(MyBattleInfo &myInfo) {
        // Use the recently received battle info to compare our data with the real data.
        char mySymbol = Utils::getPlayerSymbol(this->playerIndex);
        int ourRecentNumOfTanks = (int)myInfo.getAllSymbolBlocks(mySymbol).size(); // getting the number of tanks we currently have
        int enemyRecentNumOfTanks = (int)myInfo.getAllEnemyBlocks(this->playerIndex).size(); // getting the number of thanks our enemies currently have
        if(ourPrevNumOfTanks == -1 && enemyPrevNumOfTanks == -1){ // check if this is the first getBattleInfo request.
            this->ourPrevNumOfTanks = ourRecentNumOfTanks;
            this->enemyPrevNumOfTanks = enemyRecentNumOfTanks;
            return Report::Stable;
        }
        int ourLoss = this->ourPrevNumOfTanks - ourRecentNumOfTanks; // the number of tanks we lost since the last getBattleInfo request
        int enemyLoss = this->enemyPrevNumOfTanks - enemyRecentNumOfTanks; // the number of tanks the enemies lost since the last getBattleInfo request
        this->ourPrevNumOfTanks = ourRecentNumOfTanks; // updating
        this->enemyPrevNumOfTanks = enemyRecentNumOfTanks; // updating
        if(ourLoss == enemyLoss) // we've lost as many tanks as our enemies, the situation is stable.
            return Report::Stable;
        else if(ourLoss < enemyLoss) // we've lost fewer tanks than our enemies - we are winning the battle!
            return Report::Winning;
        else  // we've lost fewer tanks than our enemies - we are winning the battle!
            return Report::Losing;
    }

    void MyPlayer_211645361_000000000::updateStrategy(BattleInfo &info) {
        /* Update the max number of actions our Algorithm will fill, based on our collected data.
        * If we're winning, increase it, since we can afford it.
        * If losing, we need more precision, so decrease it.
        * If stable, don't change a thing.
        * Don't change the shooting protocol (set to default as "shoot if possible")
        */
        auto& myInfo = dynamic_cast<MyBattleInfo&>(info);
        Report report = getBattleReport(myInfo);
        switch (report) {
            case Losing: if(this->maxActions - 1 >= Constants::minActionsSlide) this->maxActions -= 1; break;
            case Winning: if(this->maxActions + 1 <= Constants::maxActionsSlide) this->maxActions += 1; break;
            case Stable: break;
        }
        myInfo.setMaxActions(this->maxActions);
    }


// Constructor:
    MyPlayer_211645361_000000000::MyPlayer_211645361_000000000(int playerIndex, size_t x, size_t y, size_t maxSteps, size_t numShells) {
        this->demoBoard = Board(x,y); // creating the demo board
        this->numShells = numShells;
        this->maxSteps = maxSteps;
        this->playerIndex = playerIndex;
        this->maxActions = (int)((Constants::maxActionsSlide + Constants::minActionsSlide)/2); // getting the middle value. in that case - 8.
    }

    void MyPlayer_211645361_000000000::updateTankWithBattleInfo(TankAlgorithm &tank, SatelliteView &view) {
        /* We got here since the Algorithm of one of our tanks had requested information about the battle.
         * GM gave us the Algorithm and the satelliteView of the game board as in the previous round.
         * Now we need to provide a MyBattleInfo object to the tank Algorithm.
         * The Algorithm is FAMILIAR with MyBattleInfo.
         * we will send our maintained demo board, the SatelliteView object,
         * and the block which the caller tank is currently on, according to the SatelliteView object:
         */
        Block* tankCurrBlock = getCallerTankBlock(view);
        std::unique_ptr<BattleInfo> info = std::make_unique<MyBattleInfo>(&this->demoBoard,tankCurrBlock,this->numShells,view);
        this->updateStrategy(*info); // updating the max number of action based on our data and the BattleInfo
        tank.updateBattleInfo(*info);
        /* we sent our ACTUAL demo board, and the Algorithm is updating it.
         * meaning that now (after the function has returned), the demo board has been updated
         * to have the tank final block, rotation, number of remaining shells and cooldowns.
         */
    }
}
