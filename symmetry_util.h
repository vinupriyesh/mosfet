#ifndef SYMMETRY_UTILS_H
#define SYMMETRY_UTILS_H

#include <utility>

#include "game_env_config.h"

namespace SymmetryUtils {
    inline int toID(int x, int y) {
        return y * GameEnvConfig::getInstance().mapWidth + x;
    }    

    inline void toXY(int id, int &x, int &y) {
        int& width = GameEnvConfig::getInstance().mapWidth;
        y = id / width;
        x = id % width;
    }

    inline std::pair<int, int> toXY(int id) {
        int x, y;
        toXY(id, x, y);
        return std::make_pair(x, y);
    }

    inline void toMirroredXY(int x, int y, int &xMir, int &yMir) {
        int& width = GameEnvConfig::getInstance().mapWidth;
        int& height = GameEnvConfig::getInstance().mapHeight;
        xMir = height - y - 1;
        yMir = width - x - 1;
    }

    inline int toMirroredID(int id) {
        int x, y;
        toXY(id, x, y);
        toMirroredXY(x, y, x, y);
        return toID(x, y);
    }

    inline int toMirroredID(int x , int y) {
        toMirroredXY(x, y, x, y);
        return toID(x, y);
    }

    inline int toFirstHalfID(int id) {
        int& width = GameEnvConfig::getInstance().mapWidth;
        int x, y;
        toXY(id, x, y);
        if (x + y >= width) {
            //This is in second half, return mirror
            return toMirroredID(x, y);
        } else {
            //Already a first half id
            return id;
        }
    }

    inline bool isFirstHalfID(int id) {
        int& width = GameEnvConfig::getInstance().mapWidth;
        int x, y;
        toXY(id, x, y);
        return x + y < width;
    }
}

#endif // SYMMETRY_UTILS_H
