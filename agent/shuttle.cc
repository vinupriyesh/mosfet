#include <random>

#include "shuttle.h"
#include <tuple>
#include <set>


void Shuttle::log(std::string message) {
    std::string visibilityMark = "";
    if (!this->visible) {
        visibilityMark = "~";
    }
    Logger::getInstance().log(visibilityMark + "Shuttle-" + std::to_string(this->id) + " -> " + message);
}

void Shuttle::updateUnitsData(std::vector<int> position, int energy) {
    this->previousPosition = this->position;
    this->position = position;
    this->energy = energy;
    if (energy < 0) {
        this->ghost = true;
    } else {
        this->ghost = false;
    }
}

bool Shuttle::isGhost() {
    return this->ghost;
}

void Shuttle::updateVisibility(bool isVisible) {
    this->visible = isVisible;
}

int Shuttle::getX() {
    return position[0];
}

int Shuttle::getY() {
    return position[1];
}

GameTile *Shuttle::getTileAtPosition() {
    if (gameMap.isValidTile(position[0], position[1])) {
        return &gameMap.getTile(position[0], position[1]);
    } else {
        return nullptr;
    }
}

void Shuttle::computePath() {
    log("Computing path");
    if (!visible) {
        log("Retuning as shuttle is not visible");
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();

    if (leastEnergyPathing != nullptr) {
        delete leastEnergyPathing;
    }

    if (leastEnergyPathingStopAtHaloTiles != nullptr) {
        delete leastEnergyPathingStopAtHaloTiles;
    }

    if (leastEnergyPathingStopAtVantagePoints != nullptr) {
        delete leastEnergyPathingStopAtVantagePoints;
    }

    log("Staring to pathing");
    GameTile& startTile = gameMap.getTile(position[0], position[1]);
    log("Staring tile " + startTile.toString());

    // Pathing expored tiles
    PathingConfig config = {};
    config.pathingHeuristics = LEAST_ENERGY;
    config.captureEverything();
    config.stopAtUnexploredTiles = true;
    config.doNotBumpIntoOpponentShuttles = true;

    leastEnergyPathing = new Pathing(gameMap, config);
    leastEnergyPathing->findAllPaths(startTile);
    log("First path complete");

    // Pathing halo tiles
    PathingConfig config2 = {};
    config2.pathingHeuristics = LEAST_ENERGY;
    config2.captureEverything();
    config2.stopAtHaloTiles= true;
    config2.doNotBumpIntoOpponentShuttles = true;

    leastEnergyPathingStopAtHaloTiles = new Pathing(gameMap, config2);
    leastEnergyPathingStopAtHaloTiles->findAllPaths(startTile);
    log("Second path complete");

    // Pathing capture vantage points
    PathingConfig config3 = {};
    config3.pathingHeuristics = LEAST_ENERGY;
    config3.captureEverything();
    config3.stopAtVantagePointTiles= true;
    config3.doNotBumpIntoOpponentShuttles = true;
    log("Third path complete");

    leastEnergyPathingStopAtVantagePoints = new Pathing(gameMap, config3);
    leastEnergyPathingStopAtVantagePoints->findAllPaths(startTile);

    for (const auto& pair : agentRoles) {
        pair.second->setLeastEnergyPathing(leastEnergyPathing);
        pair.second->setLeastEnergyPathingStopAtHaloTiles(leastEnergyPathingStopAtHaloTiles);
        pair.second->setLeastEnergyPathingStopAtVantagePoints(leastEnergyPathingStopAtVantagePoints);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    Metrics::getInstance().add("pathing_duration", duration.count());
    log("pathing complete");
}

void Shuttle::iteratePlan(int planIteration, Communicator &communicator) {

    if (!visible) {
        return;
    }    

    log("Iterating plan - " + std::to_string(planIteration));
    for (const auto& pair : agentRoles) {
        // log("Checking role possibility " + pair.first);
        pair.second->reset();
        if (pair.second->isRolePossible()) {
            // log("Role possible " + pair.first);
            pair.second->iteratePlan(planIteration, communicator);
        }
    }
    log("Going to decide the best role for this shuttle");

    //TODO:  below is a temporary code, this will go to the planner. Also need not have casted anything
    activeRole = nullptr;
    HaloNodeExplorerAgentRole* haloNodeExplorer = dynamic_cast<HaloNodeExplorerAgentRole*>(agentRoles["HaloNodeExplorerAgentRole"]);
    HaloNodeNavigatorAgentRole* haloNodeNavigator = dynamic_cast<HaloNodeNavigatorAgentRole*>(agentRoles["HaloNodeNavigatorAgentRole"]);
    RandomAgentRole* randomAgent = dynamic_cast<RandomAgentRole*>(agentRoles["RandomAgentRole"]);
    RelicMinerAgentRole* relicMiner = dynamic_cast<RelicMinerAgentRole*>(agentRoles["RelicMinerAgentRole"]);
    RelicMiningNavigatorAgentRole* relicMiningNavigator = dynamic_cast<RelicMiningNavigatorAgentRole*>(agentRoles["RelicMiningNavigatorAgentRole"]);
    TrailblazerAgentRole* trailblazer = dynamic_cast<TrailblazerAgentRole*>(agentRoles["TrailblazerAgentRole"]);
    DefenderAgentRole* defender = dynamic_cast<DefenderAgentRole*>(agentRoles["DefenderAgentRole"]);    

    if (defender->isRolePossible() && defender->chooseAttackingTile(communicator)) {
        activeRole = defender;
        log("Defender it is");
        Metrics::getInstance().add("defender", 1);
    } else if (relicMiner->isRolePossible()) {
        log("relicMiner it is");
        activeRole = relicMiner;
        Metrics::getInstance().add("relicMiner", 1);
    } else if(relicMiningNavigator->isRolePossible()) {
        activeRole = relicMiningNavigator;
        log("relicMiningNavirgator it is");
        Metrics::getInstance().add("relicMiningNavigator", 1);
    } else if(haloNodeExplorer->isRolePossible()) {
        activeRole = haloNodeExplorer;
        log("haloNodeExplorer it is");
        Metrics::getInstance().add("haloNodeExplorer", 1);
    } else if(haloNodeNavigator->isRolePossible()) {
        activeRole = haloNodeNavigator;
        log("haloNodeNavigator it is");
        Metrics::getInstance().add("haloNodeNavigator", 1);
    } else if(trailblazer->isRolePossible()) {
        activeRole = trailblazer;
        log("Trailblazer it is");
        Metrics::getInstance().add("trailblazer", 1);
    } else {
        activeRole = randomAgent;
        log("Random agent it is");
        Metrics::getInstance().add("randomAgent", 1);
    }
    
    // log("decided the role");
}

Shuttle::Shuttle(int id, ShuttleType type, GameMap& gameMap) : id(id), type(type), gameMap(gameMap) {
    this->visible = false;
    this->ghost = false;

    leastEnergyPathing = nullptr;

    //Populating Agent role classes

    // log("Going to create explorer roles");
    // Explorers
    agentRoles["HaloNodeExplorerAgentRole"] = new HaloNodeExplorerAgentRole(this, gameMap);
    agentRoles["TrailblazerAgentRole"] = new TrailblazerAgentRole(this, gameMap);

    // Navigators
    agentRoles["RelicMiningNavigatorAgentRole"] = new RelicMiningNavigatorAgentRole(this, gameMap);
    agentRoles["HaloNodeNavigatorAgentRole"] = new HaloNodeNavigatorAgentRole(this, gameMap);

    // Miners
    agentRoles["RelicMinerAgentRole"] = new RelicMinerAgentRole(this, gameMap);

    // Random
    agentRoles["RandomAgentRole"] = new RandomAgentRole(this, gameMap);

    // Sappers
    agentRoles["DefenderAgentRole"] = new DefenderAgentRole(this, gameMap);

    // log("Shuttle instance created");
}


bool Shuttle::isTileUnvisited(Direction direction) {
    GameTile& shuttleTile = gameMap.getTile(position[0], position[1]);

    std::tuple<bool, GameTile&> result = gameMap.isMovable(shuttleTile, direction);
    bool movable = std::get<0>(result);    
    GameTile& toTile = std::get<1>(result);
    
    if (movable) {
        log("Tile (" + std::to_string(toTile.x) + ", " + std::to_string(toTile.y) + ") is visited? " + std::to_string(toTile.isVisited()));
    } else {
        log("Tile is not movable - (" + std::to_string(toTile.x) + ", " + std::to_string(toTile.y) + ") in direction " + std::to_string(direction));
    }

    return movable && !toTile.isVisited();    
}

std::vector<int> Shuttle::act() {
    
    if (!visible) {
        //TODO: Check if we can still move invisible shuttle (If it is inside Nebula!)
        return {0, 0, 0};
    }

    log("Inside act2 -> " + std::to_string(activeRole == nullptr));
    return activeRole->bestPlan;
}

bool Shuttle::isRandomAction() {
    return activeRole != nullptr && (dynamic_cast<RandomAgentRole*>(activeRole) != nullptr || dynamic_cast<HaloNodeExplorerAgentRole*>(activeRole) != nullptr);
}

Shuttle::~Shuttle() {
    for (const auto& pair : agentRoles) {
        delete pair.second;
    }
    agentRoles.clear();
}
