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

// Is set1 a superset of set2?
bool isSuperset(const std::set<int>& set1, const std::set<int>& set2) {
    return std::includes(set1.begin(), set1.end(), set2.begin(), set2.end());
}

// Is set1 a subset of set2?
bool isSubset(const std::set<int>& set1, const std::set<int>& set2) {
    return std::includes(set2.begin(), set2.end(), set1.begin(), set1.end());
}

//TODO: why this is returning a value instead of reference?
std::set<int> subtractSets(const std::set<int>& set1, const std::set<int>& set2) {
    std::set<int> result;
    std::set_difference(set1.begin(), set1.end(), set2.begin(), set2.end(),
                        std::inserter(result, result.end()));
    return result;
}

bool contains(const std::set<int>& haloPointSet, int value) {
    return haloPointSet.find(value) != haloPointSet.end();
}

void removeValue(std::set<int>& haloPointSet, int value) {
    haloPointSet.erase(value);
}

void ConstraintObservation::log(const std::string message) const {
    Logger::getInstance().log("ConstraintObservation -> " + message);
}

/**
 * Inside constraint set we deal only with the first half tiles.  The 2nd half tiles will be converted
 */
ConstraintObservation::ConstraintObservation(int pv, const std::set<int>& hps) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    pointsValue = pv;
    for (auto& haloPoint : hps) {
        int x = haloPoint % gameEnvConfig.mapWidth;
        int y = haloPoint / gameEnvConfig.mapWidth;
        if (x+y >= gameEnvConfig.mapHeight) {
            //This point is in 2nd half, move it to first
            int xMir = gameEnvConfig.mapHeight - y - 1;
            int yMir = gameEnvConfig.mapWidth - x - 1;

            int haloPointMir = yMir * gameEnvConfig.mapWidth + xMir;

            if (haloPointSet.find(haloPointMir) == haloPointSet.end()) {
                //This is a new node
                haloPointSet.insert(haloPointMir);
            } else {
                extraMirroredHaloPointSet.insert(haloPointMir);
            }
        } else {
            //This point is in 1st half, use as is
            if (haloPointSet.find(haloPoint) == haloPointSet.end()) {
                //This is a new node
                haloPointSet.insert(haloPoint);
            } else {
                extraMirroredHaloPointSet.insert(haloPoint);
            }
        }
    }
}

bool ConstraintObservation::simplify(std::vector<ConstraintObservation> &nextRecursionCycle) const{
    if (extraMirroredHaloPointSet.size() == 0) {
        return false;
    }

    int iMatch = -1;
    int jMatch = -1;
    for (int i = 0; i <= haloPointSet.size() - extraMirroredHaloPointSet.size(); ++i) {
        for (int j = 0; j <= extraMirroredHaloPointSet.size(); ++j) {
            if (i + 2*j == pointsValue) {
                if (iMatch == -1 && jMatch == -1) {
                    iMatch = i;
                    jMatch = j;
                } else {
                    //More than 1 possible values.  Cant simplify
                    return false;
                }
            }
        }
    }

    // The non mirrored observation
    auto splitSet = subtractSets(haloPointSet, extraMirroredHaloPointSet);
    if (splitSet.size() == 0) {
        //This one only has mirrored point set
        return false;
    }
    nextRecursionCycle.push_back(ConstraintObservation(iMatch, splitSet));
    nextRecursionCycle.push_back(ConstraintObservation(jMatch, extraMirroredHaloPointSet, extraMirroredHaloPointSet));

    log("Split the haloPointSet and extraMirroredHaloPointSet " + setToString(haloPointSet) + " extra: " + setToString(extraMirroredHaloPointSet) +
             " with points " + std::to_string(iMatch) + " and " + std::to_string(jMatch) + ", original point = " + std::to_string(pointsValue));
    return true;
}

void ConstraintObservation::insertAllMirrors(std::set<int> &target) const{
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    for (auto& haloPoint : haloPointSet) {
        int x = haloPoint % gameEnvConfig.mapWidth;
        int y = haloPoint / gameEnvConfig.mapWidth;

        int xMir = gameEnvConfig.mapHeight - y - 1;
        int yMir = gameEnvConfig.mapWidth - x - 1;

        int haloPointMir = yMir * gameEnvConfig.mapWidth + xMir;

        target.insert(haloPointMir);
    }
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

void ConstraintSet::clear() {
    identifiedVantagePoints.clear();
    identifiedRegularTiles.clear();
}

/**
 * When a new relic is found and if the halo nodes overlap with existing nodes, it is better we 'forget' the older constraints. 
 * This is because, the older constraints need not be true anymore unfortunately. 
 * 
 * I am calling it phaseOut and not forget, because i might come back to this and add versioning logic.  It is too complex to think,
 * but its sad to forget the hard found constraints. :(
 */
void ConstraintSet::phaseOutOlderConstraints(int tileId) {
    tileId = SymmetryUtils::toFirstHalfID(tileId);
    log("Phasing out older constraints for tile " + std::to_string(tileId));    
    auto it = masterSet.begin();
    int count = 0;
    while (it != masterSet.end()) {
        if (contains(it->haloPointSet, tileId)) {
            log("Removing constraint with points value " + std::to_string(it->pointsValue) + " and halo point set" + setToString(it->haloPointSet));
            it = masterSet.erase(it); // Erase and get the next valid iterator 
            count++;
        } else {
            ++it; // Move to the next element
        }
    }
    Metrics::getInstance().add("phased_out_constraints", count);
}

/**
 * If the master set has any regular tile or vantage points, then we can reduce them to have a slimmer constraint set.
 */
void ConstraintSet::pruneConstraints() {
    log("Pruning constraints");
    auto it = masterSet.begin();
    std::vector<ConstraintObservation> nextRecursionCycle;
    while (it != masterSet.end()) {
        auto itPoints = it->haloPointSet.begin();
        while (itPoints != it->haloPointSet.end()) {
            int value = *itPoints;
            if (identifiedRegularTiles.find(value) != identifiedRegularTiles.end()) {
                log("Prune regular tiles " + std::to_string(value));
                itPoints = it->haloPointSet.erase(itPoints);                
            } else if (identifiedVantagePoints.find(value) != identifiedVantagePoints.end()) {
                log("Prune vantage point " + std::to_string(value));
                itPoints = it->haloPointSet.erase(itPoints);
                it->pointsValue--;
            } else {
                ++itPoints;
            }
        }

        auto itPointsExtra = it->extraMirroredHaloPointSet.begin();
        while (itPointsExtra != it->extraMirroredHaloPointSet.end()) {
            int value = *itPointsExtra;
            if (identifiedRegularTiles.find(value) != identifiedRegularTiles.end()) {
                log("Prune regular tiles mirror " + std::to_string(value));
                itPointsExtra = it->extraMirroredHaloPointSet.erase(itPointsExtra);                
            } else if (identifiedVantagePoints.find(value) != identifiedVantagePoints.end()) {
                log("Prune vantage point mirror" + std::to_string(value));
                itPointsExtra = it->extraMirroredHaloPointSet.erase(itPointsExtra);
                it->pointsValue--;
            } else {
                ++itPointsExtra;
            }
        }

        if (it->haloPointSet.empty()) {
            log("Empty halo point set, removing the constraint");
            it = masterSet.erase(it);
        } else if(it->haloPointSet.size() + it->extraMirroredHaloPointSet.size() == it->pointsValue || it->pointsValue == 0) {
            log("Constraint is terminal, removing the constraint");
            nextRecursionCycle.push_back(ConstraintObservation(it->pointsValue, std::move(it->haloPointSet)));
            it = masterSet.erase(it);            
        }else {
            ++it;
        }
    }

    for (auto& record : nextRecursionCycle) {
        addConstraint(record);
    }
}

void ConstraintSet::addConstraint(int pointsValue, std::set<int>& haloPointSet) {
    auto start = std::chrono::high_resolution_clock::now();
    log("Entering constraint with points value " + std::to_string(pointsValue) + " and halo point set" + setToString(haloPointSet));
    log("Regular tiles: " + setToString(identifiedRegularTiles));
    log("Vantage points: " + setToString(identifiedVantagePoints));

    auto it = haloPointSet.begin();
    while (it != haloPointSet.end()) {
        int value = *it;
        if (identifiedRegularTiles.find(value) != identifiedRegularTiles.end()) {
            log("Found as regular tile " + std::to_string(value));
            it = haloPointSet.erase(it); 
        } else if (identifiedVantagePoints.find(value) != identifiedVantagePoints.end()) {
            log("Found as vantage point " + std::to_string(value));
            it = haloPointSet.erase(it); 
            pointsValue--;
        } else {
            ++it; // Move to the next element
        }
    }

    ConstraintObservation observation(pointsValue, haloPointSet);
    addConstraint(std::move(observation));    

    Metrics::getInstance().add("constraint_set_size", masterSet.size());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);  
    Metrics::getInstance().add("add_constraint_duration", duration.count());
}

void ConstraintSet::reconsiderNormalizedTile(int tileId) {    
    int mirroredTileId = SymmetryUtils::toMirroredID(tileId);
    if (identifiedRegularTiles.find(tileId) != identifiedRegularTiles.end() || identifiedRegularTiles.find(mirroredTileId) != identifiedRegularTiles.end()) {
        identifiedRegularTiles.erase(tileId);
        identifiedRegularTiles.erase(mirroredTileId);
        log("Removing regular tile " + std::to_string(tileId) + " and " + std::to_string(mirroredTileId));        
    } else if (Config::phaseOutConstraints && identifiedVantagePoints.find(tileId) == identifiedVantagePoints.end() && identifiedVantagePoints.find(mirroredTileId) == identifiedVantagePoints.end()) { 
        // This if condition is there just for performance. As vantage points will always be
        // included in the set, better check if it is not a vantage point before phasing out.
        phaseOutOlderConstraints(tileId);
    }
}

void ConstraintSet::reconsiderNormalizedTile(std::vector<int> tileIds) {
    for (int tileId : tileIds) {
        reconsiderNormalizedTile(tileId);
    }
}

void ConstraintSet::addConstraint(const ConstraintObservation& observation) {
    log("Adding constraint with points value " + std::to_string(observation.pointsValue) + " and halo point set " +
         setToString(observation.haloPointSet) + ", mirror set " + setToString(observation.extraMirroredHaloPointSet));

    if (observation.haloPointSet.empty()) {
        log("Empty halo point set");
        // std::cerr<<"Problem:Empty halo point set";
        return;
    }

    if (observation.pointsValue < 0) {
        log("Problem:Points value is negative");
        std::cerr<<"Problem:Points value is negative"<<std::endl;
        return;
    }

    if (observation.pointsValue > observation.haloPointSet.size() + observation.extraMirroredHaloPointSet.size()) {
        log("Problem:Points value is more than the number of halo nodes " + std::to_string(observation.pointsValue) +
                 ", " + std::to_string(observation.haloPointSet.size()) + " + " + std::to_string(observation.extraMirroredHaloPointSet.size()));
        std::cerr<<"Problem:Points value is higher than nodes"<<std::endl;
    }
    

    if (observation.pointsValue == 0) {
        log("Regular tiles found: " + setToString(observation.haloPointSet));
        identifiedRegularTiles.insert(observation.haloPointSet.begin(), 
                                      observation.haloPointSet.end());
        observation.insertAllMirrors(identifiedRegularTiles);
    } else if (observation.pointsValue == observation.haloPointSet.size() + observation.extraMirroredHaloPointSet.size()) {
        log("Vantage points found: " + setToString(observation.haloPointSet));
        identifiedVantagePoints.insert(observation.haloPointSet.begin(), 
                                       observation.haloPointSet.end());
        observation.insertAllMirrors(identifiedVantagePoints);        
    }

    log("Resolved regular and vantage tiles");

    std::vector<ConstraintObservation> nextRecursionCycle;
    bool isSubsetFound = false;
    bool isSupersetFound = false;

    auto it = masterSet.begin();
    while (it != masterSet.end()) {
        log("Comparing with existing constraint with points value " + std::to_string(it->pointsValue) + " and halo point set" + setToString(it->haloPointSet));

        bool iterated = false;

        if (it->haloPointSet == observation.haloPointSet) {
            // The same constraint already exists

            if (it->pointsValue != observation.pointsValue) {
                log("Problem:The constraint already exists with a different points value"
                 + std::to_string(observation.pointsValue) + " vs " + std::to_string(it->pointsValue));

                std::cerr<<"Problem:The constraint already exists with a different points value"
                 + std::to_string(observation.pointsValue) + " vs " + std::to_string(it->pointsValue)<<std::endl;
            }

            log("Constraint already existing, no action");
            return;
        } else if (isSubset(observation.haloPointSet, it->haloPointSet)) {
            log("Subset found: " + setToString(observation.haloPointSet) + " is subset of " + setToString(it->haloPointSet));
            isSubsetFound = true;

            int newPointsValue = it->pointsValue - observation.pointsValue;
            auto newSubset = subtractSets(it->haloPointSet, observation.haloPointSet);
            auto newMirrorSubset = subtractSets(it->extraMirroredHaloPointSet, observation.extraMirroredHaloPointSet);
            nextRecursionCycle.push_back(ConstraintObservation(newPointsValue, std::move(newSubset), std::move(newMirrorSubset)));

            log("Erasing the superset " + setToString(it->haloPointSet));
            it = masterSet.erase(it);
            iterated = true;
        } else if (isSuperset(observation.haloPointSet, it->haloPointSet)) {
            log("Superset found: " + setToString(observation.haloPointSet) + " is superset of " + setToString(it->haloPointSet));
            isSupersetFound = true;

            int newPointsValue = observation.pointsValue - it->pointsValue;
            auto newSuperset = subtractSets(observation.haloPointSet, it->haloPointSet);
            auto newMirrorSuperset = subtractSets(observation.extraMirroredHaloPointSet, it->extraMirroredHaloPointSet);
            nextRecursionCycle.push_back(ConstraintObservation(newPointsValue, std::move(newSuperset), std::move(newMirrorSuperset)));
        }

        // Move to the next element only if it is not already done during delete
        if (!iterated) {
            ++it; 
        }
    }
    
    if (observation.haloPointSet.size() != observation.pointsValue && observation.pointsValue != 0) {
        log("This observation is not a terminal, adding it to set " + setToString(observation.haloPointSet));
        addSet(observation.pointsValue, observation.haloPointSet);
    }

    if (observation.simplify(nextRecursionCycle) || isSubsetFound || isSupersetFound) {        
        for (auto& record : nextRecursionCycle) {
            log("Recursing..");
            addConstraint(record);
        }
    }

    pruneConstraints();

    log("All done");
}

void ConstraintSet::addSet(int pointsValue, const std::set<int> &haloPointSet) {
    masterSet.emplace_back(pointsValue, std::move(haloPointSet));
}
