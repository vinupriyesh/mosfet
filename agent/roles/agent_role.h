#ifndef AGENT_ROLE_H
#define AGENT_ROLE_H

#include "logger.h"
#include "metrics.h"
#include "agent/shuttle.h"
#include "agent/pathing.h"

class AgentRole {
    protected:
        Shuttle* shuttle;
        Pathing* leastEnergyPathing;
        
    public:
        AgentRole(Shuttle* shuttle):  shuttle(shuttle) {};
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
};

class RelicMiningNavigatorAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::AgentRole;
};

class HaloNodeExplorerAgentRole: public ExplorerAgentRole {
    public:
        using ExplorerAgentRole::AgentRole;
};

class HaloNodeNavigator: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::AgentRole;
};

class TrailblazerAgentRole: public ExplorerAgentRole {
    public:
        using ExplorerAgentRole::AgentRole;
};

class RandomAgentRole: public AgentRole {
    public:
        using AgentRole::AgentRole;
};

#endif // AGENT_ROLE_H