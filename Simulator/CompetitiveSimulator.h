#pragma once
#include "BaseSimulator.h"

class CompetitiveSimulator : public BaseSimulator {
public:
    explicit CompetitiveSimulator(const Arguments& args);

protected:
    // Attributes:
    std::vector<BoardData> gameMapsData; // the list of all the board data

    // Override:
    void scanSharedObjects() override;
    void runMatches() override;

    // Printing:
    void pushOutput(std::vector<std::pair<std::string, size_t>> *scoreboard);
};
