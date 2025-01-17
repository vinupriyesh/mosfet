#include "planner.h"

void Planner::log(std::string message) {
    Logger::getInstance().log("Planner -> " + message);
}

void Planner::plan() {
    log("Planning now");
    
}