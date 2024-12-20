#include <iostream>
#include <string> 
#include <vector> 
#include <nlohmann/json.hpp>

using json = nlohmann::json;


struct Info {
    std::map<std::string, int> envCfg;
};

struct GameState {
    Info info;
};


void from_json(const json& j, Info& i) {
    j.at("env_cfg").get_to(i.envCfg);
}

void from_json(const json& j, GameState& g) {
    j.at("info").get_to(g.info);
}


std::ostream& operator<<(std::ostream& os, const Info& info) {
    os << "\nenvCfg: \n";
    for (const auto& [key, value] : info.envCfg) {
        os << key << ": " << value << "\n";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const GameState& gameState) {
    os << "info: " << gameState.info;
    return os;
}

std::string to_string(const GameState& gameState) {
    std::ostringstream oss; oss << gameState;
     return oss.str(); 
}

GameState parse(std::string input) {
    json jsonObject = json::parse(input);
    return jsonObject.get<GameState>();
}