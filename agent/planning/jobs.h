#ifndef JOBS_H
#define JOBS_H

#include "logger.h"
#include "config.h"
#include "agent/shuttle_data.h"
#include "agent/game_map.h"
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <map>


// Priority constants
const int PRIORITY_DEFENDER = 6;
const int PRIORITY_RELIC_MINER = 5;
const int PRIORITY_RELIC_MINING_NAVIGATOR = 4;
const int PRIORITY_HALO_NODE_EXPLORER = 3;
const int PRIORITY_HALO_NODE_NAVIGATOR = 2;
const int PRIORITY_TRAILBLAZER_NAVIGATOR = 1;
const int PRIORITY_RANDOM = 0;

const int JOB_PRIORITY_MULTIPLIER = 100;

enum JobType {
    RELIC_MINER,
    HALO_NODE_EXPLORER,
    DEFENDER,
    TRAILBLAZER_NAVIGATOR,
    RELIC_MINING_NAVIGATOR,
    HALO_NODE_NAVIGATOR,
};

struct Job {
    int id;
    JobType type;
    int priority;
    int targetX;
    int targetY;

    Job(int id, JobType type);
    Job(int id, JobType type, int targetX, int targetY);

    std::string getJobTypeString(JobType jobType);

    std::string to_string() {
        std::ostringstream ss;
        ss << "Job: id=" << id << ", type=" << getJobTypeString(type) << ", target=(" << targetX << ", " << targetY << ")";
        return ss.str();
    }
};

// Derived job structures
struct RelicMinerJob : Job {
    RelicMinerJob(int id, int vantagePointX, int vantagePointY);
};

struct NavigatorJob : Job {
    NavigatorJob(int id, int destinationX, int destinationY, JobType type);
};

struct RelicMiningNavigatorJob : NavigatorJob {
    RelicMiningNavigatorJob(int id, int destinationX, int destinationY);
};

struct HaloNodeNavigatorJob : NavigatorJob {
    HaloNodeNavigatorJob(int id, int destinationX, int destinationY);
};

struct TrailblazerNavigatorJob : NavigatorJob {
    TrailblazerNavigatorJob(int id, int destinationX, int destinationY);
};

struct HaloNodeExplorerJob : Job {
    HaloNodeExplorerJob(int id, int haloNodeX, int haloNodeY);
};

struct DefenderJob : Job {
    DefenderJob(int id, int opponentPositionX, int opponentPositionY);
};

//----------------------------------------------

enum JobApplicationStatus {
    APPLIED,
    ACCEPTED,
    SHUTTLE_BUSY,
    TARGET_BUSY
};

class JobApplication {
private:    
    JobApplicationStatus status;
    int stepsNeededToExecute;
    int energyNeededToExecute;

public:
    int id;
    int priority;
    ShuttleData* shuttleData;
    std::vector<int> bestPlan;
    Job* job;

    JobApplication(int id, ShuttleData* shuttleData, Job* job, std::vector<int>&& bestPlan);

    // Copy and move semantics
    JobApplication(const JobApplication& other);
    JobApplication& operator=(const JobApplication& other);
    JobApplication& operator=(JobApplication&& other) noexcept;

    void setPriority(int applicationPriority);
    void setStatus(JobApplicationStatus status);

    std::string to_string();
};

//----------------------------------------------

class JobBoard {
private:
    std::vector<Job*> jobs;
    std::vector<JobApplication> jobApplications;
    std::map<int, std::vector<JobApplication>> jobApplicationsByShuttleId;
    std::map<int, std::vector<JobApplication>> jobApplicationsByJobId;
    std::map<JobType, std::unordered_set<int>> jobTypeToJobIdMap;

    std::unordered_set<int> jobDeletionExclusions;
    void log(std::string message);

    void sortJobApplicationsStrategy0(GameMap& gameMap);
    void sortJobApplicationsStrategy1(GameMap& gameMap);
public:
    JobBoard();

    void addJob(Job* job);
    std::vector<Job*>& getJobs();
    std::unordered_set<int> getJobsForType(JobType jobType);
    std::vector<JobApplication> getJobApplicationsForId(int jobId);
    JobApplication& applyForJob(Job* job, ShuttleData* shuttleData, std::vector<int>&& bestPlan);
    void sortJobApplications(GameMap& gameMap);
    std::vector<JobApplication>& getJobApplications();
    void addJobDeletionExclusion(int jobId);
    ~JobBoard();
};

#endif // JOBS_H
