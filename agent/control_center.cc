#include "control_center.h"
#include <iostream>
#include <string>
#include "logger.h"
#include "relic.h"
#include "visualizer/visualizer_client.h"

void ControlCenter::log(std::string message) {
    Logger::getInstance().log("ControlCenter -> " + message);
}

/**
 * Initialize the control center object.  This is invoked during the first update.
 */
void ControlCenter::init(GameState& gameState) {
    log("Initializing the control center");
    
    gameEnvConfig = new GameEnvConfig(gameState);

    // Shuttles (player and enemy).  This is created once and reused.
    shuttles = new Shuttle*[gameEnvConfig->maxUnits];
    enemyShuttles = new Shuttle*[gameEnvConfig->maxUnits];
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        shuttles[i] = new Shuttle(i, ShuttleType::player, this);
        enemyShuttles[i] = new Shuttle(i, ShuttleType::enemy, this);
    }

    relics = new Relic*[gameEnvConfig->relicCount];
    for (int i = 0; i < gameEnvConfig->relicCount; ++i) {
        relics[i] = new Relic(i);     
    }
}

/**
 * Update the control center object.  This is invoked during each update.
 */
void ControlCenter::update(GameState& gameState) {

    if (shuttles == nullptr) {
        init(gameState);
    }

    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        shuttles[i]->updateUnitsData(gameState.obs.units.position[gameEnvConfig->teamId][i],
                                     gameState.obs.units.energy[gameEnvConfig->teamId][i]);
        enemyShuttles[i]->updateUnitsData(gameState.obs.units.position[gameEnvConfig->enemyTeamId][i],
                                          gameState.obs.units.energy[gameEnvConfig->enemyTeamId][i]);
                                          
        shuttles[i]->updateVisbility(gameState.obs.unitsMask[gameEnvConfig->teamId][i]);
        enemyShuttles[i]->updateVisbility(gameState.obs.unitsMask[gameEnvConfig->enemyTeamId][i]);        
    }

    for (int i = 0; i < gameEnvConfig->relicCount; ++i) {
        relics[i]->updateRelicData(gameState.obs.relicNodes[i], gameState.obs.relicNodesMask[i]);
    }
}

ControlCenter::ControlCenter() {
    // Empty constructor
    log("Starting the game");
}

ControlCenter::~ControlCenter() {
    delete gameEnvConfig;
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        delete shuttles[i];
    }
    delete[] shuttles;
}


std::vector<std::vector<int>> ControlCenter::act() {
    log("inside act for shuttle");
    std::vector<std::vector<int>> results;
    for (int i = 0; i < gameEnvConfig->maxUnits; ++i) {
        results.push_back(shuttles[i]->act());
    }
    if (Logger::getInstance().isDebugEnabled()) {
        send_game_data();
    }
    std::cerr<< "test cc" << std::endl;
    return results;
}
