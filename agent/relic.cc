#include "relic.h"

Relic::Relic(int id) {
    this->id = id;
    this->visible = false;
    this->revealed = false;
}

void Relic::updateRelicData(std::vector<int> position, bool visible) {

    // Relics dont change positions.  So once found, dont forget them!
    if (position[0] != -1 && position[1] != -1) {
        this->position = position;
        this->revealed = true;  
    }
    
    this->visible = visible;
}