#include "ComparativeSimulator.h"

ComparativeSimulator::ComparativeSimulator(const Arguments& args): BaseSimulator(args){}

void ComparativeSimulator::scanSharedObjects() {
    // scanning the game map, obtaining information about the board and the game:
    std::string gameMapPath = this->args.gameMap;
    if(this->args.generateMaps)
        for(int i = 0; i < this->args.generateMaps; i++) gameMapPath = generateMap();
    this->gameMapData = readBoard(gameMapPath);
    // extracting files path from the GameManager folder:
    this->gameManagerSOs = Files::getListOfFilesFromFolder(this->args.gameManagersFolder, ".so");
    // storing the two algorithms:
    this->algorithmSOs = this->args.algorithms;
}

bool areGameResultsEqual(const GameResult& g1, const GameResult& g2){
    // they must have the same winner or the same reason for a tie:
    if(g1.winner != g2.winner && g1.reason != g2.reason) return false;
    // they must have the same number of rounds:
    if(g1.rounds != g2.rounds) return false;
    // they must have the same board at the end:
    auto* g1SatelliteView = g1.gameState.get();
    auto* g2SatelliteView = g2.gameState.get();
    if(!g1SatelliteView && !g2SatelliteView) return true; // both empty
    if(!g1SatelliteView || !g2SatelliteView) return false; // one null, one not
    // both ok:
    auto* g1MySatelliteView = dynamic_cast<const MySatelliteView*>(g1SatelliteView);
    auto* g2MySatelliteView = dynamic_cast<const MySatelliteView*>(g2SatelliteView);
    return (*g1MySatelliteView) == (*g2MySatelliteView); //  relies on MySatelliteView::operator==
}

std::string getGameResultMessage(const GameResult& result, size_t maxSteps){
    std::string txt;
    if (result.winner == 1 || result.winner == 2) // a player has won the match, print a victory message:
        return "Player " + std::to_string(result.winner) + " won with " + std::to_string(result.remaining_tanks[result.winner - 1]) + " tanks still alive\n";
    else if(result.winner == 0 && result.reason == GameResult::ALL_TANKS_DEAD){ // an all-tanks-are-dead draw
        return "Tie, all players have zero tanks\n";
    }else if(result.winner == 0 && result.reason == GameResult::ZERO_SHELLS){ // a zero-shells draw
        return "Tie, both players have zero shells for " + std::to_string(Constants::outOfAmmoAdditionalSteps) + " steps\n";
    }else if(result.winner == 0 && result.reason == GameResult::MAX_STEPS){ // a max-steps-reached draw
        txt = "Tie, reached max steps = " + std::to_string(maxSteps) +"\n";
        for(int i = 0; i < Constants::numOfPlayers; ++i)
            txt += "player " + std::to_string(i+1) + " has " + std::to_string(result.remaining_tanks[i]) + " tanks\n";
    }
    return txt;
}

std::string getResultsAsText(std::vector<std::pair<std::string, GameResult>> &gameResults, size_t maxSteps) {
    std::string txt;
    size_t N = gameResults.size();
    std::vector<bool> used(N, false);
    for (size_t i = 0; i < N; ++i) {
        if(used[i]) continue; // already grouped
        used[i] = true;
        std::vector<size_t> group;
        group.push_back(i);
        for(size_t j = 0; j < N; j++){
            if(j == i || used[j] || !areGameResultsEqual(gameResults[i].second, gameResults[j].second)) continue;
            used[j] = true;
            group.push_back(j);
        }
        // print the GameManagers of the group:
        for(size_t index : group)
            txt += gameResults[index].first + ";";
        // print the other data:
        txt += '\n';
        if (group.empty()) continue;
        auto const& commonResult = gameResults[group[0]].second;
        txt += getGameResultMessage(commonResult, maxSteps); // game result
        txt += std::to_string(commonResult.rounds) + '\n'; // rounds
        txt += dynamic_cast<const MySatelliteView*>(commonResult.gameState.get())->toString(); // SatelliteView
    }
    return txt;
}

void ComparativeSimulator::pushOutput(std::vector<std::pair<std::string, GameResult>> *gameResults, BoardData *boardData, const std::string& uniqueTime){
    std::string txt;
    std::string outputFilePath = this->args.gameManagersFolder+"/comparative_results_"+uniqueTime+".txt";
    txt += "game_map=" + this->args.gameMap + "\n";
    txt += "algorithm1=" + Files::getPathTail(this->args.algorithms[0]) + "\n";
    txt += "algorithm2=" + Files::getPathTail(this->args.algorithms[1]) + "\n";
    txt += "\n";
    txt += getResultsAsText(*gameResults, boardData->maxSteps);
    Files::pushToFile(outputFilePath, txt);
}

void ComparativeSimulator::runMatches() {
    std::vector<Task> tasks;
    std::mutex lock;
    std::vector<std::pair<std::string, GameResult>> gameResults; // this vector will store the results of the games:
    std::string uniqueTime = Utils::getUniqueTime();
    BoardData &boardData = this->gameMapData; // getting the data from the game map:
    logBoardData(boardData);
    auto iterator = this->algorithmRegistrar.begin();
    auto end = this->algorithmRegistrar.end();
    const auto& algorithm1 = *iterator; // getting the 1st Algorithm (*iterator++)
    // "it is allowed if algorithm1 and algorithm2 point to the same .so file"
    const auto& algorithm2 = (std::next(iterator) != end) ? *std::next(iterator) : *iterator;
    // now, for every GameManager in the GameManagers folder, run it on the game map:
    for (auto& gmEntry : this->gameManagerRegistrar) {
        tasks.emplace_back([&, gmEntry=gmEntry](){
            auto gameManager = gmEntry.createGameManager(args.verbose); // getting this GameManager:
            auto player1 = algorithm1.createPlayer(1, boardData.mapWidth, boardData.mapHeight, boardData.maxSteps, boardData.numShells);
            this->logger->info("Config.players player=1 algorithm="+algorithm1.name()+" result=created");
            auto player2 = algorithm2.createPlayer(2, boardData.mapWidth, boardData.mapHeight, boardData.maxSteps, boardData.numShells);
            this->logger->info("Config.players player=2 algorithm="+algorithm2.name()+" result=created");
            // get the result of this game:
            this->logger->info("Config.match GameManager="+gmEntry.name()+" map="+boardData.mapName+" player1="+algorithm1.name()+" player2="+algorithm2.name());
            GameResult result = gameManager->run(
                    boardData.mapWidth,boardData.mapHeight,
                    *boardData.map, boardData.mapName,
                    boardData.maxSteps,
                    boardData.numShells,
                    *player1, algorithm1.name(),
                    *player2, algorithm2.name(),
                    algorithm1.getTankAlgorithmFactory(),algorithm2.getTankAlgorithmFactory());
            std::lock_guard<std::mutex> lg(lock);
            if(isValidResult(result, gmEntry.name()))
                gameResults.emplace_back(gmEntry.name(), std::move(result));
            else
                Files::pushToFile(Constants::errorFilePath, "The result returned by GameManager isn't valid");
        });
    }
    runWithThreads(tasks); // run the tasks in parallel:
    pushOutput(&gameResults, &boardData, uniqueTime); // writing to the output file:
}

