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

// ----------------------------------------------------------------------------

std::string ConstraintObservation::toString() const {
    std::ostringstream oss;
    oss << pointsValue << " @ " << setToString(haloPointSet);
    if (extraMirroredHaloPointSet.size() > 0) {
        oss<< " | " << setToString(extraMirroredHaloPointSet);
    }

    return oss.str();
}

bool ConstraintObservation::isValid() const {
    if (haloPointSet.empty()) {
        log("Empty halo point set, not a problem, silenty fail");        
        return false;
    }

    if (pointsValue < 0) {
        log("Problem:Points value is negative "+ toString());
        std::cerr<<"Problem:Points value is negative "<<std::endl;
        return false;
    }

    if (pointsValue > haloPointSet.size() + extraMirroredHaloPointSet.size()) {
        log("Problem:Points value is more than the number of halo nodes " + toString());
        std::cerr<<"Problem:Points value is higher than nodes "<<std::endl;
        return false;
    }

    for (int tileId :extraMirroredHaloPointSet) {
        if (!contains(haloPointSet, tileId)) {
            log("Problem:Mirror point not found in haloPoint " + toString());
            std::cerr<<"Problem:Mirror point not found in haloPoint "<<std::endl;
            return false;
        }
    }

    return true;
}

void ConstraintObservation::collectRegularAndVantagePoints(std::set<int> &identifiedRegularTiles, std::set<int> &identifiedVantagePoints) const{

    if (pointsValue == 0) {
        log("Regular tiles found: " + setToString(haloPointSet));
        identifiedRegularTiles.insert(haloPointSet.begin(), 
                                      haloPointSet.end());
        insertAllMirrors(identifiedRegularTiles);
    } else if (pointsValue == haloPointSet.size() + extraMirroredHaloPointSet.size()) {
        log("Vantage points found: " + setToString(haloPointSet));
        identifiedVantagePoints.insert(haloPointSet.begin(), 
                                       haloPointSet.end());
        insertAllMirrors(identifiedVantagePoints);        
    }
}

/**
 * Is 'this' a subset of other?
 */
bool ConstraintObservation::isSubsetObservation(ConstraintObservation &other) const {
    return isSubset(haloPointSet, other.haloPointSet) && isSubset(extraMirroredHaloPointSet, other.extraMirroredHaloPointSet);
}

bool ConstraintObservation::isSupersetObservation(ConstraintObservation &other) const {
    return isSuperset(haloPointSet, other.haloPointSet) && isSuperset(extraMirroredHaloPointSet, other.extraMirroredHaloPointSet);
}

void ConstraintObservation::log(const std::string message) const {
    Logger::getInstance().log("ConstraintObservation -> " + message);
}

/**
 * The constructor converts the points to the first half of the map tiles.  Inside constraints set we deal only with the
 * first half of the tiles, the second half is called the mirrors.  Mirrors are used to augment faster detection of vantage points
 */
ConstraintObservation::ConstraintObservation(int pv, const std::set<int>& hps) {
    GameEnvConfig& gameEnvConfig = GameEnvConfig::getInstance();
    pointsValue = pv;
    for (auto& haloPoint : hps) {
        int firstHalfHaloPoint = symmetry_utils::toFirstHalfID(haloPoint);

        if (!contains(haloPointSet, firstHalfHaloPoint)) {
            haloPointSet.insert(firstHalfHaloPoint);
        } else {
            extraMirroredHaloPointSet.insert(firstHalfHaloPoint);
        }
    }

    log("Created a new observation from "  + setToString(hps) + " = " + toString());
}

/**
 * Simplify uses the mirrored tiles to split the constraint.   
 * 
 * For example, " 2 @ {a, b} | {a'} " this constraint can have only a = 1, b = 0 resolution.  
 * Simply will split this into " 2 @ {a} | {a'}" and "0 @ {b}" which will further resolve in the set.
 */
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
                    jMatch = j * 2;
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
        return false;
    }

    auto normalObservation = ConstraintObservation(iMatch, splitSet);
    auto mirrorObservation = ConstraintObservation(jMatch, extraMirroredHaloPointSet, extraMirroredHaloPointSet);
    nextRecursionCycle.push_back(std::move(normalObservation));
    nextRecursionCycle.push_back(std::move(mirrorObservation));

    log("Simplified " + toString() + " into " + normalObservation.toString() + " and " + mirrorObservation.toString());
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
    log(" ---- Master Set ----");
    for (const auto& observation : masterSet) {
        log(observation.toString());
    }
    log(" --------------------");
    log("Vantage points: " + setToString(identifiedVantagePoints));
    log("Regular tiles : " + setToString(identifiedRegularTiles));
    log(" --------------------");
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
    tileId = symmetry_utils::toFirstHalfID(tileId);
    log("Phasing out older constraints for tile " + std::to_string(tileId));    
    auto it = masterSet.begin();
    int count = 0;
    while (it != masterSet.end()) {
        if (contains(it->haloPointSet, tileId)) {
            log("Removing constraint with points value " + std::to_string(it->pointsValue) + " and halo point set" + setToString(it->haloPointSet));
            it = masterSet.erase(it);
            count++;
        } else {
            ++it;
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
                log("Prune regular tiles " + std::to_string(value) + " - " + symmetry_utils::toXYString(value));
                itPoints = it->haloPointSet.erase(itPoints);                
            } else if (identifiedVantagePoints.find(value) != identifiedVantagePoints.end()) {
                log("Prune vantage point " + std::to_string(value) + " - " + symmetry_utils::toXYString(value));
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
                log("Prune regular tiles mirror " + std::to_string(value) + " - " + symmetry_utils::toXYString(value));
                itPointsExtra = it->extraMirroredHaloPointSet.erase(itPointsExtra);                
            } else if (identifiedVantagePoints.find(value) != identifiedVantagePoints.end()) {
                log("Prune vantage point mirror" + std::to_string(value) + " - " + symmetry_utils::toXYString(value));
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
            nextRecursionCycle.push_back(ConstraintObservation(it->pointsValue, std::move(it->haloPointSet), std::move(it->extraMirroredHaloPointSet)));
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
            ++it;
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
    int mirroredTileId = symmetry_utils::toMirroredID(tileId);
    if (contains(identifiedRegularTiles, tileId) || contains(identifiedRegularTiles, mirroredTileId)) {
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
    if (!observation.isValid()) {
        return;
    }

    log("Adding observation - " + observation.toString());

    observation.collectRegularAndVantagePoints(identifiedRegularTiles, identifiedVantagePoints);

    std::vector<ConstraintObservation> nextRecursionCycle;
    bool isSubsetFound = false;
    bool isSupersetFound = false;

    auto it = masterSet.begin();
    while (it != masterSet.end()) {
        log("Comparing observation with " + it->toString());

        bool iterated = false;

        if (it->haloPointSet == observation.haloPointSet && it->extraMirroredHaloPointSet == observation.extraMirroredHaloPointSet) {
            // The same constraint already exists

            if (it->pointsValue != observation.pointsValue) {
                log("Problem:The constraint already exists with a different points value"
                 + std::to_string(observation.pointsValue) + " vs " + std::to_string(it->pointsValue));

                std::cerr<<"Problem:The constraint already exists with a different points value"<<std::endl;
            }

            log("Constraint already existing, no action");
            return;
        } else if (observation.isSubsetObservation(*it)) {
            log("Subset found: " + setToString(observation.haloPointSet) + " is subset of " + setToString(it->haloPointSet));
            isSubsetFound = true;            

            int newPointsValue = it->pointsValue - observation.pointsValue;
            auto newSubset = subtractSets(it->haloPointSet, observation.haloPointSet);
            auto newMirrorSubset = subtractSets(it->extraMirroredHaloPointSet, observation.extraMirroredHaloPointSet);
            std::set<int> elementsToTransfer;
            for (int tileId: newMirrorSubset) {
                if (!contains(newSubset, tileId)){
                    elementsToTransfer.insert(tileId);
                }
            }

            for (const int& elem : elementsToTransfer) {
                newMirrorSubset.erase(elem);
                newSubset.insert(elem);
            }

            nextRecursionCycle.push_back(ConstraintObservation(newPointsValue, std::move(newSubset), std::move(newMirrorSubset)));

            log("Erasing the superset " + setToString(it->haloPointSet));
            it = masterSet.erase(it);
            iterated = true;
        } else if (observation.isSupersetObservation(*it)) {
            log("Superset found: " + setToString(observation.haloPointSet) + " is superset of " + setToString(it->haloPointSet));
            isSupersetFound = true;

            int newPointsValue = observation.pointsValue - it->pointsValue;
            auto newSuperset = subtractSets(observation.haloPointSet, it->haloPointSet);
            auto newMirrorSuperset = subtractSets(observation.extraMirroredHaloPointSet, it->extraMirroredHaloPointSet);

            std::set<int> elementsToTransfer;
            for (int tileId: newMirrorSuperset) {
                if (!contains(newSuperset, tileId)){
                    elementsToTransfer.insert(tileId);
                }
            }

            for (const int& elem : elementsToTransfer) {
                newMirrorSuperset.erase(elem);
                newSuperset.insert(elem);
            }

            nextRecursionCycle.push_back(ConstraintObservation(newPointsValue, std::move(newSuperset), std::move(newMirrorSuperset)));
        }

        // Move to the next element only if it is not already done during delete
        if (!iterated) {
            ++it; 
        }
    }
    
    if (observation.haloPointSet.size() != observation.pointsValue && observation.pointsValue != 0) {
        log("This observation is not a terminal, adding it to master set " + setToString(observation.haloPointSet));
        masterSet.emplace_back(observation);
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
