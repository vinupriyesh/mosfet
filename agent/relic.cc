#include "relic.h"

Relic::Relic(int id) {
    this->id = id;
    this->visible = false;
}

void Relic::updateRelicData(std::vector<int> position, bool visible) {
    this->position = position;
    this->visible = visible;
}