#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Info {
    std::map<std::string, int> envCfg;
};

struct Units {
    std::vector<std::vector<std::vector<int>>> position;
    std::vector<std::vector<int>> energy;
};

struct MapFeatures {
    std::vector<std::vector<int>> energy;
    std::vector<std::vector<int>> tileType;
};

struct Obs {
    Units units;
    std::vector<std::vector<int>> unitsMask;
    std::vector<std::vector<int>> sensorMask;
    MapFeatures mapFeatures;
    std::vector<int> relicNodesMask;
    std::vector<std::vector<int>> relicNodes;
    std::vector<int> teamPoints;
    std::vector<int> teamWins;
    int steps;
    int matchSteps;
};

struct GameState {
    Obs obs;
    int remainingOverageTime;
    std::string player;
    Info info;
};

// Function declarations
void from_json(const json& j, Info& i);
void from_json(const json& j, Units& u);
void from_json(const json& j, MapFeatures& m);
void from_json(const json& j, Obs& m);
void from_json(const json& j, GameState& g);

std::ostream& operator<<(std::ostream& os, const Info& info);
std::ostream& operator<<(std::ostream& os, const Units& units);
std::ostream& operator<<(std::ostream& os, const MapFeatures& mapFeatures);
std::ostream& operator<<(std::ostream& os, const Obs& obs);
std::ostream& operator<<(std::ostream& os, const GameState& gameState);

std::string to_string(const GameState& gameState);
GameState parse(const std::string& input);

#endif // PARSER_H
