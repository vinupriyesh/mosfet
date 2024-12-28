#include <gtest/gtest.h>
#include "agent/game_map.h"
#include "agent/pathing.h"

class PathingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize the game map with a simple 3x3 grid
        gameMap = new GameMap(3, 3);
        pathing = new Pathing(gameMap);

        // Set up the tiles (for simplicity, all tiles are empty and movable)
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                gameMap->getTile(x, y).setType(TileType::EMPTY, 0);
            }
        }
    }

    void TearDown() override {
        delete pathing;
        delete gameMap;
    }

    GameMap* gameMap;
    Pathing* pathing;
};

TEST_F(PathingTest, FindAllPaths) {
    GameTile* startTile = &gameMap->getTile(0, 0);
    auto distances = pathing->findAllPaths(*startTile);

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
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}