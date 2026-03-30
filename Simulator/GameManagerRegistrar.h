#pragma once
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cassert>
// Interfaces:
#include "../common/BattleInfo.h"
#include "../common/ActionRequest.h"
#include "../common/TankAlgorithm.h"
#include "../common/SatelliteView.h"
#include "../common/Player.h"
#include "../common/GameResult.h"
#include "../common/AbstractGameManager.h"   // for GameManagerFactory
#include "../common/GameManagerRegistration.h"

/// Holds one entry per loaded .so, each with its GameManagerFactory
class GameManagerRegistrar {
    class GameManagerFactories {
        std::string so_name;
        GameManagerFactory gameManagerFactory;

    public:
        GameManagerFactories(const std::string& name): so_name(name) {}
        // get the .so name:
        [[nodiscard]]
        const std::string& name() const {
            return this->so_name;
        }

        // ------------------------------------------- GameManager ------------------------------------------------ //
        // check if the GameManager factory has been set already:
        [[nodiscard]]
        bool hasGameManagerFactory() const {
            return this->gameManagerFactory != nullptr;
        }

        // if the GameManager factory hasn't been set yet, set it to be factory:
        void setGameManagerFactory(GameManagerFactory&& factory) {
            assert(!hasGameManagerFactory());
            // it hasn't, we set it for the first time:
            this->gameManagerFactory = std::move(factory);
        }

        // get the GameManager factory:
        [[nodiscard]]
        const GameManagerFactory& getGameManagerFactory() const {
            return this->gameManagerFactory;
        }

        // create a GameManager instance with the factory:
        [[nodiscard]]
        std::unique_ptr<AbstractGameManager> createGameManager(bool verbose) const {
            return this->gameManagerFactory(verbose); // a new instance of GameManager
        }

    };
    std::vector<GameManagerFactories> gameManagers;
    static GameManagerRegistrar registrar;

public:
    static GameManagerRegistrar& getGameManagerRegistrar();

    void createGameManagerFactoryEntry(const std::string& name) {
        this->gameManagers.emplace_back(name);
    }

    void addGameManagerFactoryToLastEntry(GameManagerFactory&& factory) {
        this->gameManagers.back().setGameManagerFactory(std::move(factory));
    }

    struct BadRegistrationException {
        std::string name;
        bool hasName;
        bool hasFactory;
    };

    void validateLastRegistration() {
        auto& last = gameManagers.back();
        bool okName    = !last.name().empty();
        bool okFactory = last.hasGameManagerFactory();
        if (!okName || !okFactory) {
            throw BadRegistrationException{
                    .name       = last.name(),
                    .hasName    = okName,
                    .hasFactory = okFactory
            };
        }
    }

    void removeLast() {
        this->gameManagers.pop_back();
    }

    [[nodiscard]]
    auto begin() const {
        return this->gameManagers.begin();
    }

    [[nodiscard]]
    auto end()   const {
        return this->gameManagers.end();
    }

    [[nodiscard]]
    size_t count() const {
        return this->gameManagers.size();
    }

    void clear() {
        this->gameManagers.clear();
    }

    bool isDuplicate(const std::string& name){
        for(const auto& gm : this->gameManagers)
            if(gm.name() == name) return true;

        return false;
    }
};
