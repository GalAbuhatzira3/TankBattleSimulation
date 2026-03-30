#pragma once
#include <vector>
#include <memory>
// Game Objects:
#include "Board.h"
// Interfaces:
#include "../common/SatelliteView.h"


namespace UserCommon_211645361_000000000 {
    class MySatelliteView : public SatelliteView {
    private:
        // Attributes:
        std::vector<std::vector<char>> boardCharMat;

    public:
        // Constructor:
        explicit MySatelliteView(std::vector<std::vector<char>> mat);

        // Override:
        char getObjectAt(size_t x, size_t y) const override;

        bool operator==(const MySatelliteView& other) const;

        [[nodiscard]]
        std::string toString() const;
    };
}

