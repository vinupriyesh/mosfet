#ifndef CONSTRAINT_SET_H
#define CONSTRAINT_SET_H

#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include <tuple>
#include <sstream>

#include "metrics.h"
#include "logger.h"
#include "config.h"
#include "game_env_config.h"
#include "symmetry_util.h"


class ConstraintObservation {

    private:
        void log(const std::string message) const;

    public:
        int pointsValue;
        std::set<int> haloPointSet;
        std::set<int> extraMirroredHaloPointSet;
        

        // Constructor accepting const reference
        ConstraintObservation(int pv, const std::set<int>& hps);

        bool simplify(std::vector<ConstraintObservation> &nextRecursionCycle) const;

        // Move constructor
        ConstraintObservation(int pv, std::set<int>&& hps)
            : pointsValue(pv), haloPointSet(std::move(hps)) {}

        // Constructor accepting const reference
        ConstraintObservation(int pv, const std::set<int>& hps, const std::set<int>& mhps)
            : pointsValue(pv), haloPointSet(hps), extraMirroredHaloPointSet(mhps) {}

        // Move constructor
        ConstraintObservation(int pv, std::set<int>&& hps, std::set<int>&& mhps)
            : pointsValue(pv), haloPointSet(std::move(hps)), extraMirroredHaloPointSet(std::move(mhps)) {}

        void insertAllMirrors(std::set<int>& target) const;

        std::string toString() const;

        bool isValid() const;
        void collectRegularAndVantagePoints(std::set<int> & identifiedNormalTiles, std::set<int> & identifiedVantagePoints) const;

        bool isSubsetObservation(ConstraintObservation& other) const;
        bool isSupersetObservation(ConstraintObservation& other) const;
};

class ConstraintSet {
    private:

        void log(const std::string message) const;
        void pruneConstraints();

        std::vector<ConstraintObservation> masterSet;
        
        // std::tuple<bool, ConstraintObservation&> isSubset(const std::set<int> &querySet);

        // std::tuple<bool, ConstraintObservation&> isSuperset(const std::set<int> &querySet);
        void addConstraint(const ConstraintObservation &observation);
        void phaseOutOlderConstraints(int tileId);
    public:
        std::set<int> identifiedVantagePoints;
        std::set<int> identifiedRegularTiles;

        void clear();
        void addConstraint(int, std::set<int>&);
        void reconsiderNormalizedTile(int tileId);
        void reconsiderNormalizedTile(std::vector<int> tileIds);

        std::vector<ConstraintObservation> getMasterSet() {
            return masterSet;
        }
       
        void logMasterSet() const;
};

#endif // CONSTRAINT_SET_H