#include "relic.h"

Relic::Relic(int id, std::vector<int> position) {
    this->id = id;
    this->position = position;
}

std::vector<int> Relic::getMirroredPosition(int width, int height) {
    return { height - position[1] - 1, width - position[0] - 1};
}

void Relic::addDiscoveryId(int discoveryId) {
    discoveryIds.emplace_back(discoveryId);
}
