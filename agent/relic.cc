#include "relic.h"

Relic::Relic(int id) {
    this->id = id;
    this->isVisible = false;
}

void Relic::updateRelicData(std::vector<int> position, bool isVisible) {
    this->position = position;
    this->isVisible = isVisible;
}