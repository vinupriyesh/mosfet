#ifndef CONSTANTS_H
#define CONSTANTS_H

const double LOWEST_DOUBLE = 1e-9;

const int POSSIBLE_MOVE_SIZE = 5;
const int POSSIBLE_MOVES[5][2] = {
    {0, 0},  //CENTER
    {1, 0}, //RIGHT
    {-1, 0}, //LEFT
    {0, 1}, //DOWN
    {0, -1} //UP
};


const int POSSIBLE_NEIGHBORS_SIZE = 4;
const int POSSIBLE_NEIGHBORS[4][2] = {
    {1, 0}, //RIGHT
    {-1, 0}, //LEFT
    {0, 1}, //DOWN
    {0, -1} //UP
};

#endif //CONSTANTS_H