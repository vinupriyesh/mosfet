#include "control_center.h"
#include <iostream>
#include <string>
#include "logger.h"

void ControlCenter::log(std::string message) {
    Logger::getInstance().log("ControlCenter -> " + message);
}

void ControlCenter::update(GameState& gameState) {

    if (shuttles == nullptr) {
        // Initialize the control center
        log("Creating the shuttle instances");
        playerName = gameState.player;
        Logger::getInstance().setPlayerName(playerName);
        N = gameState.info.envCfg["max_units"];
        shuttles = new Shuttle*[N];
        for (int i = 0; i < N; ++i) {
            shuttles[i] = new Shuttle();
        }
    }    
}

ControlCenter::ControlCenter() {
    // Empty constructor
    log("Starting the game");
}

ControlCenter::~ControlCenter() {
    for (int i = 0; i < N; ++i) {
        delete shuttles[i];
    }
    delete[] shuttles;
}

std::vector<std::vector<int>> ControlCenter::act() {
    log("inside act for shuttle");
    std::vector<std::vector<int>> results;
    for (int i = 0; i < N; ++i) {
        results.push_back(shuttles[i]->act());
    }
    return results;
}
