#include "jobs.h"

#include <string>
#include <sstream>
#include "config.h"

// **Job class implementations**

Job::Job(int id, JobType type) : Job(id, type, 0, 0) {
    // TODO: For now using 0. Should investigate if we really need this constructor
}

Job::Job(int id, JobType jobType, int targetX, int targetY)
    : id(id), jobType(jobType), targetX(targetX), targetY(targetY) {
    switch (jobType) {
        case RELIC_MINER:
            priority = PRIORITY_RELIC_MINER;
            break;
        case HALO_NODE_EXPLORER:
            priority = PRIORITY_HALO_NODE_EXPLORER;
            break;
        case DEFENDER:
            priority = PRIORITY_DEFENDER;
            break;
        case RELIC_MINING_NAVIGATOR:
            priority = PRIORITY_RELIC_MINING_NAVIGATOR;
            break;
        case HALO_NODE_NAVIGATOR:
            priority = PRIORITY_HALO_NODE_NAVIGATOR;
            break;
        case TRAILBLAZER_NAVIGATOR:
            priority = PRIORITY_TRAILBLAZER_NAVIGATOR;
            break;
        default:
            priority = PRIORITY_RANDOM;
            break;
    }
}

std::string Job::getJobTypeString(JobType jobType) {
    switch (jobType) {
        case RELIC_MINER:
            return "Relic Miner";
        case HALO_NODE_EXPLORER:
            return "Halo Node Explorer";
        case DEFENDER:
            return "Defender";
        case RELIC_MINING_NAVIGATOR:
            return "Relic Mining Navigator";
        case HALO_NODE_NAVIGATOR:
            return "Halo Node Navigator";
        case TRAILBLAZER_NAVIGATOR:
            return "Trailblazer Navigator";
        default:
            return "Unknown";
    }
}

// **Derived job constructors**

RelicMinerJob::RelicMinerJob(int id, int vantagePointX, int vantagePointY)
    : Job(id, RELIC_MINER, vantagePointX, vantagePointY) {}

NavigatorJob::NavigatorJob(int id, int destinationX, int destinationY, JobType type)
    : Job(id, type, destinationX, destinationY) {}

RelicMiningNavigatorJob::RelicMiningNavigatorJob(int id, int destinationX, int destinationY)
    : NavigatorJob(id, destinationX, destinationY, RELIC_MINING_NAVIGATOR) {}

HaloNodeNavigatorJob::HaloNodeNavigatorJob(int id, int destinationX, int destinationY)
    : NavigatorJob(id, destinationX, destinationY, HALO_NODE_NAVIGATOR) {}

TrailblazerNavigatorJob::TrailblazerNavigatorJob(int id, int destinationX, int destinationY)
    : NavigatorJob(id, destinationX, destinationY, TRAILBLAZER_NAVIGATOR) {}

HaloNodeExplorerJob::HaloNodeExplorerJob(int id, int haloNodeX, int haloNodeY)
    : Job(id, HALO_NODE_EXPLORER, haloNodeX, haloNodeY) {}

DefenderJob::DefenderJob(int id, int opponentPositionX, int opponentPositionY)
    : Job(id, DEFENDER, opponentPositionX, opponentPositionY) {}

//----------------------------------------------

// **JobApplication implementations**

JobApplication::JobApplication(int id, ShuttleData* shuttleData, Job* job, std::vector<int>&& bestPlan)
    : id(id), shuttleData(shuttleData), job(job), bestPlan(std::move(bestPlan)), status(APPLIED), stepsNeededToExecute(0), energyNeededToExecute(0), priority(0) {}

std::string JobApplication::to_string() {
    std::ostringstream ss;
    ss << "JobApplication: id=" << id << ", status=" << status
    << ", priority=" << priority <<
    ", shuttleData=" << shuttleData->to_string()
    << ", job=" << job->to_string();
    return ss.str();
}

JobApplication::JobApplication(const JobApplication& other)
    : id(other.id), status(other.status), stepsNeededToExecute(other.stepsNeededToExecute),
      energyNeededToExecute(other.energyNeededToExecute), priority(other.priority),
      shuttleData(other.shuttleData), bestPlan(other.bestPlan), job(other.job) {}

JobApplication& JobApplication::operator=(const JobApplication& other) {
    if (this != &other) {
        id = other.id;
        status = other.status;
        stepsNeededToExecute = other.stepsNeededToExecute;
        energyNeededToExecute = other.energyNeededToExecute;
        priority = other.priority;
        shuttleData = other.shuttleData;
        bestPlan = other.bestPlan;
        job = other.job;
    }
    return *this;
}

JobApplication& JobApplication::operator=(JobApplication&& other) noexcept {
    if (this != &other) {
        id = other.id;
        status = other.status;
        stepsNeededToExecute = other.stepsNeededToExecute;
        energyNeededToExecute = other.energyNeededToExecute;
        priority = other.priority;
        shuttleData = std::move(other.shuttleData);
        bestPlan = std::move(other.bestPlan);
        job = std::move(other.job);
    }
    return *this;
}

void JobApplication::setPriority(int applicationPriority) {
    this->priority = job->priority * JOB_PRIORITY_MULTIPLIER + applicationPriority;
}

void JobApplication::setStatus(JobApplicationStatus status) {
    this->status = status;
}

//----------------------------------------------

// **JobBoard implementations**

JobBoard::JobBoard()
    : jobs(), jobApplications(), jobApplicationsByShuttleId(), jobApplicationsByJobId(), jobTypeToJobIdMap() {}

void JobBoard::addJob(Job* job) {
    jobs.push_back(job);
}

std::vector<Job*>& JobBoard::getJobs() {
    return jobs;
}

std::unordered_set<int> JobBoard::getJobsForType(JobType jobType) {
    return jobTypeToJobIdMap[jobType];
}

std::vector<JobApplication> JobBoard::getJobApplicationsForId(int jobId) {
    return jobApplicationsByJobId[jobId];
}

JobApplication& JobBoard::applyForJob(Job* job, ShuttleData* shuttleData, std::vector<int>&& bestPlan) {
    int id = jobApplications.size();
    JobApplication jobApplication(id, shuttleData, job, std::move(bestPlan));
    jobApplications.push_back(jobApplication);
    jobApplicationsByShuttleId[shuttleData->id].push_back(jobApplication);
    jobApplicationsByJobId[job->id].push_back(jobApplication);
    jobTypeToJobIdMap[job->jobType].insert(job->id);
    log("Application received for job " + job->to_string() + " from shuttle " + shuttleData->to_string());
    return jobApplications.back();
}

void JobBoard::addJobDeletionExclusion(int jobId) {
    jobDeletionExclusions.insert(jobId);
}

void JobBoard::sortJobApplications(GameMap& gameMap) {
    if (Config::prioritizationStrategy == 1) {
        sortJobApplicationsStrategy1(gameMap);
    } else {
        sortJobApplicationsStrategy0(gameMap);
    }
}

/**
 * Nearest job from Origin first strategy
 */
void JobBoard::sortJobApplicationsStrategy1(GameMap& gameMap) {

    auto compare = [&gameMap](const JobApplication& a, const JobApplication& b) {
        GameTile& aTile = gameMap.getTile(a.job->targetX, a.job->targetY);
        GameTile& bTile = gameMap.getTile(b.job->targetX, b.job->targetY);
        
        if (a.job->jobType == JobType::DEFENDER || b.job->jobType == JobType::DEFENDER) { //Defenders always gets priority
            return a.priority > b.priority;
        } else if (std::abs(aTile.manhattanFromOrigin - bTile.manhattanFromOrigin) <= Config::prioritizationTolerance) {  //A AND B are almost closer
            return a.priority > b.priority;        
        } else {
            return aTile.manhattanFromOrigin < bTile.manhattanFromOrigin;
        }
    };
    
    std::sort(jobApplications.begin(), jobApplications.end(), compare);
}

/**
 * Nearest job from Shuttle first strategy
 */
void JobBoard::sortJobApplicationsStrategy0(GameMap& gameMap) {
    auto compare = [&gameMap](const JobApplication& a, const JobApplication& b) {
        GameTile& aTile = gameMap.getTile(a.job->targetX, a.job->targetY);
        GameTile& bTile = gameMap.getTile(b.job->targetX, b.job->targetY);
        
        GameTile& aShuttleTile = gameMap.getTile(a.shuttleData->getX(), a.shuttleData->getY());
        GameTile& bShuttleTile = gameMap.getTile(b.shuttleData->getX(), b.shuttleData->getY());

        int aManhattanFromShuttle = std::abs(a.job->targetX - a.shuttleData->getX()) + std::abs(a.job->targetY - a.shuttleData->getY());
        int bManhattanFromShuttle = std::abs(b.job->targetX - b.shuttleData->getX()) + std::abs(b.job->targetY - b.shuttleData->getY());
        
        if (a.job->jobType == JobType::DEFENDER || b.job->jobType == JobType::DEFENDER) { //Defenders always gets priority
            return a.priority > b.priority;
        } else if (std::abs(aManhattanFromShuttle - bManhattanFromShuttle) <= Config::prioritizationTolerance) {  //A AND B are almost closer
            return a.priority > b.priority;        
        } else {
            return aManhattanFromShuttle < bManhattanFromShuttle;
        } 
    };
    
    std::sort(jobApplications.begin(), jobApplications.end(), compare);
}

std::vector<JobApplication>& JobBoard::getJobApplications() {
    return jobApplications;
}

void JobBoard::log(std::string message) {
    Logger::getInstance().log("JobBoard -> " + message);
}

JobBoard::~JobBoard() {
    log("Destroying job board");

    for (Job* job : jobs) {
        if (jobDeletionExclusions.find(job->id) == jobDeletionExclusions.end()) {
            delete job;
        }
    }
    jobs.clear();
    jobApplications.clear();
    jobApplicationsByShuttleId.clear();
    jobApplicationsByJobId.clear();
    jobTypeToJobIdMap.clear();
}
