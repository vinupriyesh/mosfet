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

struct GameState {
    Info info;
};

// Function declarations
void from_json(const json& j, Info& i);
void from_json(const json& j, GameState& g);

std::ostream& operator<<(std::ostream& os, const Info& info);
std::ostream& operator<<(std::ostream& os, const GameState& gameState);

std::string to_string(const GameState& gameState);
GameState parse(const std::string& input);

#endif // PARSER_H
