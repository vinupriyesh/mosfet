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
        
        ControlCenter* cc;
        Shuttle* shuttle;
        Pathing* leastEnergyPathing;

        std::vector<int> bestPlan;

        void log(std::string message);
        
    public:

        void setLeastEnergyPathing(Pathing *leastEnergyPathing);
        Direction getDirectionTo(const GameTile &destinationTile);
        AgentRole(Shuttle *shuttle, ControlCenter *cc);
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

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class RelicMiningNavigatorAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::NavigatorAgentRole;

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class HaloNodeExplorerAgentRole: public ExplorerAgentRole {
    private:
        std::mt19937 gen; // Mersenne Twister random number generator 
        std::uniform_int_distribution<> dis; // Uniform distribution

    public:
        using ExplorerAgentRole::ExplorerAgentRole;

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class HaloNodeNavigatorAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::NavigatorAgentRole;

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class TrailblazerAgentRole: public ExplorerAgentRole {
    public:
        using ExplorerAgentRole::ExplorerAgentRole;

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

class RandomAgentRole: public AgentRole {
    private:
        std::mt19937 gen; // Mersenne Twister random number generator 
        std::uniform_int_distribution<> dis; // Uniform distribution
    public:
        using AgentRole::AgentRole;

        bool isRolePossible() override;
        void iteratePlan(int planIteration, Communicator& communicator) override;
};

#endif // AGENT_ROLE_H