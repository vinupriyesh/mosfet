#include "relic.h"

Relic::Relic(int id) {
    this->id = id;
    this->visible = false;
    this->revealed = false;
}

bool Relic::updateRelicData(std::vector<int> position, bool visible) {

    bool firstTimeReveal = false;

    // Relics dont change positions.  So once found, dont forget them!
    if (position[0] != -1 && position[1] != -1 && !this->revealed) {
        this->position = position;
        this->revealed = true;  
        firstTimeReveal = true;
    }
    
    this->visible = visible;
    return firstTimeReveal;
}