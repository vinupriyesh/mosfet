#include "datastructures/respawn_registry.h"
#include "logger.h"

#include <gtest/gtest.h>

class RespawnRegistryTest : public ::testing::Test {
    protected:        
        void SetUp() override {
            Logger::getInstance().enableLogging("../../test.log");

        }

        void TearDown() override {            
        }
};

TEST_F(RespawnRegistryTest, SimpleInsert) {
    Logger::getInstance().setPlayerName("SimpleInsert");

    RespawnRegistry registry;
    
    int step = 1;
    int verification = step;
    for (int i = 0; i < 16; i ++) {
        int playerRespawnStep = registry.pushPlayerUnit(i, step);
        int opponentRespawnStep = registry.pushOpponentUnit(i, step);

        EXPECT_EQ(playerRespawnStep, verification);
        EXPECT_EQ(opponentRespawnStep, verification);
        verification += 3;
    }

    step = 64;
    int respawnStep = registry.pushOpponentUnit(5, step);

    EXPECT_EQ(respawnStep, 67);
    EXPECT_EQ(registry.getOpponentUnitThatCanSpawnAtStep(28), 9);
    EXPECT_EQ(registry.getPlayerUnitThatCanSpawnAtStep(37), 12);
    EXPECT_EQ(registry.getOpponentUnitThatCanSpawnAtStep(67), 5);
    EXPECT_EQ(registry.getPlayerUnitThatCanSpawnAtStep(67), -1);

    registry.reset();
    step = 101;

    verification = step;
    for (int i = 0; i < 16; i ++) {
        int playerRespawnStep = registry.pushPlayerUnit(i, step);
        int opponentRespawnStep = registry.pushOpponentUnit(i, step);

        EXPECT_EQ(playerRespawnStep, verification);
        EXPECT_EQ(opponentRespawnStep, verification);
        verification += 3;
    }    

    registry.reset();
    step = 202;

    verification = step;
    for (int i = 0; i < 16; i ++) {
        int playerRespawnStep = registry.pushPlayerUnit(i, step);
        int opponentRespawnStep = registry.pushOpponentUnit(i, step);

        EXPECT_EQ(playerRespawnStep, verification);
        EXPECT_EQ(opponentRespawnStep, verification);        
        verification += 3;
    }

    step = 214;
    int playerRespawnStep = registry.pushPlayerUnit(1, step); // 1 dead at 214 step
    EXPECT_EQ(playerRespawnStep, verification);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}