// metrics.h
#ifndef METRICS_H
#define METRICS_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>


struct MetricDetails {
    int unitMoveCost = 0;
    int unitSapCost = 0;
    int unitSapRange = 0;
    int unitSensorRange = 0;
    int wins = 0;
    int losses = 0;
    int gameWon = 0;

    float unitSapDropOffFactor = -1.0;
    int unitSapDropOffFactorIdentifiedStep = -1;
    float unitEnergyVoidFactor = -1.0;
    int unitEnergyVoidFactorIdentifiedStep = -1;
    int nebulaTileEnergyReduction = -1;
    int nebulaTileEnergyReductionIdentifiedStep = -1;

    int energyNodeIdentifiedStep = -1;
    int energyNodeDriftIdnetifiedStep = -1;

    int relicsCount = 0;
    int relicDiscoveryStepRound1 = -1;
    int relicDiscoveryStepRound2 = -1;
    int relicDiscoveryStepRound3 = -1;
    int nebulaTileVisionReduction = -1;
    int nebulaTileVisionReductionIdentifiedStep = -1;

    float nebulaTileDriftSpeed = 0.0;
    int nebulaTileDriftSpeedIdentifiedStep = -1;
};

class Metrics {
            
    public:
        MetricDetails details;

        static Metrics& getInstance() {
            static Metrics instance;
            return instance;
        }

        void setPlayerName(const std::string& name) {
            player_name = name;
        }

        void setStepId(const std::string& id) {
            step_id = id;
        }

        void enableMetrics(const std::string& filename) {
            if (!log_file.is_open()) {
                log_file.open(filename, std::ios::out | std::ios::app);
                log_file << "timestep,player_id,dimension,value" << std::endl;
            }
        }

        bool isMetricEnabled() {
            return log_file.is_open();
        }

        void add(const std::string& name, const float value){
            if (log_file.is_open()) {
                log_file << step_id << "," << player_name << "," << name << "," << value << std::endl;
            }
        }

        void saveMetricDetails(const std::string& filename) {
            std::ofstream file;
            bool fileExists = false;
            
            // Check if file exists
            std::ifstream checkFile(filename);
            fileExists = checkFile.good();
            checkFile.close();
            
            // Open file in append mode
            file.open(filename, std::ios::out | std::ios::app);
            
            if (!fileExists) {
                // Write headers if file is new
                file << "unitMoveCost,unitSapCost,unitSapRange,unitSensorRange,wins,losses,gameWon,unitSapDropOffFactor,unitSapDropOffFactorIdentifiedStep,"
                    << "unitEnergyVoidFactor,unitEnergyVoidFactorIdentifiedStep,energyNodeIdentifiedStep,"
                    << "energyNodeDriftIdnetifiedStep,relicsCount,relicDiscoveryStepRound1,"
                    << "relicDiscoveryStepRound2,relicDiscoveryStepRound3,nebulaTileEnergyReduction,"
                    << "nebulaTileEnergyReductionIdentifiedStep,nebulaTileVisionReduction,"
                    << "nebulaTileVisionReductionIdentifiedStep,nebulaTileDriftSpeed,"
                    << "nebulaTileDriftSpeedIdentifiedStep" << std::endl;
            }
            
            // Write data
            file << details.unitMoveCost << ","
                << details.unitSapCost << ","
                << details.unitSapRange << ","
                << details.unitSensorRange << ","
                << details.wins << ","
                << details.losses << ","
                << details.gameWon << ","
                << details.unitSapDropOffFactor << ","
                << details.unitSapDropOffFactorIdentifiedStep << ","
                << details.unitEnergyVoidFactor << ","
                << details.unitEnergyVoidFactorIdentifiedStep << ","
                << details.energyNodeIdentifiedStep << ","
                << details.energyNodeDriftIdnetifiedStep << ","
                << details.relicsCount << ","
                << details.relicDiscoveryStepRound1 << ","
                << details.relicDiscoveryStepRound2 << ","
                << details.relicDiscoveryStepRound3 << ","
                << details.nebulaTileEnergyReduction << ","
                << details.nebulaTileEnergyReductionIdentifiedStep << ","
                << details.nebulaTileVisionReduction << ","
                << details.nebulaTileVisionReductionIdentifiedStep << ","
                << details.nebulaTileDriftSpeed << ","
                << details.nebulaTileDriftSpeedIdentifiedStep << std::endl;
            
            file.close();
        }

    private:
        std::string step_id = "-1";
        std::string player_name = "Unknown";
        std::ofstream log_file;

        Metrics() = default;
        ~Metrics() {
            if (log_file.is_open()) {
                log_file.close();
            }
        }        

        // Delete copy constructor and assignment operator
        Metrics(const Metrics&) = delete;
        Metrics& operator=(const Metrics&) = delete;
};

#endif // METRICS_H
