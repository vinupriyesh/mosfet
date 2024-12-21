#include "control_center.h"
#include <iostream>

ControlCenter::ControlCenter(int n) : N(n) {
    shuttles = new Shuttle*[N];
    for (int i = 0; i < N; ++i) {
        shuttles[i] = new Shuttle();
    }
}

ControlCenter::~ControlCenter() {
    for (int i = 0; i < N; ++i) {
        delete shuttles[i];
    }
    delete[] shuttles;
}

std::vector<std::vector<int>> ControlCenter::act() {
    std::vector<std::vector<int>> results;
    for (int i = 0; i < N; ++i) {
        results.push_back(shuttles[i]->act());
    }
    return results;
}
