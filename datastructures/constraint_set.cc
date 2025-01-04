#include "constraint_set.h"


std::string setToString(const std::set<int>& s) {
    std::ostringstream oss;
    oss << "{";
    for (auto it = s.begin(); it != s.end(); ++it) {
        if (it != s.begin()) {
            oss << ", ";
        }
        oss << *it;
    }
    oss << "}";
    return oss.str();
}

void ConstraintSet::log(const std::string message) const {
    Logger::getInstance().log("ConstraintSet -> " + message);
}

void ConstraintSet::logMasterSet() const {
    for (const auto& observation : masterSet) {
        std::string message = "Points Value: " + std::to_string(observation.pointsValue) +
                              ", Halo Point Set: " + setToString(observation.haloPointSet);
        log(message);
    }
}

// Is set1 a superset of set2?
bool isSuperset(const std::set<int>& set1, const std::set<int>& set2) {
    return std::includes(set1.begin(), set1.end(), set2.begin(), set2.end());
}

// Is set1 a subset of set2?
bool isSubset(const std::set<int>& set1, const std::set<int>& set2) {
    return std::includes(set2.begin(), set2.end(), set1.begin(), set1.end());
}

std::set<int> subtractSets(const std::set<int>& set1, const std::set<int>& set2) {
    std::set<int> result;
    std::set_difference(set1.begin(), set1.end(), set2.begin(), set2.end(),
                        std::inserter(result, result.end()));
    return result;
}


void ConstraintSet::clear() {
    identifiedVantagePoints.clear();
    identifiedRegularTiles.clear();
}

bool contains(const std::set<int>& haloPointSet, int value) {
    return haloPointSet.find(value) != haloPointSet.end();
}

void removeValue(std::set<int>& haloPointSet, int value) {
    haloPointSet.erase(value);
}

void ConstraintSet::addConstraint(int pointsValue, std::set<int>& haloPointSet) {
    log("Entry constraint with points value " + std::to_string(pointsValue) + " and halo point set" + setToString(haloPointSet));

    auto it = haloPointSet.begin();
    while (it != haloPointSet.end()) {
        int value = *it;
        if (identifiedRegularTiles.find(value) != identifiedRegularTiles.end()) {
            log("Found as regular tile " + std::to_string(value));
            it = haloPointSet.erase(it); // Erase and get the next valid iterator
        } else if (identifiedVantagePoints.find(value) != identifiedVantagePoints.end()) {
            log("Found as vantage point " + std::to_string(value));
            it = haloPointSet.erase(it); // Erase and get the next valid iterator
            pointsValue--;
        } else {
            ++it; // Move to the next element
        }
    }

    ConstraintObservation observation(pointsValue, haloPointSet);
    addConstraint(std::move(observation));
}

void ConstraintSet::addConstraint(const ConstraintObservation& observation) {
    log("Adding constraint with points value " + std::to_string(observation.pointsValue) + " and halo point set" + setToString(observation.haloPointSet));
    int pointsValue = observation.pointsValue;
    const std::set<int>& haloPointSet = observation.haloPointSet;

    if (haloPointSet.empty()) {
        log("Empty halo point set");
        // std::cerr<<"Problem:Empty halo point set";
        return;
    }

    if (pointsValue < 0) {
        log("Problem:Points value is negative");
        std::cerr<<"Problem:Points value is negative";
        return;
    }

    if (pointsValue == 0) {
        log("Regular tiles found: " + setToString(haloPointSet));
        identifiedRegularTiles.insert(observation.haloPointSet.begin(), 
                                      observation.haloPointSet.end());
    } else if (pointsValue == haloPointSet.size()) {
        log("Vantage points found: " + setToString(haloPointSet));
        identifiedVantagePoints.insert(observation.haloPointSet.begin(), 
                                       observation.haloPointSet.end());
        
    }

    std::vector<ConstraintObservation> nextRecursionCycle;
    bool isSubsetFound = false;
    bool isSupersetFound = false;

    auto it = masterSet.begin();
    while (it != masterSet.end()) {
        log("Comparing with existing constraint with points value " + std::to_string(it->pointsValue) + " and halo point set" + setToString(it->haloPointSet));

        bool iterated = false;

        if (it->haloPointSet == haloPointSet) {
            // The same constraint already exists

            if (it->pointsValue != pointsValue) {
                log("Problem:The constraint already exists with a different points value"
                 + std::to_string(pointsValue) + " vs " + std::to_string(it->pointsValue));

                std::cerr<<"Problem:The constraint already exists with a different points value"
                 + std::to_string(pointsValue) + " vs " + std::to_string(it->pointsValue);
            }

            log("Constraint already existing, no action");
            return;
        } else if (isSubset(haloPointSet, it->haloPointSet)) {
            log("Subset found: " + setToString(haloPointSet) + " is subset of " + setToString(it->haloPointSet));
            isSubsetFound = true;

            int newPointsValue = it->pointsValue - pointsValue;
            auto newSubset = subtractSets(it->haloPointSet, haloPointSet);            
            nextRecursionCycle.push_back(ConstraintObservation(newPointsValue, std::move(newSubset)));

            log("Erasing the superset " + setToString(it->haloPointSet));
            it = masterSet.erase(it); // Erase and get the next valid iterator
            iterated = true;
        } else if (isSuperset(haloPointSet, it->haloPointSet)) {
            log("Superset found: " + setToString(haloPointSet) + " is superset of " + setToString(it->haloPointSet));
            isSupersetFound = true;

            int newPointsValue = pointsValue - it->pointsValue;
            auto newSuperset = subtractSets(haloPointSet, it->haloPointSet);
            nextRecursionCycle.push_back(ConstraintObservation(newPointsValue, std::move(newSuperset)));
        }

        // Move to the next element only if it is not already done during delete
        if (!iterated) {
            ++it; 
        }
    }
    
    if (haloPointSet.size() != pointsValue && pointsValue != 0) {
        log("This observation is not a terminal, adding it to set " + setToString(haloPointSet));
        addSet(pointsValue, haloPointSet);
    }

    if (isSubsetFound || isSupersetFound) {        
        for (auto& record : nextRecursionCycle) {
            addConstraint(record);
        }
    }
}

void ConstraintSet::addSet(int pointsValue, const std::set<int> &haloPointSet) {
    masterSet.emplace_back(pointsValue, std::move(haloPointSet));
}
