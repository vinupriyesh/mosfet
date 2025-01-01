#ifndef GAME_DATA_SENDER_H
#define GAME_DATA_SENDER_H

#include "agent/control_center.h"
#include "agent/shuttle.h"

#include <string>

std::string exec(const char* cmd);
int send_game_data(Shuttle** shuttle, Shuttle** enemyShuttle, Relic** relic,
                   GameEnvConfig* gameEnvConfig, GameMap* gameMap, int port);

#endif // GAME_DATA_SENDER_H
