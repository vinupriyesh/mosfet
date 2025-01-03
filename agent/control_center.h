#ifndef CONTROLCENTER_H
#define CONTROLCENTER_H

#include "parser.h"
#include "agent/relic.h"
#include "logger.h"
#include "metrics.h"
#include "agent/game_map.h"
#include "agent/shuttle.h"
#include "datastructures/constraint_set.h"
#include <vector>
#include <string>

class Shuttle; //Forward declaration


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

    // Constructor
    GameEnvConfig(GameState& gameState) {
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
    relicCount = gameState.obs.relicNodesMask.size();
    }
};

class ControlCenter {
private:
    ConstraintSet* haloConstraints;       

    // dynamic objects
    Shuttle** shuttles; 
    Shuttle** opponentShuttles;
    Relic** relics;    

    // private methods
    void log(std::string message);
    void init(GameState &gameState);

public: 
    int teamPoints;
    int opponentTeamPoints;
    int teamPointsDelta;
    int opponentTeamPointsDelta;
    int currentStep;
    int currentMatchStep;
    int remainingOverageTime;

    bool allRelicsFound = false;
    bool allTilesExplored = false;
    bool allTilesVisited = false;

    int relicsFound = 0;
    int tilesExplored = 0;
    int tilesVisited = 0;

    GameEnvConfig* gameEnvConfig; 
    GameMap* gameMap;

    ControlCenter();
    ~ControlCenter();
    void update(GameState &gameState);
    std::vector<std::vector<int>> act();
};

#endif // CONTROLCENTER_H
