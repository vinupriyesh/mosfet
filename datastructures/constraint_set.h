#ifndef CONSTRAINT_SET_H
#define CONSTRAINT_SET_H

#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include <tuple>
#include <sstream>

#include "logger.h"


struct ConstraintObservation {
    int pointsValue;
    std::set<int> haloPointSet;

    // Constructor accepting const reference
    ConstraintObservation(int pv, const std::set<int>& hps)
        : pointsValue(pv), haloPointSet(hps) {}

    // Move constructor
    ConstraintObservation(int pv, std::set<int>&& hps)
        : pointsValue(pv), haloPointSet(std::move(hps)) {}
};

class ConstraintSet {
    private:
        void log(const std::string message) const;
      

        std::vector<ConstraintObservation> masterSet;

        void addSet(int pointsValue, const std::set<int>& newSet);
        // std::tuple<bool, ConstraintObservation&> isSubset(const std::set<int> &querySet);

        // std::tuple<bool, ConstraintObservation&> isSuperset(const std::set<int> &querySet);
        void deleteObservation(const ConstraintObservation &observation);
        void addConstraint(const ConstraintObservation &observation);
    public:
        std::set<int> identifiedVantagePoints;
        std::set<int> identifiedRegularTiles;

        void clear();        
        void addConstraint(int, const std::set<int>&);

        std::vector<ConstraintObservation> getMasterSet() {
            return masterSet;
        }

       
        void logMasterSet() const;
};

#endif // CONSTRAINT_SET_H