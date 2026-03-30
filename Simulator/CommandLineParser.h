#pragma once
#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <experimental/filesystem>
// Implementations:
#include "../UserCommon/Utils.h"
#include "../UserCommon/Config.h"
// Namespaces:
using namespace UserCommon_211645361_000000000;
namespace fs = std::experimental::filesystem;

// this struct will contain the arguments given in the command line
struct Arguments {
    enum Mode {Comparative, Competitive} mode{}; // the mode in which the Simulator will run (Comparative or Competitive)
    // Comparative
    std::string gameMap;                 // 1 game map
    std::string gameManagersFolder;      // a folder of GameManagers
    std::vector<std::string> algorithms; // 2 algorithms
    // Competitive
    std::string gameMapsFolder;          // a folder of game maps
    std::string gameManager;             // 1 GameManager
    std::string algorithmsFolder;        // a folder of algorithms
    // Both:
    std::optional<int> numThreads;       // the number of requested threads (optional)
    std::optional<int> generateMaps;     // the number of maps to be generated (optional)
    bool verbose = false;                // GameManager will print game steps iff verbose == true
};

class CommandLineParser {
public:
    CommandLineParser(int argc, char* argv[]) {
        parseTokens(argc, argv);
        dispatchMode();
        validate();
    }

    [[nodiscard]]
    Arguments getArgs() const{
        return this->args;
    }

private:
    Arguments args;
    std::map<std::string, std::string> kv;  // holds key=value strings
    std::set<std::string> flags;            // any bare flags or modes that start with - and math either -comparative or -competition
    std::vector<std::string> unknowns;      // a list of tokens the parser doesn't support
    std::vector<std::string> duplicates;    // a list of tokens the parser has already seen

    // populate the kv map, flags and unknowns vectors and args
    void parseTokens(int argc, char* argv[]) {
        // argv[0] is the program name, so we start iterating at i=1
        for (int i = 1; i < argc; ++i) {
            std::string s = argv[i];  // we copy each argument into std::string s for easier handling
            if (s.rfind('-', 0) == 0) {  // we check if s is a flag -<flag name>
                if (s == "-comparative" || s == "-competition")  // the mode flag
                    this->flags.insert(s);
                else if (s == "-verbose") // the verbose flag
                    this->args.verbose = true;
                else // the program doesn't support this flag
                    this->unknowns.push_back(s);
            }else { // we check if s is of 'key=value' form
                auto eq = s.find('=');
                if (eq == std::string::npos) { // no '=', and not a flag -> unknown
                    this->unknowns.push_back(s);
                    continue;
                }
                // s = 'key=value'. split at the '='
                std::string key = Utils::trim(s.substr(0, eq));
                std::string val = Utils::trim(s.substr(eq+1));
                if(!this->kv[key].empty())
                    this->duplicates.push_back(key);
                this->kv[key] = val; // store it in the map
            }
        }
    }

    // setting the mode of the Simulator based on the command line (Comparative or Competitive)
    void dispatchMode() {
        bool isComparative = this->flags.count("-comparative");
        bool isCompetitive = this->flags.count("-competition");
        if (isComparative == isCompetitive) // both either 0 or 1 - not good
            Files::error("Must specify exactly one of -comparative or -competition.");
        this->args.mode = isComparative ? Arguments::Comparative : Arguments::Competitive; // setting the mode
    }

    void checkList(const std::vector<std::string>& list, const std::string& listName, bool isFatal){
        if (!list.empty()) {
            std::ostringstream os;
            os << listName + " arguments:";
            for (auto& arg : list)
                os << " " << arg;
            if(isFatal)
                Files::error(os.str()); // unrecoverable error
            else
                std::cerr << "\nWARNING: " << os.str() << "\n"; // recoverable error, just give a warning
        }
    }

    void validate() {
        // required fields:
        std::set<std::string> optional = {"num_threads", "verbose", "gen_maps"};
        std::set<std::string> required;
        if (args.mode == Arguments::Comparative)
            required = {"game_map", "game_managers_folder", "algorithm1", "algorithm2"};
        else  // competition
            required = {"game_maps_folder", "game_manager", "algorithms_folder"};
        // check unknowns:
        for (const auto& [key, _] : kv)
            if(!required.count(key) and !optional.count(key))
                unknowns.push_back(key);
        checkList(unknowns, "Unsupported", true);
        // check missing:
        std::vector<std::string> missing;
        for (auto& req : required)
            if (!kv.count(req))
                missing.push_back(req);
        checkList(missing, "Missing required", true);
        // check duplicates:
        checkList(duplicates, "Duplicates: (last value was taken into consideration) ", false);
        // populate args from kv:
        if (args.mode == Arguments::Comparative) {
            args.gameMap = kv["game_map"];
            args.gameManagersFolder = kv["game_managers_folder"];
            args.algorithms = { kv["algorithm1"], kv["algorithm2"] };
        } else {
            args.gameMapsFolder = kv["game_maps_folder"];
            args.gameManager = kv["game_manager"];
            args.algorithmsFolder = kv["algorithms_folder"];
        }
        // handle the threads field:
        if (kv.count("num_threads")) { // the user filled this field
            args.numThreads = std::stoi(kv["num_threads"]); // extracting the value
            if (args.numThreads < 1) // num thread should be at least 1
                Files::error("num_threads must be at least 1.");
        }
        if (kv.count("gen_maps")) { // the user filled this field
            args.generateMaps = std::stoi(kv["gen_maps"]); // extracting the value
            if (args.generateMaps <= 0 || args.generateMaps > Constants::maxMapsToGenerate) // num thread should be at least 1
                Files::error("the number of maps to be generated should be between 1 and 100");
        }
        // file/directory checks:
        if (args.mode == Arguments::Comparative) {
            // check the single game map:
            if(!args.generateMaps) // will be overridden anyway
                Files::checkFile(args.gameMap, "game map");
            // check that game_managers_folder exists and has at least 1 .so file:
            Files::checkDirectory(args.gameManagersFolder, "game managers folder", ".so");
            // check the two Algorithm .so files:
            for (auto& alg : args.algorithms)
                Files::checkFile(alg, "Algorithm");
        } else { // competition mode: one game_manager .so
            // check that game_maps_folder exists and has at least 1 .so file:
            if(!args.generateMaps) // will be overridden anyway
                Files::checkDirectory(args.gameMapsFolder, "game maps folder", ".txt");
            // check the single game_manager .so file:
            Files::checkFile(args.gameManager, "game manager");
            // check the algorithms' folder:
            Files::checkDirectory(args.algorithmsFolder, "Algorithm folder", ".so");
        }
    }
};

