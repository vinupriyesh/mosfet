#include "parser.h"

// Define from_json for Info
void from_json(const json& j, Info& i) {
    if (j.contains("env_cfg")) {
        j.at("env_cfg").get_to(i.envCfg);
    }
}

void from_json(const json &j, Units &u) {
    if (j.contains("position")) {
        j.at("position").get_to(u.position);
    }
    if (j.contains("energy")) {
        j.at("energy").get_to(u.energy);
    }
}

void from_json(const json &j, MapFeatures &m) {
    if (j.contains("energy")) {
        j.at("energy").get_to(m.energy);
    }
    if (j.contains("tile_type")) {
        j.at("tile_type").get_to(m.tileType);
    }
}

void from_json(const json &j, Obs &m) {
    if (j.contains("units")) {
        j.at("units").get_to(m.units);
    }
    if (j.contains("units_mask")) {
        j.at("units_mask").get_to(m.unitsMask);
    }
    if (j.contains("sensor_mask")) {
        j.at("sensor_mask").get_to(m.sensorMask);
    }
    if (j.contains("map_features")) {
        j.at("map_features").get_to(m.mapFeatures);
    }
    if (j.contains("relic_nodes_mask")) {
        j.at("relic_nodes_mask").get_to(m.relicNodesMask);
    }
    if (j.contains("relic_nodes")) {
        j.at("relic_nodes").get_to(m.relicNodes);
    }
    if (j.contains("team_points")) {
        j.at("team_points").get_to(m.teamPoints);
    }
    if (j.contains("team_wins")) {
        j.at("team_wins").get_to(m.teamWins);
    }
    if (j.contains("steps")) {
        j.at("steps").get_to(m.steps);
    }
    if (j.contains("match_steps")) {
        j.at("match_steps").get_to(m.matchSteps);
    }
}

// Define from_json for GameState
void from_json(const json& j, GameState& g) {
    if (j.contains("obs")) {
        j.at("obs").get_to(g.obs);
    }
    if (j.contains("remainingOverageTime")) {
        j.at("remainingOverageTime").get_to(g.remainingOverageTime);
    }
    if (j.contains("player")) {
        j.at("player").get_to(g.player);
    }
    if (j.contains("info")) {
        j.at("info").get_to(g.info);
    }
}

// Define output operator for Info
std::ostream& operator<<(std::ostream& os, const Info& info) {
    os << "\n\tenvCfg: \n";
    for (const auto& [key, value] : info.envCfg) {
        os << "\t\t" << key << ": " << value << "\n";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Units &units) {
    os << "\n\tposition: \n";
    for (const auto& position : units.position) {
        os << "\t\t[";
        for (const auto& pos : position) {
            os << "[";
            for (const auto& p : pos) {
                os << p << ", ";
            }
            os << "], ";
        }
        os << "], ";
    }
    os << "\n\tenergy: \n";
    for (const auto& e1 : units.energy) {
        os << "\t\t[";
        for (const auto& e2 : e1) {
            os << e2 << ", ";
        }
        os << "], ";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const MapFeatures &mapFeatures) {
    os << "\n\tenergy: \n";
    for (const auto& energy : mapFeatures.energy) {
        os << "\t\t[";
        for (const auto& e : energy) {
            os << e << ", ";
        }
        os << "], ";
    }
    os << "\n\ttileType: \n";
    for (const auto& tileType : mapFeatures.tileType) {
        os << "\t\t[";
        for (const auto& t : tileType) {
            os << t << ", ";
        }
        os << "], ";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Obs &obs) {
    os << "\n\tunits: " << obs.units;
    os << "\n\tunitsMask: \n";
    for (const auto& unitsMask : obs.unitsMask) {
        os << "\t\t[";
        for (const auto& u : unitsMask) {
            os << u << ", ";
        }
        os << "], ";
    }
    os << "\n\tsensorMask: \n";
    for (const auto& sensorMask : obs.sensorMask) {
        os << "\t\t[";
        for (const auto& s : sensorMask) {
            os << s << ", ";
        }
        os << "], ";
    }
    os << "\n\tmapFeatures: " << obs.mapFeatures;
    os << "\n\trelicNodesMask: \n";
    for (const auto& relicNodesMask : obs.relicNodesMask) {
        os << "\t\t" << relicNodesMask << ", ";
    }
    os << "\n\trelicNodes: \n";
    for (const auto& relicNodes : obs.relicNodes) {
        os << "\t\t[";
        for (const auto& r : relicNodes) {
            os << r << ", ";
        }
        os << "], ";
    }
    os << "\n\tteamPoints: \n";
    for (const auto& teamPoints : obs.teamPoints) {
        os << "\t\t" << teamPoints << ", ";
    }
    os << "\n\tteamWins: \n";
    for (const auto& teamWins : obs.teamWins) {
        os << "\t\t" << teamWins << ", ";
    }
    os << "\n\tsteps: " << obs.steps << "\n";
    os << "\tmatchSteps: " << obs.matchSteps << "\n";
    return os;
}

// Define output operator for GameState
std::ostream& operator<<(std::ostream& os, const GameState& gameState) {
    os << "obs: " << gameState.obs << "\n";
    os << "remainingOverageTime: " << gameState.remainingOverageTime << "\n";
    os << "player: " << gameState.player << "\n";
    os << "info: " << gameState.info << "\n";    
    return os;
}

// Define to_string for GameState
std::string to_string(const GameState& gameState) {
    std::ostringstream oss;
    oss << "\n" << gameState;
    return oss.str();
}

// Define parse function
GameState parse(const std::string& input) {
    json jsonObject = json::parse(input);
    return jsonObject.get<GameState>();
}
