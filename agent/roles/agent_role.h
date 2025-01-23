#ifndef AGENT_ROLE_H
#define AGENT_ROLE_H

#include "logger.h"
#include "metrics.h"
#include "agent/shuttle.h"
#include "agent/game_map.h"
#include "game_env_config.h"
#include "agent/pathing.h"
#include "agent/roles/communicator.h"

class AgentRole {
    protected:
        std::string roleClassName;
        bool unableToAct = false;
        GameMap& gameMap;
        Shuttle* shuttle;
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
        AgentRole(Shuttle *shuttle, GameMap& gamemap);
        void reset();
        virtual bool isRolePossible() = 0;
        virtual void iteratePlan(int planIteration, Communicator& communicator) = 0;
};

class ExplorerAgentRole : public AgentRole {
    public:
        using AgentRole::AgentRole;
};

class NavigatorAgentRole: public AgentRole {
    public:
        using AgentRole::AgentRole;
};

class RelicMinerAgentRole : public AgentRole {
    public:
        using AgentRole::AgentRole;
        RelicMinerAgentRole(Shuttle *shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class RelicMiningNavigatorAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::NavigatorAgentRole;
        RelicMiningNavigatorAgentRole(Shuttle *shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class HaloNodeExplorerAgentRole: public ExplorerAgentRole {
    private:
        std::mt19937 gen; // Mersenne Twister random number generator 
        std::uniform_int_distribution<> dis; // Uniform distribution

    public:
        using ExplorerAgentRole::ExplorerAgentRole;
        HaloNodeExplorerAgentRole(Shuttle *shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class HaloNodeNavigatorAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::NavigatorAgentRole;
        HaloNodeNavigatorAgentRole(Shuttle *shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class TrailblazerAgentRole: public ExplorerAgentRole {
    public:
        using ExplorerAgentRole::ExplorerAgentRole;
        TrailblazerAgentRole(Shuttle *shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class RandomAgentRole: public AgentRole {
    private:
        std::mt19937 gen; // Mersenne Twister random number generator 
        std::uniform_int_distribution<> dis; // Uniform distribution
    public:
        using AgentRole::AgentRole;
        RandomAgentRole(Shuttle *shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class DefenderAgentRole: public AgentRole {
    private:
        std::vector<Shuttle*> opponents;
    public:
        int attackingTileId = -1;

        using AgentRole::AgentRole;
        DefenderAgentRole(Shuttle *shuttle, GameMap& gamemap);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;

        bool chooseAttackingTile(Communicator &communicator);
};

#endif // AGENT_ROLE_H