#ifndef CONSTANTS_H
#define CONSTANTS_H

const double LOWEST_DOUBLE = 1e-9;

const int UNIT_SPAWN_ENERGY = 100;

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


const int POSSIBLE_NEBULA_ENERGY_REDUCTION_VALUES[] = {0, 1, 2, 3, 5, 25};
const int POSSIBLE_NEBULA_ENERGY_REDUCTION_VALUES_SIZE = 6;

const float POSSIBLE_UNIT_ENERGY_VOID_FACTOR_VALUES[] = {0.0625, 0.125, 0.25, 0.375};
const int POSSIBLE_UNIT_ENERGY_VOID_FACTOR_VALUES_SIZE = 4;

const float POSSIBLE_UNIT_SAP_DROP_OFF_FACTOR_VALUES[] = {0.25, 0.5, 1.0};
const float POSSIBLE_UNIT_SAP_DROP_OFF_FACTOR_VALUES_SIZE = 3;

#endif //CONSTANTS_H