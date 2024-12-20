#include "parser.h"

// Define from_json for Info
void from_json(const json& j, Info& i) {
    j.at("env_cfg").get_to(i.envCfg);
}

// Define from_json for GameState
void from_json(const json& j, GameState& g) {
    j.at("info").get_to(g.info);
}

// Define output operator for Info
std::ostream& operator<<(std::ostream& os, const Info& info) {
    os << "\nenvCfg: \n";
    for (const auto& [key, value] : info.envCfg) {
        os << key << ": " << value << "\n";
    }
    return os;
}

// Define output operator for GameState
std::ostream& operator<<(std::ostream& os, const GameState& gameState) {
    os << "info: " << gameState.info;
    return os;
}

// Define to_string for GameState
std::string to_string(const GameState& gameState) {
    std::ostringstream oss;
    oss << gameState;
    return oss.str();
}

// Define parse function
GameState parse(const std::string& input) {
    json jsonObject = json::parse(input);
    return jsonObject.get<GameState>();
}
