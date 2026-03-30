#include "MyTankAlgorithm_211645361_000000000.h"

namespace Algorithm_211645361_000000000{
    REGISTER_TANK_ALGORITHM(MyTankAlgorithm_211645361_000000000)

    // Helping methods:
    bool MyTankAlgorithm_211645361_000000000::isInDanger(MyBattleInfo *myInfo, Block *block) {
        std::vector<Block*> shellBlocks= myInfo->getAllSymbolBlocks(Symbols::shell);
        for(Block* shellBlock : shellBlocks){
            std::pair<int,int> aim = block->vectorTo(shellBlock); // the direction between {block} and the shell
            if(Utils::isOneOfDirections(aim)) // it is a direct line
                return true;
        }
        return false;
    }

    bool MyTankAlgorithm_211645361_000000000::willLikelyHitMyself(MyBattleInfo *myInfo) {
        Board* demoBoard = myInfo->getDemoBoard();
        Block* shooter = myInfo->getTankBlock(); // the block the shooter is standing on
        auto direction = shooter->getTank()->getCannonDirection(); // his aim
        Block* shellTrajectory = demoBoard->getNeighbor(shooter, direction); // this will track the block of the shell
        while(shellTrajectory != shooter){ // as long as the shell doesn't hit the shooter:
            auto [x,y] = shellTrajectory->getPosition();
            char entitySymbol = myInfo->getView()->getObjectAt(x,y); // get the symbol of the entity on that block
            if(entitySymbol == Symbols::wall || entitySymbol == Symbols::shell)
                return false; // if it's a wall\shell, the shell will be exploded before it reaches him, he can take the shot
            shellTrajectory = demoBoard->getNeighbor(shellTrajectory, direction); // not a wall, the shell advance
        }
        return true;
    }

    bool MyTankAlgorithm_211645361_000000000::tryToEvacuate(MyBattleInfo *myInfo) {
        Board *board = myInfo->getDemoBoard();
        Block* tankBlock = myInfo->getTankBlock();
        Tank* tank = tankBlock->getTank();
        char playerSymbol = Utils::getPlayerSymbol(this->playerIndex);
        // the player IS in danger, at least one shell is about to hit him:
        // first, try to understand if there is a block the player CAN move to, where he wouldn't be caught in the line of fire:
        for(int n: Constants::closeRotations){ // give advantage to close rotation (quickest transition)
            auto targetDir = static_cast<Enums::Direction>(Utils::mod(tank->getCannonDirection()+n, 8));
            Block* neighborBlock = board->getNeighbor(tankBlock, targetDir); // getting the neighbor block
            auto [x,y] = neighborBlock->getPosition();
            char neighborSymbol = myInfo->getView()->getObjectAt(x,y);
            // we can only move there if it is an empty block or a block with an enemy:
            if(neighborSymbol == Symbols::empty || (Utils::isPlayer(neighborSymbol) && neighborSymbol != playerSymbol)){
                if(board->at({x,y})->getSymbol() != playerSymbol)
                    return false;
                // meaning: the player CAN move there (not a wall, shell or one of our other tanks)
                // we also checked that none of our teammates are going to be there. If so, find another neighbor (we'll rather die than kill a teammate - lose 1 instead of 2)
                // ok now all we need to check is that the neighbor block is safe:
                if(!isInDanger(myInfo, neighborBlock)){
                    moveToNeighbor(myInfo, neighborBlock, targetDir, false); // it is safe, move there, no time to shoot.
                    return true;
                }
            }
        }
        return false;
    }

    void MyTankAlgorithm_211645361_000000000::setToShoot(MyBattleInfo *myInfo) {
        Block* shooterBlock = myInfo->getTankBlock();
        Tank* tank = shooterBlock->getTank();
        Enums::Direction shootingDir = tank->getCannonDirection();
        // if there is a clear shot at an enemy, and the tank can shoot - do it:
        if(isAimingAtEnemyOnly(myInfo, shooterBlock, shootingDir) && tank->canShoot()){
            shoot(tank);
            return;
        }// there is no clear shot, check if we need to rotate:
        if(rotateToShoot(myInfo, shooterBlock))
            return;
        // there is no clear shot and rotating will not work.
        // if the shooting protocol is Reposition - find a neighbor block to move to, that has a clear shot at an enemy.
        if(myInfo->getShootingProtocol() == MyBattleInfo::Reposition)
            moveToShoot(myInfo);
    }

    bool MyTankAlgorithm_211645361_000000000::isFriendlyFire(MyBattleInfo *myInfo, Block *shooterBlock,Enums::Direction aim) const {
        // return true iff the tank is aiming at a tank from his team
        char playerSymbol = Utils::getPlayerSymbol(this->playerIndex);
        std::vector<Block*> teamBlocks = myInfo->getAllSymbolBlocks(playerSymbol);
        for(Block* block : teamBlocks){
            if(block == shooterBlock)
                continue;
            if(shooterBlock->isDirectLine(block,aim))
                return true;
        }
        return false;
    }

    bool MyTankAlgorithm_211645361_000000000::isAimingAtEnemyOnly(MyBattleInfo *myInfo, Block *shooterBlock,Enums::Direction aim) const {
        // return true iff the tank is aiming at an enemy (and not a member of his team)
        std::vector<Block*> enemyBlocks = myInfo->getAllEnemyBlocks(this->playerIndex);
        for(Block* block : enemyBlocks){
            if(shooterBlock->isDirectLine(block,aim))
                return !isFriendlyFire(myInfo, shooterBlock, aim);
        }
        return false;
    }

    void MyTankAlgorithm_211645361_000000000::placeDemoTank(MyBattleInfo *myInfo) {
        Block* tankBlock = myInfo->getTankBlock();
        tankBlock->place(std::make_unique<Tank>(
                this->tankIndex,
                this->playerIndex,
                myInfo->getInitialNumShells()));
    }

    void MyTankAlgorithm_211645361_000000000::moveToNeighbor(MyBattleInfo *myInfo, Block *nextBlock,Enums::Direction targetDir, bool takeClearShot) {
        Tank* tank = myInfo->getTankBlock()->getTank();
        // if the neighbor is on our direction, proceed forward
        if(targetDir == tank->getCannonDirection()){
            if(!moveForward(tank))
                return;
        }
            // the neighbor is behind us, need to move backward
        else if(targetDir == tank->getCannonBackwardDirection()){
            if(!moveBackward(tank))
                return;
        }
        else{ // need to rotate and after that move forward:
            if(!rotate(myInfo, targetDir, takeClearShot) || !moveForward(tank))
                return;
        }
        myInfo->updateTankBlock(nextBlock);
    }

    void MyTankAlgorithm_211645361_000000000::moveToShoot(MyBattleInfo *myInfo) {
        Block* shooterBlock = myInfo->getTankBlock();
        Tank* tank = shooterBlock->getTank();
        for(int n: Constants::closeRotations){
            auto targetDir = static_cast<Enums::Direction>(Utils::mod(tank->getCannonDirection()+n, 8));
            Block* neighborBlock = myInfo->getDemoBoard()->getNeighbor(shooterBlock, targetDir); // getting the neighbor block
            auto [x,y] = neighborBlock->getPosition();
            char neighborSymbol = myInfo->getView()->getObjectAt(x,y);
            char playerSymbol = Utils::getPlayerSymbol(this->playerIndex);
            if(neighborSymbol == Symbols::empty || (Utils::isPlayer(neighborSymbol) && neighborSymbol != playerSymbol)){
                // meaning: the player CAN move there (not a wall, shell or one of our other tanks)
                // but if a tank from our team is destined to be there, don't progress.
                if(myInfo->getDemoBoard()->at({x,y})->getSymbol() == playerSymbol)
                    continue;
                for(Block* block : myInfo->getAllEnemyBlocks(this->playerIndex)){
                    std::pair<int,int> aim = neighborBlock->vectorTo(block);
                    if(Utils::isOneOfDirections(aim)){
                        moveToNeighbor(myInfo, neighborBlock, targetDir, false);
                        rotateToShoot(myInfo,neighborBlock);
                        return;
                    }
                }
            }
        }
    }

    bool MyTankAlgorithm_211645361_000000000::rotateToShoot(MyBattleInfo *myInfo, Block *shooterBlock) {
        std::vector<Block*> enemyBlocks = myInfo->getAllEnemyBlocks(this->playerIndex);
        Tank* tank = myInfo->getTankBlock()->getTank();
        for(Block* block : enemyBlocks){ // for every enemy tank
            std::pair<int,int> aim = shooterBlock->vectorTo(block); // get the vector from this tank to their tank
            if(Utils::isOneOfDirections(aim)){ // is it one of the 8 directions?
                Enums::Direction dir = Utils::deltaToDirection(aim); // it is - 'dir' is the direction.
                auto rotations = Utils::directionToRotation(tank->getCannonDirection(), dir);
                // if it is a friendly fire, or we can't add <#rotations needed> actions + 1 (for shooting) look for another option
                if(isFriendlyFire(myInfo, shooterBlock, dir) || !canAddBundleActions((int)rotations.size() + 1))
                    continue;
                rotate(myInfo,dir, false); // rotate to it
                if(tank->canShoot())
                    shoot(tank); // the player has a clear shot at an enemy now.
                return true;
            }
        }
        return false;
    }

    std::vector<std::vector<Node>> MyTankAlgorithm_211645361_000000000::getGraphFromBoard(Board *board) const {
        std::vector<std::vector<Node>> graph;
        graph.resize(board->getHeight());
        for (size_t y = 0; y < board->getHeight(); ++y) {
            graph[y].reserve(board->getWidth());
            for (size_t x = 0; x < board->getWidth(); ++x)
                graph[y].emplace_back(board->at({y, x}));  // This is on purpose!
        }
        return graph;
    }

    std::stack<Block *> MyTankAlgorithm_211645361_000000000::BFS(MyBattleInfo *myInfo, Block *target) const{
        // getting the graph:
        Board* demoBoard = myInfo->getDemoBoard();
        std::vector<std::vector<Node>> graph = getGraphFromBoard(demoBoard); // convert the board into a graph
        auto [rootX,rootY] = myInfo->getTankBlock()->getPosition(); // player
        Node* root = &graph[rootX][rootY];
        auto [goalX,goalY] = target->getPosition(); // enemy
        Node* goal = &graph[goalX][goalY];
        // BFS Algorithm:
        std::queue<Node*> Q;
        root->markExplored();
        Q.emplace(root);
        while(!Q.empty()){
            Node* node = Q.front(); // store the head
            Q.pop(); // remove the node we're working on
            if(node->getBlock() == goal->getBlock())
                return node->getChildToParentStack();
            // for everyone of the 8 adjacent blocks
            for(int i = 0; i < 8; ++i){
                auto direction = static_cast<Enums::Direction>(i);
                Block* neighborBlock = demoBoard->getNeighbor(node->getBlock(), direction);
                auto [x,y] = neighborBlock->getPosition();
                char entitySymbol = myInfo->getView()->getObjectAt(x,y);  // getting the symbol at (x,y) from the satellite
                // if the neighbor is empty or has an enemy tank on it - progress:
                char playerSymbol = Utils::getPlayerSymbol(this->playerIndex);
                if(entitySymbol == Symbols::empty|| (Utils::isPlayer(entitySymbol) && entitySymbol != playerSymbol)){
                    // but if a tank from our team is destined to be there, don't progress.
                    if(demoBoard->at({x,y})->getSymbol() == playerSymbol){
                        if(neighborBlock == target)
                            return {}; // no reason in keep checking, another tank from our team is targeting this enemy
                        continue; // keep checking for alternatives paths, this one is blocked by one of our teammates
                    }
                    Node *neighbor = &graph[x][y];
                    if (!neighbor->hasExplored()) { // we've not visited this node yet
                        neighbor->markExplored();
                        neighbor->setParent(node);
                        Q.emplace(neighbor);
                    }
                }
            }
        }
        return {};
    }

    std::stack<Block *> MyTankAlgorithm_211645361_000000000::getPathToClosestEnemy(MyBattleInfo *myInfo) {
        Board* board = myInfo->getDemoBoard();
        size_t minDistance = 5 * board->getWidth() * board->getHeight();
        std::stack<Block*> shortestPath = {};
        std::vector<Block*> enemyBlocks = myInfo->getAllEnemyBlocks(this->playerIndex);
        for(Block* block : enemyBlocks){
            std::stack<Block*> path = BFS(myInfo, block);
            if(!path.empty() && path.size() < minDistance){ // we certainly don't prefer the "No-Path" path
                minDistance = path.size();
                shortestPath = path;
            }
        }
        return shortestPath;
    }

    void MyTankAlgorithm_211645361_000000000::refillActionQueue(MyBattleInfo *myInfo) {
        // here we need to give the tank actions to do
        Board* board = myInfo->getDemoBoard();
        Tank* tank = myInfo->getTankBlock()->getTank();
        // first, get the closest path to an enemy (in terms of number of blocks)
        std::stack<Block*> path = getPathToClosestEnemy(myInfo);
        if(path.empty()){ // no path will keep the player alive
            if(!isInDanger(myInfo, myInfo->getTankBlock()))
                setToShoot(myInfo); // try to take a shot at an enemy if possible
            else if(tryToEvacuate(myInfo)) return; // the tank is in danger, try to evacuate. If successful, let GameManager progress
            // the tank is either not in danger, or can't evacuate to a safe neighbor, might as well fire as much as he can
            if(this->actionsQueue.empty()) { // there isn't a direct line to any of the enemies, try to break the wall we're facing
                for(int i = 0; i < Constants::hitsToBreakWall; ++i){
                    if(tank->isOutOfAmmo()) return;
                    while (!tank->canShoot()){
                        if(!canAddAction()) return;
                        doNothing(tank);
                    }
                    if(!isFriendlyFire(myInfo, myInfo->getTankBlock(), tank->getCannonDirection()) && !willLikelyHitMyself(myInfo))
                        shoot(tank);
                } // after we broke the wall - rotate the tank so next time it will shoot at another wall
                rotate(myInfo, Utils::rotationToDirection(Enums::EasyLeft, tank->getCannonDirection()), false);
            }
            return; // not much to do now, let GameManager progress
        }
        // there is a safe path for the player to reach an enemy
        while(!path.empty() && canAddAction()){
            // no need to check for incoming shell because the tank is on the move. Might as well just progress as we planned
            Block* nextBlock = path.top(); // next block in the path
            path.pop();
            Enums::Direction targetDir = board->directionToNeighbor(myInfo->getTankBlock(),nextBlock); // direction to this block (neighbor)
            // if the player has his tank aimed at an enemy and can shoot, take the shot.
            if(isAimingAtEnemyOnly(myInfo, myInfo->getTankBlock(), tank->getCannonDirection()) && tank->canShoot())
                shoot(tank);
            // move to that neighbor
            moveToNeighbor(myInfo, nextBlock, targetDir, true);
        }
    }


    // Commands:
    bool MyTankAlgorithm_211645361_000000000::moveForward(Tank *tank) {
        if(!canAddAction())
            return false;
        this->actionsQueue.emplace(ActionRequest::MoveForward);
        tank->decCooldowns();
        tank->cancelBackwardEligibility();
        return true;
    }

    bool MyTankAlgorithm_211645361_000000000::moveBackward(Tank *tank) {
        if(!canAddAction()) return false;
        this->actionsQueue.emplace(ActionRequest::MoveBackward);
        if(!tank->hasRequestedBackward()) // a new backward request
            tank->startBackwardRequest();
        while(!tank->canMoveBackward()){
            if(!canAddAction()) return false;
            this->actionsQueue.emplace(ActionRequest::MoveBackward);
            tank->decCooldowns();
        }
        return true;
    }

    void MyTankAlgorithm_211645361_000000000::shoot(Tank *tank) {
        if(!canAddAction())
            return;
        this->actionsQueue.emplace(ActionRequest::Shoot);
        tank->setShootingCooldown();
        tank->decArtillery();
        tank->decCooldowns();
        tank->cancelBackwardEligibility();
    }

    bool MyTankAlgorithm_211645361_000000000::rotate(MyBattleInfo *myInfo, Enums::Direction targetDir, bool takeClearShot) {
        Tank* tank = myInfo->getTankBlock()->getTank();
        auto rotations = Utils::directionToRotation(tank->getCannonDirection(), targetDir);
        for(Enums::Rotation r : rotations){ // can be multiple rotations
            if(!canAddAction())
                return false;
            this->actionsQueue.emplace(Utils::rotationToAction(r));
            tank->decCooldowns();
            tank->cancelBackwardEligibility();
            tank->rotateCannon(r);
            if(takeClearShot && isAimingAtEnemyOnly(myInfo, myInfo->getTankBlock(), tank->getCannonDirection()) && tank->canShoot())
                shoot(tank);
        }
        return true;
    }

    void MyTankAlgorithm_211645361_000000000::doNothing(Tank *tank) {
        this->actionsQueue.emplace(ActionRequest::DoNothing);
        tank->decCooldowns();
        tank->cancelBackwardEligibility();
    }

    void MyTankAlgorithm_211645361_000000000::getBattleInfo(Tank *tank) {
        this->actionsQueue.emplace(ActionRequest::GetBattleInfo);
        tank->decCooldowns();
        tank->cancelBackwardEligibility();
    }


    // Predicates:
    bool MyTankAlgorithm_211645361_000000000::canAddAction() {
        return actionsQueue.size() < this->queueMaxSize - 1;
    }

    bool MyTankAlgorithm_211645361_000000000::canAddBundleActions(size_t n) {
        return actionsQueue.size() + n <= this->queueMaxSize - 1;
    }


    // Constructor:
    MyTankAlgorithm_211645361_000000000::MyTankAlgorithm_211645361_000000000(int playerIndex, int tankIndex) {
        this->playerIndex = playerIndex;
        this->tankIndex = tankIndex;
    }


    // Override
    ActionRequest MyTankAlgorithm_211645361_000000000::getAction() {
        // There are no pending actions, ask for an update on the battle
        if(this->actionsQueue.empty())
            return ActionRequest::GetBattleInfo;
        // There are still pending actions to be executed, send the next one
        ActionRequest action = this->actionsQueue.front();
        this->actionsQueue.pop(); // the GM will execute it, we can remove the action from the queue
        return action;
    }

    void MyTankAlgorithm_211645361_000000000::updateBattleInfo(BattleInfo &info) {
        // the new information about the battle has arrived, we can down-cast to MyBattleInfo:
        auto& myBattleInfo = dynamic_cast<MyBattleInfo&>(info);
        /* if this tank has YET to be registered (placing a demo tank on the demo board)
         * then we (Algorithm) will do it, adding the tankIndex we got at the constructor.
         * the player maintains demo tanks for HIS TANKS ONLY. Therefore, a check for the entity type tank would be sufficient.
         */
        if(!myBattleInfo.getTankBlock()->isStaticType(Entity::Type::TANK))
            placeDemoTank(&myBattleInfo);

        // all set! we can start filling the actions queue for this tank
        this->queueMaxSize = myBattleInfo.getMaxActions();
        refillActionQueue(&myBattleInfo);
        getBattleInfo(myBattleInfo.getTankBlock()->getTank());
    }
}
