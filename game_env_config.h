#ifndef GAME_ENV_CONFIG_H
#define GAME_ENV_CONFIG_H

#include <iostream>

#include "parser.h"
#include "logger.h"
#include "metrics.h"

struct GameEnvConfig {
    std::string playerName;
    int maxUnits;
    int teamId;
    int relicCount;
    int opponentTeamId;    
    int unitMoveCost;
    int unitSapCost;
    int unitSapRange;
    int unitSensorRange;
    int matchCountPerEpisode;
    int maxStepsInMatch;
    int mapHeight;
    int mapWidth;
    bool initialized = false;
    int nebulaTileEnergyReduction;
    
    void init(GameState& gameState) {
        // Player ID and team ID
        playerName = gameState.player;
        Logger::getInstance().setPlayerName(playerName);
        Metrics::getInstance().setPlayerName(playerName);

        if (playerName == "player_0") {
            teamId = 0;
            opponentTeamId = 1;
        } else {
            teamId = 1;
            opponentTeamId = 0;
        }

        // Game info
        matchCountPerEpisode = gameState.info.envCfg["match_count_per_episode"];
        maxStepsInMatch = gameState.info.envCfg["max_steps_in_match"];
        mapHeight = gameState.info.envCfg["map_height"];
        mapWidth = gameState.info.envCfg["map_width"];
        maxUnits = gameState.info.envCfg["max_units"];
        unitMoveCost = gameState.info.envCfg["unit_move_cost"];
        unitSapCost = gameState.info.envCfg["unit_sap_cost"];
        unitSapRange = gameState.info.envCfg["unit_sap_range"];
        unitSensorRange = gameState.info.envCfg["unit_sensor_range"];

        nebulaTileEnergyReduction = 10; //TODO:  This has to be identified dynamically, lets start with 10!
        relicCount = gameState.obs.relicNodesMask.size();

        initialized = true;
    }

    static GameEnvConfig& getInstance() {
        static GameEnvConfig instance;
        return instance;
    }
};

#endif //GAME_ENV_CONFIG_H