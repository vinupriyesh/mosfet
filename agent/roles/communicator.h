#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <set>

#include "logger.h"
#include "metrics.h"

class Communicator {
    public:
        std::set<int> attackingTileIds;

};

#endif //COMMUNICATOR_H