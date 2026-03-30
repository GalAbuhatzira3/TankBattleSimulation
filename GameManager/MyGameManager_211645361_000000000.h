#pragma once
// Implementations:
#include "../UserCommon/MySatelliteView.h"
// Game Objects:
#include "../UserCommon/Wall.h"
#include "../UserCommon/Mine.h"
#include "../UserCommon/Node.h"
#include "../UserCommon/Config.h"
// Interfaces:
#include "../common/BattleInfo.h"
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"
#include "../common/AbstractGameManager.h"
#include "../common/GameManagerRegistration.h"
// Namespaces:
using namespace UserCommon_211645361_000000000;

namespace GameManager_211645361_000000000{
    class MyGameManager_211645361_000000000 : public AbstractGameManager{
    private:
        // Structures:
        struct TankObject{
            Block* block{}; // the block which the tank is currently on
            std::unique_ptr<TankAlgorithm> algorithm; // the Algorithm for this tank
            ActionRequest action = ActionRequest::DoNothing; // the next action for this tank
            // attributes for UI:
            bool wasIgnored = false; // (the last command of this tank was ignored)
            int wasKilled = false;  // (the tank was killed in the last step)
            bool isLongGone = false; // (the tank was killed before the last step)
        };

        // Attributes:
        Board board;
        size_t maxSteps = 0;
        size_t numShells = 0;
        bool verbose = false;
        std::array<Player*,Constants::numOfPlayers> players{};
        int remTanks[Constants::numOfPlayers]{}; // remaining tanks for each player
        std::vector<TankObject> tanks; // list of data about each tank
        std::string stepsFilePath;
        size_t step = 0;
        bool gameOver = false;
        bool allTanksOutOfAmmo = false;
        std::string uniqueNumber;
        std::unique_ptr<Logger> logger; // logger
        LogLevel level; // logger level
        const MiniCfg cfg = loadCfg(Constants::ConfigFilePath); // configuration file (Maybe shows a compilation error, but works fine)


        // Initialization:
        void initBoard(size_t width, size_t height);

        // Update methods:
        void updateShellsMovement();
        void updateCounters();
        void updateTanksNextAction();
        void printStepDescription();
        GameResult nextStep();

        // Commands:
        void executeAllActions(std::vector<std::vector<char>> *mat);
        void satelliteToGameBoard(const SatelliteView& satelliteView, const std::array<TankAlgorithmFactory,Constants::numOfPlayers>& factories);
        void executeAction(int tankIndex, std::vector<std::vector<char>> mat);

        // Movement:
        void attemptMoveTank(int tankIndex);
        void moveTank(int tankIndex, Block* targetBlock);
        void shoot(int tankIndex);
        void moveShell(Block* shellBlock, bool hasFired);
        void rotateTank(int tankIndex, Enums::Rotation rotation);

        // Get/Set:
        void killTank(Block* tankBlock);

        // Game Over:
        GameResult maxStepsTie();
        void checkAllTanksOutOfAmmo();
        GameResult noAmmoLimitReached();
        bool checkNoTanksTie();
        GameResult noTanksTie();
        int checkVictory();
        GameResult victory(int winnerID, int numTanksAlive);
        GameResult checkGameOver();
        void declareGameOver();


    public:
        // Constructor:
        MyGameManager_211645361_000000000(bool verbose);

        // Override:
        GameResult run(
                size_t map_width,
                size_t map_height,
                const SatelliteView& map, std::string map_name,
                size_t max_steps, size_t num_shells,
                Player& player1, std::string name1,
                Player& player2, std::string name2,
                TankAlgorithmFactory player1_tank_algo_factory, TankAlgorithmFactory player2_tank_algo_factory
        ) override;
    };
}

