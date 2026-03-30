#include <utility>
#include "MySatelliteView.h"

namespace UserCommon_211645361_000000000 {
// Constructor:
    MySatelliteView::MySatelliteView(std::vector<std::vector<char>> mat){
        this->boardCharMat = std::move(mat);
    }

// Override:
    char MySatelliteView::getObjectAt(size_t x, size_t y) const{
        if(x >= this->boardCharMat.size() || y >= this->boardCharMat[0].size())
            return '&';
        return this->boardCharMat[x][y];
    }

    bool MySatelliteView::operator==(const MySatelliteView& other) const {
        return boardCharMat == other.boardCharMat;
    }

    std::string MySatelliteView::toString() const {
        std::string txt;
        if (this->boardCharMat.empty()) return "";

        for (size_t r = 0; r < this->boardCharMat.size(); ++r) {
            for (size_t c = 0; c < this->boardCharMat[0].size(); ++c)
                txt += boardCharMat[r][c];
            txt += "\n";
        }
        return txt;
    }
}


