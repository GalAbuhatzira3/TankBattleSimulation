#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include <atomic>
#include <dlfcn.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <iostream>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <chrono>
#include <sched.h>
#include <utility>
#include <random>  // ONLY FOR THE MAP GENERATOR!
#include <optional>
#include <algorithm>
#include <filesystem>
// Interfaces:
#include "AlgorithmRegistrar.h"
#include "GameManagerRegistrar.h"
// Implementations:
#include "../UserCommon/MySatelliteView.h"
#include "../UserCommon/Config.h"
#include "CommandLineParser.h"
// Namespaces
using namespace UserCommon_211645361_000000000;
// Threads:
using Task = std::function<void()>;

class BaseSimulator {
public:
    explicit BaseSimulator(const Arguments& args);
    virtual ~BaseSimulator();
    void simulate(); // Runs the full pipeline: scan → load → run → cleanup

protected:
    struct BoardData{
        std::string mapName;
        size_t maxSteps{};
        size_t numShells{};
        size_t mapHeight{};
        size_t mapWidth{};
        std::vector<std::string> boardLines;
        std::unique_ptr<SatelliteView> map;
    };

    const Arguments args; // will hold the arguments
    std::vector<std::string> gameManagerSOs; // Comparative - list of file paths, Competitive - 1 file path
    std::vector<std::string> algorithmSOs;   // Comparative - 1 file path       , Competitive - list of file paths
    std::vector<void*> openedAlgorithmsHandles;
    std::vector<void*> openedGameManagerHandles;
    AlgorithmRegistrar& algorithmRegistrar;
    GameManagerRegistrar& gameManagerRegistrar;
    std::unique_ptr<Logger> logger;
    std::string uniqueTime;
    MiniCfg cfg = loadCfg(Constants::ConfigFilePath);

    // Simulator helping functions:
    void runWithThreads(std::vector<Task>& tasks); 
    void loadSharedObjects();
    void cleanup();

    // Implemented in derived classes:
    virtual void scanSharedObjects() = 0;
    virtual void runMatches() = 0;

    // Predicates:
    bool isValidResult(const GameResult& result, const std::string& gmName);

    // Board:
    BoardData readBoard(const std::string &inputFilePath);
    void trimLine(std::string &line);
    std::vector<std::string> readLines(const std::string &filename);
    size_t parseVariables(const std::vector<std::string> &lines, BoardData& boardData);
    void captureBoard(const std::vector<std::string> &lines, size_t lastVarLine, BoardData& boardData);
    void warnColumnCounts(BoardData& boardData);
    std::vector<std::vector<char>> buildMatrix(BoardData& boardData);
    void warnRowCount(BoardData &boardData);
    std::string generateMap();

    // Logger:
    void setLogger();
    void logBoardData(BoardData &boardData);
};
