#ifndef AGENT_ROLE_H
#define AGENT_ROLE_H

#include "logger.h"
#include "metrics.h"
#include "agent/shuttle.h"
#include "agent/control_center.h"
#include "agent/pathing.h"
#include "agent/roles/communicator.h"

class ControlCenter; // Forward declaration

class AgentRole {
    protected:
        std::string roleClassName;
        bool unableToAct = false;
        ControlCenter* cc;
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
        AgentRole(Shuttle *shuttle, ControlCenter *cc);
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
        RelicMinerAgentRole(Shuttle *shuttle, ControlCenter *cc);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class RelicMiningNavigatorAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::NavigatorAgentRole;
        RelicMiningNavigatorAgentRole(Shuttle *shuttle, ControlCenter *cc);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class HaloNodeExplorerAgentRole: public ExplorerAgentRole {
    private:
        std::mt19937 gen; // Mersenne Twister random number generator 
        std::uniform_int_distribution<> dis; // Uniform distribution

    public:
        using ExplorerAgentRole::ExplorerAgentRole;
        HaloNodeExplorerAgentRole(Shuttle *shuttle, ControlCenter *cc);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class HaloNodeNavigatorAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::NavigatorAgentRole;
        HaloNodeNavigatorAgentRole(Shuttle *shuttle, ControlCenter *cc);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class TrailblazerAgentRole: public ExplorerAgentRole {
    public:
        using ExplorerAgentRole::ExplorerAgentRole;
        TrailblazerAgentRole(Shuttle *shuttle, ControlCenter *cc);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class RandomAgentRole: public AgentRole {
    private:
        std::mt19937 gen; // Mersenne Twister random number generator 
        std::uniform_int_distribution<> dis; // Uniform distribution
    public:
        using AgentRole::AgentRole;
        RandomAgentRole(Shuttle *shuttle, ControlCenter *cc);

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

#endif // AGENT_ROLE_H