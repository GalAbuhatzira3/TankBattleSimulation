#pragma once
#include <iomanip>
#include <ios>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <stack>
#include <functional>
#include "variant"
#include <experimental/filesystem>
// Interfaces:
#include "../common/SatelliteView.h"
#include "../common/ActionRequest.h"
#include "../common/GameResult.h"

namespace fs = std::experimental::filesystem;
using namespace std::chrono;

namespace UserCommon_211645361_000000000 {
    namespace Constants {
        inline constexpr int numOfPlayers = 2; // the number of players - 2
        inline constexpr int shootingCooldown = 5; // the number of steps the player has to wait before firing again
        inline constexpr int backwardMoveCooldown = 2; // the number of steps the player has to wait before moving back again
        inline constexpr int shellSpeed = 2; // the number of blocks a shell is progressing in a step
        inline constexpr int outOfAmmoAdditionalSteps = 40; // the number of steps the game continues after both players are out of ammo
        inline constexpr int hitsToBreakWall = 2; // the number of hits a wall can endure before collapsing
        inline constexpr char const* errorFilePath = "input_errors.txt"; // the path to the file where all the errors will be written into
        inline constexpr int maxActionsSlide = 12; // the upper bound on the maxActions attribute (algorithms)
        inline constexpr int minActionsSlide = 6; // the lower bound on the maxActions attribute (algorithms)
        inline constexpr std::array<int,8> closeRotations = {6,7,0,1,2,4,5,3}; // quicker rotations are ahead
        inline constexpr size_t winPoints = 3; // the points an Algorithm gets for a win
        inline constexpr size_t losePoints = 0; // the points an Algorithm gets for a loss
        inline constexpr size_t tiePoints = 1; // the points an Algorithm gets for a tie
        inline constexpr size_t minLoadedAlgorithmSO = 1; // the minimal number of Algorithms we need in order to operate the game
        inline constexpr size_t minLoadedGameManagerSO = 1; // the minimal number of GameManager we need in order to operate the game
        // see features.txt for more information about the following constants:
        inline constexpr const char* gameManagerLoggerPrefix = "GameManager/GameManager_log_";
        inline constexpr const char* simulatorLoggerPrefix = "Simulator/Simulator_log_";
        inline constexpr const char* ConfigFilePath = "config.ini";
        inline constexpr size_t wallPct = 20;
        inline constexpr size_t minePct = 6;
        inline constexpr size_t emptyPct = 70;
        inline constexpr size_t p1Pct = 3;
        inline constexpr size_t p2Pct = 3;
        inline constexpr size_t maxRows = 25;
        inline constexpr size_t maxCols = 25;
        inline constexpr size_t maxShells = 100;
        inline constexpr size_t maxSteps = 1000;
        inline constexpr const char* mapDir = "maps";
        inline constexpr size_t maxMapsToGenerate = 100;

    }

    namespace Symbols {
        inline constexpr std::array<char,2> players = {'1','2'};
        inline constexpr char mine = '@'; // mine
        inline constexpr char wall = '#'; // a wall with 0 hits
        inline constexpr char shell = '*'; // a shell in movement
        inline constexpr char shellOnMine = '*'; // a shell over mine (visualization)
        inline constexpr char empty = ' ';
        inline constexpr char outOfBounds = '&';
        inline constexpr char thisTank = '%';
    }

    namespace Enums {
        enum Direction{
            U = 0,
            UR = 1,
            R = 2,
            DR = 3,
            D = 4,
            DL = 5,
            L = 6,
            UL = 7
        }; // the 8 directions, each with number i, such that Direction d is d_i * 45 angles

        enum Rotation {
            HardLeft = -2,
            EasyLeft = -1,
            EasyRight = 1,
            HardRight = 2
        }; // the 4 rotations
    }

    namespace Utils {
        // text:
        inline std::string getUniqueTime() {
            auto now = system_clock::now();
            duration<double> ts = now.time_since_epoch();
            constexpr size_t NUM_DIGITS = 9;
            size_t NUM_DIGITS_P = std::pow(10, NUM_DIGITS);
            std::ostringstream oss;
            oss << std::setw(NUM_DIGITS) << std::setfill('0') << size_t(ts.count() * NUM_DIGITS_P) % NUM_DIGITS_P;
            std::string s = oss.str();
            return s;
        }

        inline std::string trim(const std::string& s) {
            auto l = s.find_first_not_of(" \t");
            auto r = s.find_last_not_of(" \t");
            return (l==std::string::npos) ? "" : s.substr(l, r-l+1);
        }

        // Player:
        inline char getPlayerSymbol(int playerIndex) {
            if(playerIndex == 1) return '1';
            if(playerIndex == 2) return '2';
            throw std::invalid_argument("invalid playerID. Supports 2 players");
        }

        inline char isPlayer(char symbol) {
            if(symbol == '1' || symbol == '2')
                return true;
            return false;
        }


        // Calculations:
        inline int mod(int num, int base) {
            return ((((num) % base) + base) % base);
        }

        inline std::string actionToString(ActionRequest action) {
            switch (action) {
                case ActionRequest::MoveForward:
                    return "MoveForward";
                case ActionRequest::MoveBackward:
                    return "MoveBackward";
                case ActionRequest::RotateLeft90:
                    return "RotateLeft90";
                case ActionRequest::RotateRight90:
                    return "RotateRight90";
                case ActionRequest::RotateLeft45:
                    return "RotateLeft45";
                case ActionRequest::RotateRight45:
                    return "RotateRight45";
                case ActionRequest::Shoot:
                    return "Shoot";
                case ActionRequest::GetBattleInfo:
                    return "GetBattleInfo";
                case ActionRequest::DoNothing:
                    return "DoNothing";
            }
            throw std::invalid_argument("Unhandled action request value");
        }


        // Direction functions:
        inline std::pair<int, int> directionToVector(Enums::Direction direction) {
            switch (direction) {
                case Enums::Direction::U:
                    return {-1, 0};
                case Enums::Direction::UR:
                    return {-1, 1};
                case Enums::Direction::R:
                    return {0, 1};
                case Enums::Direction::DR:
                    return {1, 1};
                case Enums::Direction::D:
                    return {1, 0};
                case Enums::Direction::DL:
                    return {1, -1};
                case Enums::Direction::L:
                    return {0, -1};
                case Enums::Direction::UL:
                    return {-1, -1};
                default:
                    throw std::invalid_argument("Unhandled direction value");
            }
        }

        inline Enums::Direction vectorToDirection(std::pair<int, int> vector) {
            int dX = vector.first;
            int dY = vector.second;
            if (dX == -1 && dY == 0) return Enums::Direction::U;
            else if (dX == -1 && dY == 1) return Enums::Direction::UR;
            else if (dX == 0 && dY == 1) return Enums::Direction::R;
            else if (dX == 1 && dY == 1) return Enums::Direction::DR;
            else if (dX == 1 && dY == 0) return Enums::Direction::D;
            else if (dX == 1 && dY == -1) return Enums::Direction::DL;
            else if (dX == 0 && dY == -1) return Enums::Direction::L;
            else if (dX == -1 && dY == -1) return Enums::Direction::UL;
            std::cout << std::to_string(dX) + "," + std::to_string(dY) << std::endl;
            throw std::invalid_argument("Unhandled vector value");
        }

        inline std::string directionToString(Enums::Direction direction) {
            switch (direction) {
                case Enums::Direction::U:
                    return "up";
                case Enums::Direction::UR:
                    return "up-right";
                case Enums::Direction::R:
                    return "right";
                case Enums::Direction::DR:
                    return "down-right";
                case Enums::Direction::D:
                    return "down";
                case Enums::Direction::DL:
                    return "down-left";
                case Enums::Direction::L:
                    return "left";
                case Enums::Direction::UL:
                    return "up-left";
                default:
                    throw std::invalid_argument("Unhandled direction value");
            }
        }

        inline bool isOneOfDirections(std::pair<int, int> delta) {
            int dX = delta.first;
            int dY = delta.second;
            if (dX == 0 && dY == 0)
                return false;
            if (dX == 0 || dY == 0 || std::abs(dX) == std::abs(dY))
                return true;

            return false;
        }

        inline Enums::Direction deltaToDirection(std::pair<int, int> delta) {
            int dX = delta.first;
            int dY = delta.second;

            // at least one of them is zero?
            // dX=0, meaning, the direction is either L or R
            if (dX == 0) { // (0,C)
                if (dY > 0) return Enums::Direction::R;
                if (dY < 0) return Enums::Direction::L;
            }
            // dY=0, meaning, the direction is either U or D
            if (dY == 0) { // (C,0)
                if (dX > 0) return Enums::Direction::D;
                if (dX < 0) return Enums::Direction::U;
            }
            // none of them is zero, meaning it could be diagonal:
            if (std::abs(dX) == std::abs(dY)) {
                if (dX > 0 && dY > 0) return Enums::Direction::DR;
                if (dX > 0 && dY < 0) return Enums::Direction::DL;
                if (dX < 0 && dY > 0) return Enums::Direction::UR;
                if (dX < 0 && dY < 0) return Enums::Direction::UL;
            }
            throw std::invalid_argument("delta is not one of the 8 directions");
        }

        inline std::pair<int, int> getTargetPosition(std::pair<int, int> base, Enums::Direction direction) {
            auto [x, y] = base;
            auto [offsetX, offsetY] = directionToVector(direction);
            return {x + offsetX, y + offsetY};
        }

        inline Enums::Direction getBackwardDirection(Enums::Direction direction) {
            int baseD = static_cast<int>(direction);
            int result = mod(baseD + 4, 8);  // modulo 8
            auto newDirection = static_cast<Enums::Direction>(result);
            return newDirection;
        }


        // Rotation functions:
        inline std::string rotationToString(Enums::Rotation rotation) {
            switch (rotation) {
                case Enums::Rotation::HardLeft:
                    return "hard left";
                case Enums::Rotation::EasyLeft:
                    return "easy left";
                case Enums::Rotation::EasyRight:
                    return "easy right";
                case Enums::Rotation::HardRight:
                    return "hard right";
                default:
                    throw std::invalid_argument("Unhandled rotation value");
            }
        }

        inline Enums::Direction rotationToDirection(Enums::Rotation rotation, Enums::Direction direction) {
            int baseD = static_cast<int>(direction);
            int baseR = static_cast<int>(rotation);
            int result = mod(baseD + baseR, 8); // modulo 8
            auto newDirection = static_cast<Enums::Direction>(result);
            return newDirection;
        }

        inline std::vector<Enums::Rotation> directionToRotation(Enums::Direction from, Enums::Direction to) {
            int baseFrom = static_cast<int>(from);
            int baseTo = static_cast<int>(to);
            int result = mod(baseTo - baseFrom, 8);
            if (result == 0) return {};
            if (result == 1) return {Enums::Rotation::EasyRight};
            if (result == 2) return {Enums::Rotation::HardRight};
            if (result == 3) return {Enums::Rotation::HardRight, Enums::Rotation::EasyRight};
            if (result == 4) return {Enums::Rotation::HardRight, Enums::Rotation::HardRight};
            if (result == 5) return {Enums::Rotation::HardLeft, Enums::Rotation::EasyLeft};
            if (result == 6) return {Enums::Rotation::HardLeft};
            if (result == 7) return {Enums::Rotation::EasyLeft};
            return {};
        }

        inline ActionRequest rotationToAction(Enums::Rotation rotation) {
            switch (rotation) {
                case Enums::HardLeft:
                    return ActionRequest::RotateLeft90;
                case Enums::EasyLeft:
                    return ActionRequest::RotateLeft45;
                case Enums::EasyRight:
                    return ActionRequest::RotateRight45;
                case Enums::HardRight:
                    return ActionRequest::RotateRight90;
                default:
                    throw std::invalid_argument("Unhandled rotation value");
            }
        }

    }

    namespace Files{
        inline std::string getPathTail(const std::string& s){
            auto pos = s.rfind('/');
            return (pos == std::string::npos) ? s: s.substr(pos + 1);
        }

        inline void pushToFile(const std::string& filePath, const std::string& data){
            std::ofstream file(filePath, std::ios::out | std::ios::app);
            if(!file){ // couldn't open the file → print to the standard output
                std::cout<< "Failed opening the file.\n" + data << std::endl;
                return;
            }
            file << data <<std::endl;
            if(!file) // the writing has failed → print to the standard output
                std::cout<< "Failed writing to the file.\n" + data << std::endl;
            file.flush();
            file.close();
        }

        inline void usage(){
            std::cerr << R"(Usage:
            key=value pairs (no spaces around the '=' sign)
            Comparative mode:
                Simulator -comparative
                game_map=<file>
                game_managers_folder=<dir>
                algorithm1=<so-file>
                algorithm2=<so-file>
                [num_threads=<n>] [-verbose] [gen_maps=<n>]

            Competition mode:
                Simulator -competition
                game_maps_folder=<dir>
                game_manager=<so-file>
                algorithms_folder=<dir>
                [num_threads=<n>] [-verbose] [gen_maps=<n>])";
        }

        inline void error(const std::string& msg){
            usage();
            std::cerr << "\nERROR: " << msg << "\n";
            std::exit(1);
        }

        inline void checkFile(const std::string& path, const std::string& argName){
            try{
                if(!fs::exists(path)) // non-existing file
                    error(argName + ": the '" + path + "' file doesn't exist.");
                if (!std::ifstream(path)) // file can't be traversed
                    error(argName + ": the '" + path + "' file can't be opened.");
            } catch(const fs::filesystem_error& err){
                error(argName + ": filesystem error on '" + path + "'. details: " + err.what());
            }
        }

        inline void checkDirectory(const std::string& path, const std::string& argName, const std::string& validExtension){
            try{
                if(!fs::exists(path)) // non-existing folder
                    error(argName + ": the '" + path + "' folder doesn't exist.");
                if(!fs::is_directory(path)) // folder can't be traversed
                    error(argName + ": the '" + path + "' path isn't a directory.");
                // ok, the path is a directory, and does exist, now look for valid files.
                for (auto& entry : fs::directory_iterator(path, fs::directory_options::skip_permission_denied))
                    if (entry.path().extension().string() == validExtension)
                        return; // we found at least one valid file, we can exit now.
                // if we got here, it means there aren't valid files in this folder:
                error(argName + ": no '"+validExtension+"' files found in '" + path + "'."); // folder has 0 valid files
            } catch(const fs::filesystem_error& err){
                error(argName + ": filesystem error on '" + path + "'. details: " + err.what());
            }
        }

        inline std::vector<std::string> getListOfFilesFromFolder(const std::string& path, const std::string& extension){
            std::vector<std::string> lst;
            try{
                for (auto& entry : fs::directory_iterator(path, fs::directory_options::skip_permission_denied))
                    if (entry.path().extension().string() == extension)
                        lst.push_back(entry.path().string()); // storing the address to this file
            } catch(const fs::filesystem_error& err){
                error("filesystem error on '" + path + "'. details: " + err.what());
            }
            return lst;
        }
    }

}

