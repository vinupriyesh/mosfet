#include <gtest/gtest.h>
#include "agent/game_map.h"
#include "agent/pathing.h"

class PathingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize the game map with a simple 3x3 grid
        gameMap = new GameMap(3, 3);        

        // Set up the tiles (for simplicity, all tiles are empty and movable)
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                gameMap->getTile(x, y).setType(TileType::EMPTY, 0);
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

    Pathing* pathing = new Pathing(gameMap, config);
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

    Pathing* pathing = new Pathing(gameMap, config);

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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}