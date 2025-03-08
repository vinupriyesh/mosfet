#ifndef AGENT_ROLE_H
#define AGENT_ROLE_H

#include "agent/shuttle_data.h"
#include "agent/game_map.h"
#include "agent/pathing.h"
#include "agent/roles/communicator.h"
#include "agent/planning/jobs.h"

#include <random>
#include "config.h"

class AgentRole {
    protected:
        std::string roleClassName;
        GameMap& gameMap;
        ShuttleData& shuttle;
        Pathing* leastEnergyPathing;

        void log(const std::string& message);
        
    public:
        void setLeastEnergyPathing(Pathing *leastEnergyPathing);
        Direction getDirectionTo(const GameTile &destinationTile);
        std::tuple<int, int> getRelativePosition(const GameTile &destinationTile);
        AgentRole(ShuttleData& shuttle, GameMap& gameMap);
        virtual void surveyJobBoard(JobBoard& jobBoard) = 0;
        virtual ~AgentRole() = default;
};

class RechargeAgentRole : public AgentRole {
    public:
        using AgentRole::AgentRole;
        RechargeAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        void surveyJobBoard(JobBoard& jobBoard) override;
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

        void surveyJobBoard(JobBoard& jobBoard) override;
};

class RelicMiningNavigatorAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::NavigatorAgentRole;
        RelicMiningNavigatorAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        void surveyJobBoard(JobBoard &jobBoard) override;
};

class HaloNodeExplorerAgentRole: public ExplorerAgentRole {
    private:
        std::mt19937 gen = std::mt19937(Config::seed);
        std::uniform_int_distribution<> dis;
        int removeOutOfBounds(int moveId, int x, int y);
    public:
        using ExplorerAgentRole::ExplorerAgentRole;
        HaloNodeExplorerAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        void surveyJobBoard(JobBoard &jobBoard) override;
};

class HaloNodeNavigatorAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::NavigatorAgentRole;
        HaloNodeNavigatorAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        void surveyJobBoard(JobBoard &jobBoard) override;
};

class TrailblazerAgentRole: public NavigatorAgentRole {
    public:
        using NavigatorAgentRole::NavigatorAgentRole;
        TrailblazerAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        void surveyJobBoard(JobBoard& jobBoard) override;
};

class RandomAgentRole: public AgentRole {
    private:
        std::mt19937 gen = std::mt19937(Config::seed);
        std::uniform_int_distribution<> dis; 
    public:
        using AgentRole::AgentRole;
        RandomAgentRole(ShuttleData& shuttle, GameMap& gamemap);
        void surveyJobBoard(JobBoard &jobBoard);
};

class DefenderAgentRole: public AgentRole {
    private:
        std::vector<ShuttleData*> opponents;
    public:
        int attackingTileId = -1;

        using AgentRole::AgentRole;
        DefenderAgentRole(ShuttleData& shuttle, GameMap& gamemap);

        bool chooseAttackingTile(Communicator &communicator);
        void surveyJobBoard(JobBoard &jobBoard);
};

#endif // AGENT_ROLE_H
