#ifndef GAME_ENV_CONFIG_H
#define GAME_ENV_CONFIG_H

#include "parser.h"
#include "logger.h"
#include "metrics.h"

struct GameEnvConfig {
    std::string playerName;
    int maxUnits;
    int teamId;
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
    int originX, originY;
    int opponentOriginX, opponentOriginY;
    
    void init(GameState& gameState) {
        // Player ID and team ID
        playerName = gameState.player;
        Logger::getInstance().setPlayerName(playerName);
        Metrics::getInstance().setPlayerName(playerName);

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

        if (playerName == "player_0") {
            teamId = 0;
            opponentTeamId = 1;
            originX = 0;
            originY = 0;
            opponentOriginX = mapWidth - 1;
            opponentOriginY = mapHeight - 1;
        } else {
            teamId = 1;
            opponentTeamId = 0;
            originX = mapWidth - 1;
            originY = mapHeight - 1;
            opponentOriginX = 0;
            opponentOriginY = 0;
        }        

        initialized = true;
    }

    static GameEnvConfig& getInstance() {
        static GameEnvConfig instance;
        return instance;
    }
};

#endif //GAME_ENV_CONFIG_H