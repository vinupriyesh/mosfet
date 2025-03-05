#include <string>
#include <unordered_map>
#ifndef RESPAWN_TIMER_H
#define RESPAWN_TIMER_H

class RespawnRegistry {

    private:
        void log(const std::string& message);

        void logCurrentState();

        // Records = [unitId] -> stepId
        std::unordered_map<int, int> playerRespawnRecord;
        std::unordered_map<int, int> opponentRespawnRecord;

        // Records = [unitId] -> stepId
        std::unordered_map<int, int> playerDeathRecord;
        std::unordered_map<int, int> opponentDeathRecord;

        // Indexes = [stepId] -> unitId
        std::unordered_map<int, int> playerRespawnStepIndex;
        std::unordered_map<int, int> opponentRespawnStepIndex;

        int playerRespawnQueueSize;
        int opponentRespawnQueueSize;

        int getNextSpawnStep(int currentStep, int queueSize);
        int slotTheCurrentUnit(int currentStep, int& queueSize, std::unordered_map<int, int>& respawnRecord, std::unordered_map<int, int>& respawnStepIndex, std::unordered_map<int, int>& deathRecord, int unitId);

    public:
        int playerUnitRespawned = -1;
        int opponentUnitRespawned = -1;

        int pushPlayerUnit(int unitId, int stepOffset);
        int pushOpponentUnit(int unitId, int stepOffset);

        int getPlayerUnitSpawnStep(int unitId);
        int getOpponentUnitSpawnStep(int unitId);

        int getPlayerUnitThatCanSpawnAtStep(int step);
        int getOpponentUnitThatCanSpawnAtStep(int step);

        void reset();
        void step(int step);

        void printUpcomingRespawns(int currentStep);

        bool isOpponentShuttleAlive(int shuttleId, int stepId);

        RespawnRegistry(): playerRespawnQueueSize(0), opponentRespawnQueueSize(0) {};
};

#endif // RESPAWN_TIMER_H