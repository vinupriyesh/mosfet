#include <random>

#include "shuttle.h"
#include <tuple>
#include <set>


void Shuttle::log(std::string message) {
    std::string visibilityMark = "";
    if (!shuttleData.visible) {
        visibilityMark = "~";
    }
    Logger::getInstance().log(visibilityMark + "Shuttle-" + std::to_string(shuttleData.id) + " -> " + message);
}

void Shuttle::updateUnitsData(std::vector<int> position, int energy, int timestep) {
    this->shuttleData.previousPosition = this->shuttleData.position;
    this->shuttleData.position = position;
    this->shuttleData.energy = energy;
    if (energy < 0) {
        this->shuttleData.ghost = true;
    } else {
        this->shuttleData.ghost = false;        
    }

    if (this->shuttleData.visible) {
        this->shuttleData.lastKnownEnergy = energy;
        this->shuttleData.lastEnergyUpdateTime = timestep;
    }
}

bool Shuttle::isGhost() {
    return this->shuttleData.ghost;
}

bool Shuttle::isVisible(){
    return this->shuttleData.visible;
}

void Shuttle::updateVisibility(bool isVisible) {
    this->shuttleData.visible = isVisible;
}

void Shuttle::computePath() {    
    if (!shuttleData.visible) {
        log("Retuning as shuttle is not visible");
        return;
    }

    log("Computing path");

    auto start = std::chrono::high_resolution_clock::now();

    if (leastEnergyPathing != nullptr) {
        delete leastEnergyPathing;
    }
    
    GameTile& startTile = gameMap.getTile(shuttleData.position[0], shuttleData.position[1]);    

    // Pathing expored tiles
    PathingConfig config = {};
    config.pathingHeuristics = LEAST_ENERGY;
    config.captureEverything();
    config.stopAtUnexploredTiles = false;
    config.doNotBumpIntoOpponentShuttles = true;

    leastEnergyPathing = new Pathing(gameMap, config);
    leastEnergyPathing->findAllPaths(startTile);    

    for (const auto& pair : agentRoles) {
        pair.second->setLeastEnergyPathing(leastEnergyPathing);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    Metrics::getInstance().add("pathing_duration", duration.count());
    log("pathing complete");
}

void Shuttle::surveyJobBoard(JobBoard &jobBoard) {
    if (!shuttleData.visible) {
        return;
    }

    log("Surveying job board");

    computePath();

    for (const auto& pair : agentRoles) {
        pair.second->surveyJobBoard(jobBoard);
    }

}

ShuttleData &Shuttle::getShuttleData() {
    return shuttleData;
}

Shuttle::Shuttle(int id, ShuttleType type, GameMap& gameMap)
        :shuttleData(ShuttleData(id, type)), gameMap(gameMap) {

    leastEnergyPathing = nullptr;

    //Populating Agent role classes

    // Explorers
    agentRoles["HaloNodeExplorerAgentRole"] = new HaloNodeExplorerAgentRole(shuttleData, gameMap);
    agentRoles["TrailblazerAgentRole"] = new TrailblazerAgentRole(shuttleData, gameMap);

    // Navigators
    agentRoles["RelicMiningNavigatorAgentRole"] = new RelicMiningNavigatorAgentRole(shuttleData, gameMap);
    agentRoles["HaloNodeNavigatorAgentRole"] = new HaloNodeNavigatorAgentRole(shuttleData, gameMap);

    // Miners
    agentRoles["RelicMinerAgentRole"] = new RelicMinerAgentRole(shuttleData, gameMap);

    // Random
    agentRoles["RandomAgentRole"] = new RandomAgentRole(shuttleData, gameMap);

    // Sappers
    agentRoles["DefenderAgentRole"] = new DefenderAgentRole(shuttleData, gameMap);

    // log("Shuttle instance created");
}

bool Shuttle::isTileUnvisited(Direction direction) {
    GameTile& shuttleTile = gameMap.getTile(shuttleData.position[0], shuttleData.position[1]);

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
    
    if (!shuttleData.visible) {
        return {0, 0, 0};
    }

    if (bestPlan.size() == 0) {
        log("Unable to prepare a plan");
        // std::cerr<<"Unable to prepare a plan"<<std::endl;
        return {0, 0, 0};
    }

    return bestPlan;
}

Shuttle::~Shuttle() {
    for (const auto& pair : agentRoles) {
        delete pair.second;
    }
    agentRoles.clear();
}
