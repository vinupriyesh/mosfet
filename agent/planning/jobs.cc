#include "jobs.h"

// **Job class implementations**

Job::Job(int id, JobType type) : Job(id, type, 0, 0) {
    // TODO: For now using 0. Should investigate if we really need this constructor
}

Job::Job(int id, JobType type, int targetX, int targetY)
    : id(id), type(type), targetX(targetX), targetY(targetY) {
    switch (type) {
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
    jobTypeToJobIdMap[job->type].insert(job->id);
    return jobApplications.back();
}

void JobBoard::addJobDeletionExclusion(int jobId) {
    jobDeletionExclusions.insert(jobId);
}

void JobBoard::sortJobApplications() {
    auto compare = [](const JobApplication& a, const JobApplication& b) {
        return a.priority > b.priority;
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
