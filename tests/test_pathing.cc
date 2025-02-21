#include <gtest/gtest.h>
#include "agent/game_map.h"
#include "agent/pathing.h"
#include "parser.h"
#include "agent/control_center.h"

std::string seed_2087279490 = R"(
{"obs": {"units": {"position": [[[-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1]], [[-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1]]], "energy": [[-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1], [-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1]]}, "units_mask": [[false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false], [false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false]], "sensor_mask": [[true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true], [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true]], "map_features":{"energy":[[-6,-4,2,7,10,8,5,1,-3,-4,-5,-5,-4,-2,0,1,2,1,-1,-4,-5,-2,4,9],[-4,2,8,9,7,1,-3,-6,-6,-5,-4,-3,-2,-1,0,2,4,5,4,0,-5,-6,-2,4],[1,7,9,6,0,-5,-6,-4,-1,2,4,5,4,3,2,2,4,6,8,6,1,-5,-6,-2],[6,10,7,0,-5,-5,-2,3,7,9,9,9,7,5,3,0,0,3,7,10,7,1,-5,-5],[9,8,1,-5,-5,-1,5,9,9,8,5,4,4,3,2,-1,-3,-3,2,7,10,6,0,-4],[10,5,-3,-6,-2,5,9,8,4,-1,-3,-3,-2,1,2,1,-3,-6,-4,2,7,8,4,-1],[8,1,-6,-4,3,9,8,2,-3,-6,-6,-5,-3,1,4,5,2,-3,-6,-3,3,6,5,1],[6,-3,-6,-1,7,9,4,-3,-6,-4,0,1,1,2,6,9,8,2,-3,-3,0,4,4,2],[4,-4,-5,2,9,8,-1,-6,-4,3,8,7,3,1,3,8,9,5,1,-1,0,2,2,1],[2,-5,-4,4,9,5,-3,-6,0,8,9,8,1,-4,-2,3,6,4,2,2,3,2,0,0],[1,-5,-3,5,9,4,-3,-5,1,7,8,3,-4,-6,-4,1,2,1,1,3,5,3,-1,-2],[1,-4,-2,4,7,4,-2,-3,1,3,1,-4,-6,-4,1,3,1,-3,-2,4,7,4,-2,-4],[2,-2,-1,3,5,3,1,1,2,1,-4,-6,-4,3,8,7,1,-5,-3,4,9,5,-3,-5],[2,0,0,2,3,2,2,4,6,3,-2,-4,1,8,9,8,0,-6,-3,5,9,4,-4,-5],[2,1,2,2,0,-1,1,5,9,8,3,1,3,7,8,3,-4,-6,-1,8,9,2,-5,-4],[1,2,4,4,0,-3,-3,2,8,9,6,2,1,1,0,-4,-6,-3,4,9,7,-1,-6,-3],[0,1,5,6,3,-3,-6,-3,2,5,4,1,-3,-5,-6,-6,-3,2,8,9,3,-4,-6,1],[-2,-1,4,8,7,2,-4,-6,-3,1,2,1,-2,-3,-3,-1,4,8,9,5,-2,-6,-3,5],[-2,-4,0,6,10,7,2,-3,-3,-1,2,3,4,4,5,8,9,9,5,-1,-5,-5,1,8],[0,-5,-5,1,7,10,7,3,0,0,3,5,7,9,9,9,7,3,-2,-5,-5,0,7,10],[5,-2,-6,-5,1,6,8,6,4,2,2,3,4,5,4,2,-1,-4,-6,-5,0,6,9,7],[9,4,-2,-6,-5,0,4,5,4,2,0,-1,-2,-3,-4,-5,-6,-6,-3,1,7,9,8,2],[9,9,4,-2,-5,-4,-1,1,2,1,0,-2,-4,-5,-5,-4,-3,1,5,8,10,7,2,-4],[4,9,9,5,0,-2,-2,0,1,2,2,2,1,1,2,4,6,8,10,9,6,1,-4,-6]],"tile_type":[[0,0,0,2,0,0,0,2,0,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0],[0,0,2,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,2,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,2,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,1,1,1,0,0,0,2,0,0,0,0,0,0,0,0,2],[0,0,0,0,0,0,0,2,1,1,2,2,1,1,0,0,0,0,0,0,0,0,0,0],[0,2,0,0,0,0,0,2,2,1,2,2,1,1,1,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,2,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0],[0,2,2,2,0,0,0,0,0,0,0,0,0,0,1,2,2,0,0,0,0,2,0,0],[1,2,2,2,2,0,0,0,0,0,0,0,0,0,1,2,2,1,0,0,0,2,0,0],[1,1,0,0,0,2,0,0,0,0,0,0,0,0,0,1,1,1,0,2,2,0,1,1],[1,0,0,0,0,2,0,0,0,0,0,0,0,0,0,2,1,1,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,2],[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,2,0,0,0],[0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,2,2],[0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,2,0,0,2,0],[0,0,0,0,0,0,0,0,0,1,2,2,0,0,0,2,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0]]}, "relic_nodes": [[-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1], [-1, -1]], "relic_nodes_mask": [false, false, false, false, false, false], "team_points": [0, 0], "team_wins": [0, 0], "steps": 0, "match_steps": 0}, "step": 0, "remainingOverageTime": 600, "player": "player_0", "info": {"env_cfg": {"max_units": 16, "match_count_per_episode": 5, "max_steps_in_match": 100, "map_height": 24, "map_width": 24, "num_teams": 2, "unit_move_cost": 4, "unit_sap_cost": 31, "unit_sap_range": 5, "unit_sensor_range": 3}}}
    )";

class PathingTest : public ::testing::Test {
    
protected:
    void SetUp() override {
         Logger::getInstance().enableLogging("../../test.log");

        // Initialize the game map with a simple 3x3 grid
        gameMap = new GameMap(3, 3);        

        // Set up the tiles (for simplicity, all tiles are empty and movable) 
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                gameMap->getTile(x, y).setType(TileType::EMPTY, 0, false);
            }
        }
    }

    void TearDown() override {
        delete gameMap;
    }

    GameMap* gameMap;
    
};


TEST_F(PathingTest, FindAllPaths) {

    PathingConfig config = {};
    config.pathingHeuristics = SHORTEST_DISTANCE;
    config.stopAtUnexploredTiles = false;
    config.captureUnexploredTileDestinations = false;

    Pathing* pathing = new Pathing(*gameMap, config);
    GameTile* startTile = &gameMap->getTile(0, 0);
    pathing->findAllPaths(*startTile);
    auto distances = pathing->distances;

    // Check the distances from the start tile to all other tiles
    EXPECT_EQ(distances[&gameMap->getTile(0, 0)].first, 0);
    EXPECT_EQ(distances[&gameMap->getTile(1, 0)].first, 1);
    EXPECT_EQ(distances[&gameMap->getTile(2, 0)].first, 2);
    EXPECT_EQ(distances[&gameMap->getTile(0, 1)].first, 1);
    EXPECT_EQ(distances[&gameMap->getTile(1, 1)].first, 2);
    EXPECT_EQ(distances[&gameMap->getTile(2, 1)].first, 3);
    EXPECT_EQ(distances[&gameMap->getTile(0, 2)].first, 2);
    EXPECT_EQ(distances[&gameMap->getTile(1, 2)].first, 3);
    EXPECT_EQ(distances[&gameMap->getTile(2, 2)].first, 4);

    // Check the paths from the start tile to a specific tile
    std::vector<GameTile*> expectedPath = {startTile, &gameMap->getTile(1, 0), &gameMap->getTile(2, 0)};
    EXPECT_EQ(distances[&gameMap->getTile(2, 0)].second, expectedPath);
    delete pathing;
}

TEST_F(PathingTest, FindAllPathsWithStopAtExploredTiles) {
    // Set the stopAtExploredTiles flag
    PathingConfig config = {};
    config.pathingHeuristics = SHORTEST_DISTANCE;
    config.stopAtUnexploredTiles = false;
    config.captureUnexploredTileDestinations = false;

    Pathing* pathing = new Pathing(*gameMap, config);

    // Mark some tiles as explored
    gameMap->getTile(0, 0).setExplored(true, 0);
    gameMap->getTile(1, 0).setExplored(true, 0);
    gameMap->getTile(1, 1).setExplored(true, 0);

    GameTile* startTile = &gameMap->getTile(0, 0);
    pathing->findAllPaths(*startTile);
    auto distances = pathing->distances;

    // Find the shortest tile that has explored == false
    GameTile* shortestUnexploredTile = nullptr;
    int shortestDistance = std::numeric_limits<int>::max();

    for (const auto& [tile, distancePathPair] : distances) {
        if (!tile->isExplored() && distancePathPair.first < shortestDistance) {
            shortestUnexploredTile = tile;
            shortestDistance = distancePathPair.first;
        }
    }

    // Check that the shortest unexplored tile is correct
    ASSERT_NE(shortestUnexploredTile, nullptr);
    EXPECT_EQ(shortestUnexploredTile, &gameMap->getTile(0, 1));
    EXPECT_EQ(shortestDistance, 1);
    delete pathing;
}

TEST_F(PathingTest, FindLeastEnergyPath) {
    
    GameState gameState = parse (seed_2087279490);
    ControlCenter* cc = new ControlCenter();
    cc->update(gameState); 

    PathingConfig config = {};
    config.pathingHeuristics = LEAST_ENERGY;
    config.stopAtUnexploredTiles = false;
    config.captureUnexploredTileDestinations = false;

    Pathing* pathing = new Pathing(*cc->gameMap, config);

    GameTile* startTile = &cc->gameMap->getTile(11, 3);
    pathing->findAllPaths(*startTile);
    auto distances = pathing->distances;

    Logger::getInstance().log("Parsed value -> " + std::to_string(gameState.obs.mapFeatures.tileType[4][2]));
    Logger::getInstance().log("Type of (4,2) is " + std::to_string(cc->gameMap->getTile(4, 2).getType()));
    EXPECT_EQ(gameState.obs.mapFeatures.tileType[4][2], 2);
    EXPECT_EQ(cc->gameMap->getTile(4, 2).getType(), 3);

    for (const auto& [tile, distancePathPair] : distances) {
        // Logger::getInstance().log("Distance to ( " + std::to_string(tile->x) + "," + std::to_string(tile->y) + ") is " + std::to_string(distancePathPair.first));
        if (tile->x == 5 && tile->y == 8) {
            Logger::getInstance().log("distance to (5, 8) is " + std::to_string(distancePathPair.first));

            EXPECT_EQ(distancePathPair.first, 65);

            const std::vector<std::tuple<int, int>> idealPath = { {11, 3}, {11, 4}, {10, 4}, {9, 4}, {8, 4}, {8, 5}, {7, 5}, {6, 5}, {6, 6}, {5, 6}, {5, 7}, {5, 8}};
            int idx = 0;
            for (GameTile* pathTile : distancePathPair.second) {                
                Logger::getInstance().log("Step ( " + std::to_string(pathTile->x) + "," + std::to_string(pathTile->y) + "), tileType - " + std::to_string(pathTile->getType()) 
                + ", energy -> " + std::to_string(pathTile->getEnergy()));
                EXPECT_EQ(pathTile->x, std::get<0>(idealPath[idx]));
                EXPECT_EQ(pathTile->y, std::get<1>(idealPath[idx]));
                idx++;
            }
        }  
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}