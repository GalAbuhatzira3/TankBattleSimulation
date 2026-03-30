#include "CompetitiveSimulator.h"


CompetitiveSimulator::CompetitiveSimulator(const Arguments& args): BaseSimulator(args){}

void CompetitiveSimulator::scanSharedObjects() {
    // scanning the game map folder, and extracting the BoardData objects:
    std::vector<std::string> gameMapsPaths;
    std::string gameMapsFolder = this->args.gameMapsFolder;
    if(this->args.generateMaps)
        for(int i = 0; i < this->args.generateMaps; i++) gameMapsFolder = generateMap();
    gameMapsPaths = Files::getListOfFilesFromFolder(gameMapsFolder, ".txt");
    for(auto& path : gameMapsPaths)
        this->gameMapsData.push_back(readBoard(path));
    // storing the GameManager:
    this->gameManagerSOs = {this->args.gameManager};
    // scanning the algorithms folder, and extracting .so files:
    this->algorithmSOs = Files::getListOfFilesFromFolder(this->args.algorithmsFolder, ".so");
}

void updateScoreboard(std::vector<std::pair<std::string, size_t>> &scoreboard, GameResult &result, size_t p1, size_t p2){
    if(result.winner == 0){
        scoreboard[p1].second += Constants::tiePoints;
        scoreboard[p2].second += Constants::tiePoints;
    }else if (result.winner == 1){
        scoreboard[p1].second += Constants::winPoints;
        scoreboard[p2].second += Constants::losePoints;
    }else if (result.winner == 2){
        scoreboard[p1].second += Constants::losePoints;
        scoreboard[p2].second += Constants::winPoints;
    }
}

std::string getResultsAsText(std::vector<std::pair<std::string, size_t>> &scoreboard){
    std::string txt;
    std::sort(scoreboard.begin(), scoreboard.end(),
              [] (auto const& a, auto const& b)
              {return a.second > b.second;}
    );
    for (auto const& [AlgorithmName, totalScore] : scoreboard)
        txt += AlgorithmName + " " + std::to_string(totalScore) + "\n";
    return txt;
}

void CompetitiveSimulator::pushOutput(std::vector<std::pair<std::string, size_t>> *scoreboard){
    std::string txt;
    std::string outputFilePath = this->args.algorithmsFolder+"/competition_"+Utils::getUniqueTime()+".txt";
    txt += "game_maps_folder=" + this->args.gameMapsFolder + "\n";
    txt += "game_manager=" + Files::getPathTail(this->args.gameManager) + "\n\n";
    txt += getResultsAsText(*scoreboard);
    Files::pushToFile(outputFilePath, txt);
}

void CompetitiveSimulator::runMatches() {
    std::vector<Task> tasks;
    std::mutex lock;
    auto& gmEntry = *this->gameManagerRegistrar.begin();
    // getting the sizes:
    size_t N = this->algorithmRegistrar.count(); // the number of algorithms
    size_t K = this->gameMapsData.size(); // the number of game maps:
    // getting the algorithms:
    using EntryType = std::decay_t<decltype(*algorithmRegistrar.begin())>;
    std::vector<EntryType> algorithms(this->algorithmRegistrar.begin(),this->algorithmRegistrar.end());
    if(this->algorithmRegistrar.count() <= Constants::minLoadedAlgorithmSO)
        Files::error("For competitive mode you have to load at least 2 algorithms .so files");
    // a win counter for the algorithms:
    std::vector<std::pair<std::string, size_t>> scoreboard;
    scoreboard.reserve(N);
    for (size_t i = 0; i < N; ++i)
        scoreboard.emplace_back(algorithms[i].name(),0); // <Algorithm name, 0>
    // for each map k, and each Algorithm i, compute the opponent Algorithm j:
    for(size_t k = 0; k < K; k++){
        for (size_t i = 0; i < N; ++i) {
            size_t j = (i+1 + (k % (N-1))) % N;
            if(N%2 == 0 && k == N/2 + 1 && i >= N/2)
                continue; //in the case of k = N/2 - 1 (if N is even), the pairing for each algorithm in both games would be exactly the same.
            tasks.emplace_back([&, k, i, j]() {
                auto& boardData = this->gameMapsData[k];
                logBoardData(boardData);
                const auto& algorithm1 = algorithms[i];
                const auto& algorithm2 = algorithms[j];
                auto player1 = algorithm1.createPlayer(1, boardData.mapWidth, boardData.mapHeight, boardData.maxSteps, boardData.numShells);
                this->logger->info("Config.players player=1 algorithm="+algorithm1.name()+" result=created");
                auto player2 = algorithm2.createPlayer(2, boardData.mapWidth, boardData.mapHeight, boardData.maxSteps, boardData.numShells);
                this->logger->info("Config.players player=2 algorithm="+algorithm2.name()+" result=created");
                // getting a fresh GameManager instance:
                auto gameManager = gmEntry.createGameManager(args.verbose); // the GameManager
                this->logger->info("Config.match GameManager="+gmEntry.name()+" map="+boardData.mapName+" player1[ind="+std::to_string(i)+"]="+algorithm1.name()+" player2[ind="+std::to_string(j)+"]="+algorithm2.name());
                GameResult result = gameManager->run(
                        boardData.mapWidth,boardData.mapHeight,
                        *boardData.map, boardData.mapName,
                        boardData.maxSteps,
                        boardData.numShells,
                        *player1, algorithm1.name(),
                        *player2, algorithm2.name(),
                        algorithm1.getTankAlgorithmFactory(), algorithm2.getTankAlgorithmFactory());
                std::lock_guard<std::mutex> lg(lock);
                if(isValidResult(result, gmEntry.name()))
                    updateScoreboard(scoreboard, result, i, j); // [i] vs [j]
                else
                    Files::pushToFile(Constants::errorFilePath, "The result returned by GameManager isn't valid");
            });
        }
    }
    runWithThreads(tasks); // run the tasks in parallel:
    pushOutput(&scoreboard); // writing to the output file:
}