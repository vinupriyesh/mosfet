#include "game_map.h"


GameMap::GameMap(int width, int height) : width(width), height(height) {
    map.resize(height);
    for (int y = 0; y < height; ++y) {
        map[y].reserve(width);
        for (int x = 0; x < width; ++x) {
            map[y].emplace_back(x, y);
        }
    }
}