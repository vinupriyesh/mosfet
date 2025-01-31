#ifndef JOBS_H
#define JOBS_H

#include "agent/shuttle_data.h"

#include <vector>
#include <string>

enum JobType {
    RELIC_MINER,
    NAVIGATOR,
    HALO_NODE_EXPLORER,
    DEFENDER,
    TRAILBLAZER
};

struct Job {
    int id;    
    JobType type;

    Job(int id, JobType type): id(id), type(type) {}

    std::string getJobTypeString(JobType jobType) {
    switch (jobType) {
        case RELIC_MINER:
            return "Relic Miner";
        case NAVIGATOR:
            return "Navigator";
        case HALO_NODE_EXPLORER:
            return "Halo Node Explorer";
        case DEFENDER:
            return "Defender";
        case TRAILBLAZER:
            return "Trailblazer";
        default:
            return "Unknown";
        }
    }
};

struct RelicMinerJob: Job {
    int vantagePointX;
    int vantagePointY;
    RelicMinerJob(int id, int vantagePointX, int vantagePointY): Job(id, RELIC_MINER), vantagePointX(vantagePointX), vantagePointY(vantagePointY) {}
};

struct NavigatorJob: Job {
    int destinationX;
    int destinationY;
    NavigatorJob(int id, int destinationX, int destinationY): Job(id, NAVIGATOR), destinationX(destinationX), destinationY(destinationY) {}
};

struct HaloNodeExplorerJob: Job {
    int haloNodeX; //TODO:  this should be a node cluster in future
    int haloNodeY;
    HaloNodeExplorerJob(int id, int haloNodeX, int haloNodeY): Job(id, HALO_NODE_EXPLORER), haloNodeX(haloNodeX), haloNodeY(haloNodeY) {}
};

struct DefenderJob: Job {
    int opponentPositionX;
    int opponentPositionY;
    DefenderJob(int id, int opponentPositionX, int opponentPositionY): Job(id, DEFENDER), opponentPositionX(opponentPositionX), opponentPositionY(opponentPositionY) {}
};

struct TrailblazerJob: Job {
    TrailblazerJob(int id): Job(id, TRAILBLAZER) {}
};

//----------------------------------------------

struct JobApplication {
    int id;
    ShuttleData& shuttleData;
    Job& job;
    std::vector<int> bestPlan;

    int stepsNeededToExecute;
    int energyNeededToExecute;    

    JobApplication(int id, ShuttleData& shuttleData, Job& job, std::vector<int>&& bestPlan): id(id), shuttleData(shuttleData), job(job), bestPlan(std::move(bestPlan)) {
        stepsNeededToExecute = 0;
        energyNeededToExecute = 0;
    }
};

//----------------------------------------------

class JobBoard {
    private:
        std::vector<Job> jobs;
        std::vector<JobApplication> jobApplications;
        std::map<int, std::vector<JobApplication>> jobApplicationsByShuttleId;
        std::map<int, std::vector<JobApplication>> jobApplicationsByJobId;

        void log(std::string message) {
            Logger::getInstance().log("JobBoard -> " + message);
        }

    public:
        JobBoard() {
            jobs = std::vector<Job>();
            jobApplications = std::vector<JobApplication>();
            jobApplicationsByShuttleId = std::map<int, std::vector<JobApplication>>();
            jobApplicationsByJobId = std::map<int, std::vector<JobApplication>>();
        }

        void addJob(Job& job) {
            jobs.push_back(job);
        }

        std::vector<Job>& getJobs() {
            return jobs;
        }

        JobApplication& applyForJob(Job& job, ShuttleData& shuttleData, std::vector<int>&& bestPlan) {
            int id = jobApplications.size();
            JobApplication* jobApplication = new JobApplication{id, shuttleData, job, std::move(bestPlan)};
            jobApplications.push_back(*jobApplication);
            jobApplicationsByShuttleId[shuttleData.id].push_back(*jobApplication);
            jobApplicationsByJobId[job.id].push_back(*jobApplication);
            return *jobApplication;
        }

        ~JobBoard() {
            log("Destroying job board");
            
            for (auto& job: jobs) {
                delete &job;
            }

            for (auto& jobApplication: jobApplications) {
                delete &jobApplication;
            }

            jobApplications.clear();
            jobApplicationsByShuttleId.clear();
            jobApplicationsByJobId.clear();
        }
};


#endif // JOBS_H