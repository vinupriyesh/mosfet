#include <string>
#include <unordered_map>
#ifndef RESPAWN_TIMER_H
#define RESPAWN_TIMER_H

class RespawnRegistry {

    private:
        void log(std::string message);

        std::unordered_map<int, int> playerUnitRespawnRecord;
        std::unordered_map<int, int> opponentUnitRespawnRecord;

        std::unordered_map<int, int> playerUnitRespawnIndex;
        std::unordered_map<int, int> opponentUnitRespawnIndex;

        int currentPlayerCooldownStep;
        int currentOpponentCooldownStep;

    public:
        int pushPlayerUnit(int unitId, int stepOffset);
        int pushOpponentUnit(int unitId, int stepOffset);

        int getPlayerUnitSpawnStep(int unitId);
        int getOpponentUnitSpawnStep(int unitId);

        int getPlayerUnitThatCanSpawnAtStep(int step);
        int getOpponentUnitThatCanSpawnAtStep(int step);

        void reset();

        void printUpcomingRespawns(int currentStep);

        RespawnRegistry(): currentPlayerCooldownStep(0), currentOpponentCooldownStep(0) {};
};

#endif // RESPAWN_TIMER_H