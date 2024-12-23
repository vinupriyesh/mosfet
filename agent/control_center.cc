#include "control_center.h"
#include <iostream>
#include <string>
#include "logger.h"
#include "relic.h"

void ControlCenter::log(std::string message) {
    Logger::getInstance().log("ControlCenter -> " + message);
}

/**
 * Initialize the control center object.  This is invoked during the first update.
 */
void ControlCenter::init(GameState& gameState) {
    log("Initializing the control center");
    
    // Player ID and team ID
    playerName = gameState.player;
    Logger::getInstance().setPlayerName(playerName);

    if (playerName == "player_0") {
        teamId = 0;
        enemyTeamId = 1;
    } else {
        teamId = 1;
        enemyTeamId = 0;
    }

    // Max units in the game
    maxUnits = gameState.info.envCfg["max_units"];

    // Shuttles (player and enemy).  This is created once and reused.
    shuttles = new Shuttle*[maxUnits];
    enemyShuttles = new Shuttle*[maxUnits];
    for (int i = 0; i < maxUnits; ++i) {
        shuttles[i] = new Shuttle(i, ShuttleType::player, this);
        enemyShuttles[i] = new Shuttle(i, ShuttleType::enemy, this);
    }

    relicCount = gameState.obs.relicNodesMask.size();

    relics = new Relic*[relicCount];
    for (int i = 0; i < relicCount; ++i) {
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

    for (int i = 0; i < maxUnits; ++i) {
        shuttles[i]->updateUnitsData(gameState.obs.units.position[teamId][i],
                                     gameState.obs.units.energy[teamId][i]);
        enemyShuttles[i]->updateUnitsData(gameState.obs.units.position[enemyTeamId][i],
                                          gameState.obs.units.energy[enemyTeamId][i]);
                                          
        shuttles[i]->updateVisbility(gameState.obs.unitsMask[teamId][i]);
        enemyShuttles[i]->updateVisbility(gameState.obs.unitsMask[enemyTeamId][i]);        
    }

    for (int i = 0; i < relicCount; ++i) {
        relics[i]->updateRelicData(gameState.obs.relicNodes[i], gameState.obs.relicNodesMask[i]);
    }
}

ControlCenter::ControlCenter() {
    // Empty constructor
    log("Starting the game");
}

ControlCenter::~ControlCenter() {
    for (int i = 0; i < maxUnits; ++i) {
        delete shuttles[i];
    }
    delete[] shuttles;
}

std::vector<std::vector<int>> ControlCenter::act() {
    log("inside act for shuttle");
    std::vector<std::vector<int>> results;
    for (int i = 0; i < maxUnits; ++i) {
        results.push_back(shuttles[i]->act());
    }
    return results;
}
