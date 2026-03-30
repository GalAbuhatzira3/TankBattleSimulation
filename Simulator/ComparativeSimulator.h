#pragma once
#include "BaseSimulator.h"

class ComparativeSimulator : public BaseSimulator {
public:
    explicit ComparativeSimulator(const Arguments& args);

protected:
    // Attributes:
    BoardData gameMapData; // will store the data about the one game map provided

    // Override:
    void scanSharedObjects() override;
    void runMatches() override;

    // Printing:
    void pushOutput(std::vector<std::pair<std::string, GameResult>> *gameResults, BoardData *boardData, const std::string& uniqueTime);
};
