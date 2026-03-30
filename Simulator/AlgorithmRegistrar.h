#pragma once
#include <string>
#include <vector>
#include <functional>
#include <cassert>
#include <memory>
// Interfaces:
#include "../common/SatelliteView.h"
#include "../common/BattleInfo.h"
#include "../common/ActionRequest.h"
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"
#include "../common/TankAlgorithmRegistration.h"
#include "../common/PlayerRegistration.h"

class AlgorithmRegistrar {
    // This singleton holds one entry per loaded .so, each entry bundling:
    class AlgorithmAndPlayerFactories {
        std::string so_name;                        // the .so file name. e.g. "Algorithm_211645361_000000000"
        TankAlgorithmFactory tankAlgorithmFactory;  // factory to make TankAlgorithm instances
        PlayerFactory playerFactory;                // factory to make Player instances

    public:
        explicit AlgorithmAndPlayerFactories(const std::string& name): so_name(name) {}
        // get the .so name:
        [[nodiscard]]
        const std::string& name() const {
            return this->so_name;
        }

        // ------------------------------------------- TankAlgorithm ------------------------------------------------ //
        // check if the TankAlgorithm factory has been set already:
        [[nodiscard]]
        bool hasTankAlgorithmFactory() const {
            return this->tankAlgorithmFactory != nullptr;
        }

        // if the TankAlgorithm factory hasn't been set yet, set it to be factory:
        void setTankAlgorithmFactory(TankAlgorithmFactory&& factory) {
            assert(!hasTankAlgorithmFactory());
            // it hasn't, we set it for the first time:
            this->tankAlgorithmFactory = std::move(factory);
        }

        // get the TankAlgorithm factory:
        [[nodiscard]]
        const TankAlgorithmFactory& getTankAlgorithmFactory() const {
            return this->tankAlgorithmFactory;
        }

        // create a TankAlgorithm instance with the factory:
        [[nodiscard]]
        std::unique_ptr<TankAlgorithm> createTankAlgorithm(int player_index, int tank_index) const {
            return this->tankAlgorithmFactory(player_index, tank_index); // a new instance of TankAlgorithm
        }
        // ------------------------------------------------ Player ------------------------------------------------- //
        // checking if it has a player factory:
        [[nodiscard]]
        bool hasPlayerFactory() const {
            return this->playerFactory != nullptr;
        }

        // if the Player factory hasn't been set yet, set it to be factory:
        void setPlayerFactory(PlayerFactory&& factory) {
            assert(!hasPlayerFactory());
            this->playerFactory = std::move(factory);
        }

        // get the Player factory:
        [[nodiscard]]
        const PlayerFactory & getPlayerFactory() const {
            return this->playerFactory;
        }

        // create a Player instance using the factory:
        [[nodiscard]]
        std::unique_ptr<Player> createPlayer(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells) const {
            return this->playerFactory(player_index, x, y, max_steps, num_shells); // a new instance of Player
        }

    };
    // we're done with the definition of the AlgorithmAndPlayerFactories class.
    // now define attributes for the AlgorithmRegistrar:
    std::vector<AlgorithmAndPlayerFactories> algorithms; // a list of the AlgorithmAndPlayerFactories instances
    static AlgorithmRegistrar registrar;  // the one global instance

public:
    static AlgorithmRegistrar& getAlgorithmRegistrar();
    // create a new entry in the list, currently without a factory:
    void createAlgorithmFactoryEntry(const std::string& name) {
        this->algorithms.emplace_back(name);
    }

    // Invoked by each plugin’s REGISTER_PLAYER macro,
    // giving us a std::function that can build one of its Player objects. Internally it does:
    void addPlayerFactoryToLastEntry(PlayerFactory&& factory) {
        algorithms.back().setPlayerFactory(std::move(factory));
    }

    // Invoked by each plugin’s REGISTER_TANK_ALGORITHM macro,
    // giving us a std::function that can build one of its TankAlgorithm objects. Internally it does:
    void addTankAlgorithmFactoryToLastEntry(TankAlgorithmFactory&& factory) {
        algorithms.back().setTankAlgorithmFactory(std::move(factory));
    }

    struct BadRegistrationException {
        std::string name;
        bool hasName, hasPlayerFactory, hasTankAlgorithmFactory;
    };
    /*
     * After both registrations, we check that:
     *      we actually pushed an entry (hasName)
     *      we got a tank‐factory
     *      we got a player‐factory
     * If any is missing, we throw BadRegistrationException so the Simulator can log the bad plugin
     * and removeLast() that entry.
     */
    void validateLastRegistration() {
        const auto& last = algorithms.back();
        bool hasName = (!last.name().empty());
        if(!hasName || !last.hasPlayerFactory() || !last.hasTankAlgorithmFactory() ) {
            throw BadRegistrationException{
                    .name = last.name(),
                    .hasName = hasName,
                    .hasPlayerFactory = last.hasPlayerFactory(),
                    .hasTankAlgorithmFactory = last.hasTankAlgorithmFactory()
            };
        }
    }

    // remove the last entry from the algorithms list:
    void removeLast() {
        this->algorithms.pop_back();
    }

    // return the first entry of the algorithms list:
    [[nodiscard]]
    auto begin() const {
        return this->algorithms.begin();
    }

    // return the last entry of the algorithms list:
    [[nodiscard]]
    auto end() const {
        return this->algorithms.end();
    }

    // return the number of entries of the algorithms list:
    [[nodiscard]]
    size_t count() const {
        return this->algorithms.size();
    }

    // remove all entries on the algorithms list:
    void clear()  {
        this->algorithms.clear();
    }

    bool isDuplicate(const std::string& name){
       for(const auto& algo : this->algorithms)
           if(algo.name() == name) return true;

       return false;
    }
};
