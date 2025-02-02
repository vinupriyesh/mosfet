#ifndef AGENT_ROLE_H
#define AGENT_ROLE_H

#include "logger.h"
#include "metrics.h"
#include "agent/shuttle_data.h"
#include "agent/game_map.h"
#include "game_env_config.h"
#include "agent/pathing.h"
#include "agent/roles/communicator.h"
#include "agent/planning/jobs.h"

#include <random>

class AgentRole {
    protected:
        std::string roleClassName;
        bool unableToAct = false;
        GameMap& gameMap;
        ShuttleData& shuttle;
        Pathing* leastEnergyPathing;
        Pathing* leastEnergyPathingStopAtHaloTiles;
        Pathing* leastEnergyPathingStopAtVantagePoints;

        void log(std::string message);
        
    public:
        std::vector<int> bestPlan; 
        void setLeastEnergyPathing(Pathing *leastEnergyPathing);
        void setLeastEnergyPathingStopAtHaloTiles(Pathing *leastEnergyPathingStopAtHaloTiles);
        void setLeastEnergyPathingStopAtVantagePoints(Pathing *leastEnergyPathingStopAtVantagePoints);
        Direction getDirectionTo(const GameTile &destinationTile);
        std::tuple<int, int> getRelativePosition(const GameTile &destinationTile);
        AgentRole(ShuttleData& shuttle, GameMap& gameMap);
        void reset();
        virtual bool isRolePossible() = 0;
        virtual void iteratePlan(int planIteration, Communicator& communicator) = 0;

        virtual void surveyJobBoard(JobBoard& jobBoard) = 0;
};

class ExplorerAgentRole : public AgentRole {
    public:
        using AgentRole::AgentRole;
};

class NavigatorAgentRole: public AgentRole {
    protected:
        void surveyJob(JobBoard& jobBoard, Job *job);
    public:
        using AgentRole::AgentRole;
};

class RelicMinerAgentRole : public AgentRole {
    public:
        using AgentRole::AgentRole;
        RelicMinerAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;

        void surveyJobBoard(JobBoard& jobBoard) override;
};

class RelicMiningNavigatorAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::NavigatorAgentRole;
        RelicMiningNavigatorAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
        void surveyJobBoard(JobBoard &jobBoard) override;
};

class HaloNodeExplorerAgentRole: public ExplorerAgentRole {
    private:
        std::mt19937 gen; // Mersenne Twister random number generator 
        std::uniform_int_distribution<> dis; // Uniform distribution

    public:
        using ExplorerAgentRole::ExplorerAgentRole;
        HaloNodeExplorerAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
        void surveyJobBoard(JobBoard &jobBoard) override;
};

class HaloNodeNavigatorAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::NavigatorAgentRole;
        HaloNodeNavigatorAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
        void surveyJobBoard(JobBoard &jobBoard) override;
};

class TrailblazerAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::NavigatorAgentRole;
        TrailblazerAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;

        void surveyJobBoard(JobBoard& jobBoard) override;
};

class RandomAgentRole: public AgentRole {
    private:
        std::mt19937 gen; // Mersenne Twister random number generator 
        std::uniform_int_distribution<> dis; // Uniform distribution
    public:
        using AgentRole::AgentRole;
        RandomAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
        void surveyJobBoard(JobBoard &jobBoard);
};

class DefenderAgentRole: public AgentRole {
    private:
        std::vector<ShuttleData*> opponents;
    public:
        int attackingTileId = -1;

        using AgentRole::AgentRole;
        DefenderAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;

        bool chooseAttackingTile(Communicator &communicator);
        void surveyJobBoard(JobBoard &jobBoard);
};

#endif // AGENT_ROLE_H
