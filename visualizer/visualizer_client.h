#ifndef GAME_DATA_SENDER_H
#define GAME_DATA_SENDER_H

#include "agent/game_map.h"
#include "agent/shuttle.h"
#include "config.h"

#include <string>

class VisualizerClient {
    private:
        int teamId;
        bool livePlayEnabled;
        bool recordingEnabled;
        int port;

        GameMap& gameMap;
        Shuttle** shuttles;
        Shuttle** opponentShuttles;
        std::map<int, Relic*>& relics;

        std::ofstream log_file;

        void log(std::string message);
        std::string exec(const char* cmd);
        std::string get_data();
        std::string upload_data(std::string data);

    public:
        VisualizerClient(GameMap& gameMap, Shuttle** shuttles, Shuttle** oppponentShuttles, std::map<int, Relic*>& relics);
        int send_game_data();
        ~VisualizerClient();
};

#endif // GAME_DATA_SENDER_H
