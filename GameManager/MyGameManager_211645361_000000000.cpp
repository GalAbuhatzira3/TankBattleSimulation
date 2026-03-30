#include "MyGameManager_211645361_000000000.h"

namespace GameManager_211645361_000000000{
    REGISTER_GAME_MANAGER(MyGameManager_211645361_000000000)


    // Initialization:
    void MyGameManager_211645361_000000000::initBoard(size_t width, size_t height) {
        this->board = Board(width, height);
    }


    // Update methods:
    void MyGameManager_211645361_000000000::updateShellsMovement() {
        // for each one of the possible 32 active shells, progress it one step (at shell speed)
        auto shellsBlocks = this->board.getAllEntitiesBlocks(Entity::Type::SHELL);
        for(auto & shellsBlock : shellsBlocks)
            if(!gameOver && shellsBlock->hasEntity(Entity::DYNAMIC))
                moveShell(shellsBlock, true);
    }

    void MyGameManager_211645361_000000000::updateCounters() {
        if(gameOver)return;

        for(auto & tank : tanks){
            if(tank.wasKilled) continue;
            tank.block->getTank()->decCooldowns();
        }
    }

    void MyGameManager_211645361_000000000::updateTanksNextAction() {
        for(auto & tank : tanks){
            if(tank.wasKilled)
                continue;
            // setting the next action to be executed for this tank according to its Algorithm:
            tank.action = tank.algorithm->getAction();
        }
    }

    void MyGameManager_211645361_000000000::printStepDescription() {
        std::string msg;
        for(auto & tank : tanks){
            if(tank.isLongGone)
                msg += "killed, ";
            else{
                msg += Utils::actionToString(tank.action);
                if(tank.wasIgnored){
                    msg += " (ignored)";
                    tank.wasIgnored = false;
                }
                if(tank.wasKilled){
                    msg += " (killed)";
                    tank.isLongGone = true;
                }
                msg += ", ";
            }
        }
        msg.resize(msg.size() - 2); // trimming
        if(this->verbose) // print only if verbose = true
            Files::pushToFile(this->stepsFilePath,msg);
    }

    GameResult MyGameManager_211645361_000000000::nextStep() {
        // execute the next step in the game:
        step++;
        // first, get the matrix representation of the board as it is at the START of the step, in case of a battle info request.
        // note that changes into the game board will NOT be applied into this
        std::vector<std::vector<char>> mat = board.getBoardMatrix();  // convert the board into a matrix of chars
        // Progress shells:
        updateShellsMovement();
        if(gameOver) // check for game over scenario
            return checkGameOver();
        // Progress tanks:
        updateTanksNextAction();
        executeAllActions(&mat);
        printStepDescription(); // print this step's description
        GameResult result = checkGameOver();
        checkAllTanksOutOfAmmo();
        // Decrease all the countdowns (moving, shooting):
        updateCounters();
        return result;
    }


    // Commands:
    void MyGameManager_211645361_000000000::executeAllActions(std::vector<std::vector<char>> *mat) {
        for(size_t i = 0; i < tanks.size(); i++){
            if(tanks[i].wasKilled) continue;
            executeAction((int)i, *mat);
        }
    }

    void MyGameManager_211645361_000000000::satelliteToGameBoard(const SatelliteView &satelliteView, const std::array<TankAlgorithmFactory, Constants::numOfPlayers> &factories) {
        for (size_t y = 0; y < this->board.getHeight(); ++y){
            for (size_t x = 0; x < this->board.getWidth(); ++x){
                Block* thisBlock = this->board.at({y, x}); // this is on purpose
                char symbol = satelliteView.getObjectAt(y, x);  // this is on purpose
                if(symbol == Symbols::wall){
                    thisBlock->place(std::make_unique<Wall>());  // place a new wall object
                    logger->info("Board.action at=" + thisBlock->toString() +" placed=wall");
                }
                else if(symbol == Symbols::mine){
                    thisBlock->place(std::make_unique<Mine>());  // place a new mine object
                    logger->info("Board.action at=" + thisBlock->toString() +" placed=mine");
                }
                else if(Utils::isPlayer(symbol)){  // one of 1,2 (players available symbols)
                    int index = symbol - '0';
                    // place a new tank object
                    int sum = 0;
                    for(int remTank : remTanks)
                        sum += remTank;
                    thisBlock->place(std::make_unique<Tank>(sum,index,this->numShells));
                    logger->info("Board.action at=" + thisBlock->toString() +" placed=tank tankID="+std::to_string(sum)+" playerID="+std::to_string(index)+" ammo="+std::to_string(numShells));
                    // keeping information about this tank:
                    TankObject tankObject;
                    tankObject.block = thisBlock; // its current block
                    tankObject.algorithm = factories[index-1](index, sum); // Algorithm for this tank (player-based)
                    tanks.emplace_back(std::move(tankObject)); // storing this tank in GM's list
                    remTanks[index - 1]++; // player {index - 1} has another tank
                }
                else if(symbol != Symbols::empty) // there is an unspecified char on this block
                    logger->warn("Board.warn at=" + thisBlock->toString() +" reason=unsupported char decision=treated as space");
            }
        }
    }

    void MyGameManager_211645361_000000000::executeAction(int tankIndex, std::vector<std::vector<char>> mat) {
        Block* tankBlock = tanks[tankIndex].block;
        ActionRequest action = tanks[tankIndex].action;
        int playerID = tankBlock->getTank()->getOwnerID();
        switch(action){
            case ActionRequest::MoveForward: case ActionRequest::MoveBackward: attemptMoveTank(tankIndex); break;
            case ActionRequest::RotateLeft90:  rotateTank(tankIndex,Enums::HardLeft); break;
            case ActionRequest::RotateRight90: rotateTank(tankIndex,Enums::HardRight); break;
            case ActionRequest::RotateLeft45:  rotateTank(tankIndex,Enums::EasyLeft); break;
            case ActionRequest::RotateRight45: rotateTank(tankIndex,Enums::EasyRight); break;
            case ActionRequest::Shoot: shoot(tankIndex); break;
            case ActionRequest::DoNothing: logger->info("Player.request step="+std::to_string(step)+" player="+std::to_string(playerID)+" tank="+std::to_string(tankIndex)+" action=DoNothing status=ACCEPTED"); break;
            case ActionRequest::GetBattleInfo:{ // the tank Algorithm has requested an information about the battle
                auto [x,y] = tankBlock->getPosition();
                mat[x][y] = Symbols::thisTank; // marking the {tankIndex} tank position on the board
                players[playerID - 1]->updateTankWithBattleInfo(*tanks[tankIndex].algorithm, *std::make_unique<MySatelliteView>(mat));
                logger->info("Player.request step="+std::to_string(step)+" player="+std::to_string(playerID)+" tank="+std::to_string(tankIndex)+" action=GetBattleInfo status=ACCEPTED");
                break;
            }
            default:
                logger->critical("Unhandled action");
        }
    }


    // Movement:
    void MyGameManager_211645361_000000000::attemptMoveTank(int tankIndex) {
        if(gameOver) return;
        Block* tankBlock = tanks[tankIndex].block;
        Block* targetBlock;
        Tank* tank = tankBlock->getTank();
        ActionRequest action = tanks[tankIndex].action;
        std::string logText = "Player.request step="+std::to_string(step)+" player="+std::to_string(tank->getOwnerID())+" tank="+std::to_string(tank->getTankID())+" at="+tankBlock->toString();
        if(action == ActionRequest::MoveForward){
            targetBlock = board.at(tankBlock->getEntityTargetPosition(Entity::STATIC));
            tank->cancelBackwardRequest();
            logText += " request=forward";
            // Check if the player has asked for backward move, and is currently on a cooldown:
            if(tank->hasRequestedBackward()  && !tank->canMoveBackward()){
                logger->info(logText+" to="+targetBlock->toString()+" status=DENIED reason=pending backward (now canceled)");
                tanks[tankIndex].wasIgnored = true;
            }else
                moveTank(tankIndex, targetBlock);
        }else{
            // the player asks to move backwards.
            targetBlock = board.at(tankBlock->getTankBackwardTargetPosition());
            logText += " request=backward";
            if(!tank->hasRequestedBackward()){ // this is a NEW request, the player has to wait 2 game steps after this
                tank->startBackwardRequest();
                logger->info(logText+" to="+targetBlock->toString()+" status=ACCEPTED note=executing in "+std::to_string(tank->getMovingCooldown()));
                tanks[tankIndex].wasIgnored = true;
            }// a backward move request is pending. Check the cooldown:
            else if(tank->canMoveBackward())// the player HAS already requested for a backward move, is the cooldown over?
                moveTank(tankIndex, targetBlock);
             // Can't move, still on a cooldown. Ignore this request
            else{
                logger->info(logText+" to="+targetBlock->toString()+" status=DENIED reason=cooldown ("+std::to_string(tank->getMovingCooldown())+")");
                tanks[tankIndex].wasIgnored = true;
            }
        }
    }

    void MyGameManager_211645361_000000000::moveTank(int tankIndex, Block *targetBlock) {
        Block* tankBlock = tanks[tankIndex].block;
        Tank* tank = tankBlock->getTank();
        std::string logText = "Player.request step="+std::to_string(step)+" player="+std::to_string(tank->getOwnerID())+" tank="+std::to_string(tank->getTankID())+" at="+tankBlock->toString();
        Enums::Direction direction = board.directionToNeighbor(tankBlock,targetBlock);
        if(targetBlock->isStaticType(Entity::Type::WALL)){
            // Illegal segment: a tank can't run into a wall
            logger->illegal(logText+" to="+targetBlock->toString()+" status=DENIED reason=wall");
            tanks[tankIndex].wasIgnored = true;
            return;
        }
        // Legal segment: Moving the player to the desired block
        logger->info(logText+" to="+targetBlock->toString()+" status=ACCEPTED");
        if(targetBlock->hasEntity(Entity::DYNAMIC)){ // The block has a shell on it
            logger->key("Player.killed step="+std::to_string(this->step)+" player="+std::to_string(tank->getOwnerID())+" tank="+std::to_string(tank->getTankID())+" at="+tankBlock->toString()+" reason=shell");
            killTank(tankBlock);
            targetBlock->destroy(Entity::DYNAMIC);
        }else if(targetBlock->isStaticType(Entity::Type::MINE)){ // The block has a mine on it
            logger->key("Player.killed step="+std::to_string(this->step)+" player="+std::to_string(tank->getOwnerID())+" tank="+std::to_string(tank->getTankID())+" at="+tankBlock->toString()+" reason=mine");
            killTank(tankBlock);
            targetBlock->destroy(Entity::STATIC);
        }else if(targetBlock->isStaticType(Entity::Type::TANK)){ // The block has a tank on it
            int rivalID = targetBlock->getTank()->getOwnerID();
            logger->key("Player.killed step="+std::to_string(this->step)+" player="+std::to_string(tank->getOwnerID())+" tank="+std::to_string(tank->getTankID())+" at="+tankBlock->toString()+" reason=tank (collision)");
            logger->key("Player.killed step="+std::to_string(this->step)+" player="+std::to_string(targetBlock->getTank()->getOwnerID())+" tank="+std::to_string(targetBlock->getTank()->getTankID())+" at="+targetBlock->toString()+" reason=tank (collision)");
            killTank(tankBlock);
            killTank(targetBlock);
        }else{
            targetBlock->pickAndPlace(tankBlock,Entity::STATIC); // placing the player at his new position
            tanks[tankIndex].block = targetBlock;
        }
    }

    void MyGameManager_211645361_000000000::shoot(int tankIndex) {
        if(gameOver) return;
        Block* tankBlock = tanks[tankIndex].block;
        Tank *tank = tankBlock->getTank();
        tank->cancelBackwardEligibility();
        std::string logText = "Player.request step="+std::to_string(step)+" player="+std::to_string(tank->getOwnerID())+" tank="+std::to_string(tank->getTankID())+" at="+tankBlock->toString()+" request=shoot";
        if(tank->hasRequestedBackward() && !tank->canMoveBackward()){
            logger->info(logText+" status=DENIED reason=pending backward");
            tanks[tankIndex].wasIgnored = true;
        }else if(tank->isOutOfAmmo()){
            logger->illegal(logText+" status=DENIED reason=no ammo");
            tanks[tankIndex].wasIgnored = true;
        }else if(!tank->canShoot()){
            logger->illegal(logText+" status=DENIED reason=cooldown ("+std::to_string(tank->getShootingCooldown())+")");
            tanks[tankIndex].wasIgnored = true;
        }else{
            logger->info(logText+" status=ACCEPTED");
            // Handling the placing of the shell
            tankBlock->place(tank->getLoadedShell()); // loading the shell into tank's block
            moveShell(tankBlock, false); // placing the shell at the target block
            tank->fire(); // the tank fired the loaded shell
            return;
        }
    }

    void MyGameManager_211645361_000000000::moveShell(Block *shellBlock, bool hasFired) {
        std::string gameOverMsg;
        std::string logText;
        int iter = 1;
        if(hasFired)
            iter = Constants::shellSpeed;
        for(int _ = 0; _ < iter; _++){
            auto *shell = dynamic_cast<Shell*>(shellBlock->getEntity(Entity::DYNAMIC));
            Block *targetBlock = board.at(shellBlock->getEntityTargetPosition(Entity::DYNAMIC));
            if(targetBlock->isStaticType(Entity::Type::WALL)){ // The shell will hit a wall
                shellBlock->destroy(Entity::DYNAMIC); // the shell exploded
                // Checking if the wall collapsed:
                Wall* wall = dynamic_cast<Wall*>(targetBlock->getEntity(Entity::STATIC));
                wall->hit();
                logText = "Board.action step="+std::to_string(step)+" object=wall at="+targetBlock->toString();
                if(wall->isDestroyed()){
                    logger->info(logText+" condition=collapsed");
                    targetBlock->destroy(Entity::STATIC);
                }else
                    logger->info(logText+" condition=endured");
                break;
            }else if(targetBlock->hasEntity(Entity::DYNAMIC)){ // the shell will hit another shell
                shellBlock->destroy(Entity::DYNAMIC); // the shell exploded
                targetBlock->destroy(Entity::DYNAMIC); // The other shell was destroyed as well
                break;
            }else if(targetBlock->isStaticType(Entity::Type::TANK)){ // the shell will hit a tank
                Tank* tank = dynamic_cast<Tank*>(targetBlock->getEntity(Entity::STATIC));
                logger->key("Player.killed step="+std::to_string(this->step)+" player="+std::to_string(tank->getOwnerID())+" tank="+std::to_string(tank->getTankID())+" at="+targetBlock->toString()+" reason=shell");
                shellBlock->destroy(Entity::DYNAMIC); // the shell exploded
                killTank(targetBlock); // the tank was destroyed
                break;
            }else{// The shell proceed without hitting anything
                targetBlock->pickAndPlace(shellBlock,Entity::DYNAMIC);
                shellBlock = targetBlock;
            }
        }
    }

    void MyGameManager_211645361_000000000::rotateTank(int tankIndex, Enums::Rotation rotation) {
        if(gameOver) return;
        Tank* tank = tanks[tankIndex].block->getTank();
        tank->cancelBackwardEligibility();
        std::string logText = "Player.request step="+std::to_string(step)+" player="+std::to_string(tank->getOwnerID())+" tank="+std::to_string(tank->getTankID())+" at="+tanks[tankIndex].block->toString()+" request=rotate ("+Utils::rotationToString(rotation)+")";
        if(tank->hasRequestedBackward() && !tank->canMoveBackward()){
            logger->info(logText + " status=DENIED reason=pending backward");
            tanks[tankIndex].wasIgnored = true;
        }else{
            logText += " from="+Utils::directionToString(tank->getCannonDirection());
            tank->rotateCannon(rotation);
            logText +=" to="+Utils::directionToString(tank->getCannonDirection());
            logger->info(logText + " status=ACCEPTED");
        }
    }


    // Get/Set:
    void MyGameManager_211645361_000000000::killTank(Block *tankBlock) {
        int index = tankBlock->getTank()->getTankID();
        int playerID = tankBlock->getTank()->getOwnerID();
        remTanks[playerID - 1] --;
        tanks[index].block = nullptr;
        tanks[index].algorithm = nullptr;
        tanks[index].wasKilled = true;
        tankBlock->destroy(Entity::STATIC);
    }


    // Game Over:
    GameResult MyGameManager_211645361_000000000::maxStepsTie() {
        declareGameOver();
        std::string msg = "Tie, reached max steps = " + std::to_string(maxSteps) +"\n";
        for(int i = 0; i < Constants::numOfPlayers; ++i)
            msg += "player " + std::to_string(i+1) + " has " + std::to_string(remTanks[i]) + " tanks\n";
        if(this->verbose)
            Files::pushToFile(this->stepsFilePath, msg);
        return GameResult{0, GameResult::MAX_STEPS};
    }

    void MyGameManager_211645361_000000000::checkAllTanksOutOfAmmo() {
        if(gameOver) return;
        for(auto & tank : tanks)
            if(tank.wasKilled || !tank.block->getTank()->isOutOfAmmo())
                return;
        // all tanks are out of ammo, the game will continue for 40 more steps.
        this->maxSteps = std::min(this->maxSteps, this->step + Constants::outOfAmmoAdditionalSteps);
        this->allTanksOutOfAmmo = true;
    }

    GameResult MyGameManager_211645361_000000000::noAmmoLimitReached() {
        declareGameOver();
        std::string msg = "Tie, both players have zero shells for " + std::to_string(Constants::outOfAmmoAdditionalSteps) + " steps";
        if(this->verbose)
            Files::pushToFile(this->stepsFilePath, msg);
        return GameResult{0, GameResult::ZERO_SHELLS};
    }

    bool MyGameManager_211645361_000000000::checkNoTanksTie() {
        bool noTanks = true;
        for(int remTank : this->remTanks)
            noTanks = noTanks && (remTank == 0);
        return noTanks;
    }

    GameResult MyGameManager_211645361_000000000::noTanksTie() {
        declareGameOver();
        if(this->verbose)
            Files::pushToFile(this->stepsFilePath, "Tie, all players have zero tanks");
        this->logger->debug("Match.end step="+std::to_string(this->step)+" result=tie reason=no tanks");
        return GameResult{0, GameResult::ALL_TANKS_DEAD};
    }

    int MyGameManager_211645361_000000000::checkVictory() {
        int winnerID = -1;
        for(int i = 0; i < Constants::numOfPlayers; ++i)
            if(remTanks[i] > 0){ // player i has at least 1 tank left
                if(winnerID == -1)  // he is the FIRST from 0,...,i-1 to have so
                    winnerID = i; // make him as the possible winner
                else // he is NOT the first, meaning there is no winner yet
                    return -1; // we can return
            }
        return winnerID;
    }

    GameResult MyGameManager_211645361_000000000::victory(int winnerID, int numTanksAlive) {
        declareGameOver();
        if(this->verbose)
            Files::pushToFile(this->stepsFilePath,"Player " + std::to_string(winnerID + 1) + " won with " + std::to_string(numTanksAlive) + " tanks still alive");
        this->logger->debug("Match.end step="+std::to_string(this->step)+" result=win winner="+std::to_string(winnerID + 1)+"");
        return GameResult{winnerID + 1};
    }

    GameResult MyGameManager_211645361_000000000::checkGameOver() {
        // check if there is a winner:
        int winnerID = checkVictory();
        if(winnerID != -1)
            return victory(winnerID, remTanks[winnerID]);
        // check if there is at least one player who still has tanks:
        if(checkNoTanksTie())
            return noTanksTie();
        // check if the maximum number of steps has been reached
        if(step == maxSteps){
            if(!allTanksOutOfAmmo)
                return maxStepsTie();
            else
                return noAmmoLimitReached();
        }
        return GameResult{}; // the game isn't over
    }

    void MyGameManager_211645361_000000000::declareGameOver() {
        this->gameOver = true;
    }


    // Constructor:
    MyGameManager_211645361_000000000::MyGameManager_211645361_000000000(bool verbose) {
        this->verbose = verbose;
    }


    // Override:
    GameResult MyGameManager_211645361_000000000::run(size_t map_width, size_t map_height, const SatelliteView &map,
                                                      std::string map_name, size_t max_steps, size_t num_shells,
                                                      Player &player1, std::string name1, Player &player2,
                                                      std::string name2, TankAlgorithmFactory player1_tank_algo_factory,
                                                      TankAlgorithmFactory player2_tank_algo_factory) {
        this->uniqueNumber = Utils::getUniqueTime(); // a unique time stamp for this game
        this->level = this->cfg.gmLogLevel.value_or(LogLevel::UNKNOWN);
        std::string logPrefix = this->cfg.gmLogFilePrefix.value_or(Constants::gameManagerLoggerPrefix);
        this->logger = std::make_unique<Logger>(logPrefix+this->uniqueNumber+".log",
                                                this->cfg.gmLogPrintToConsole.value_or(false),
                                                this->cfg.gmLogIncludeTID.value_or(true),
                                                this->level);
        this->players = { &player1, &player2 };// store the players:
        this->maxSteps = max_steps; // set the max steps
        this->numShells = num_shells; // set the num of shells
        this->stepsFilePath = "output_" + name1 + "_vs_" + name2 + "_game_" + this->uniqueNumber + ".txt";
        this->logger->debug("Match.start: map="+map_name+" player1="+name1+" player2="+name2);
        initBoard(map_width, map_height); // create the plain board, based on the given dimensions
        // pack the factories into an array:
        std::array<TankAlgorithmFactory,Constants::numOfPlayers> factories = {player1_tank_algo_factory, player2_tank_algo_factory };
        satelliteToGameBoard(map, factories); // populate the game board using the SatelliteView object and the factories
        // the main loop of the game:
        GameResult result = checkGameOver(); // there is a chance the game has already ended
        while(!gameOver && step < this->maxSteps)
            result = nextStep();
        for(int remTank : this->remTanks) // the game has ended, complete GameResult info (only the last one counts)
            result.remaining_tanks.emplace_back((size_t)remTank);
        result.gameState = std::make_unique<MySatelliteView>(board.getBoardMatrix());
        result.rounds = this->step; // could be 0 (the game ended before starting)
        return result;
    }
}
