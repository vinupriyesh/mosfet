#ifndef GAME_DATA_SENDER_H
#define GAME_DATA_SENDER_H

#include "agent/game_map.h"
#include "agent/opponent_tracker.h"
#include "agent/shuttle.h"

#include <string>

class VisualizerClient {
    private:
        size_t currentInsertIndex = 0;

        int teamId;
        bool livePlayEnabled;
        bool recordingEnabled;
        int port;

        GameMap& gameMap;
        Shuttle** shuttles;
        Shuttle** opponentShuttles;
        std::map<int, Relic*>& relics;
        OpponentTracker& opponentTracker;

        std::ofstream log_file;

        static void log(const std::string& message);
        std::string exec(const char* cmd);
        std::string getData(std::vector<std::vector<int>> actions);
        std::string uploadData(std::string data);

    public:
        VisualizerClient(GameMap& gameMap, Shuttle** shuttles, Shuttle** oppponentShuttles, std::map<int, Relic*>& relics, OpponentTracker& opponentTracker);
        int sendGameData(std::vector<std::vector<int>> results);
        ~VisualizerClient();
};

#endif // GAME_DATA_SENDER_H
