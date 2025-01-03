#include "datastructures/constraint_set.h"
#include <gtest/gtest.h>

class ConstraintSetTest : public ::testing::Test {
protected:
    ConstraintSet constraintSet;

    void SetUp() override {
        Logger::getInstance().enableLogging("../../test.log");
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(ConstraintSetTest, SimpleAddConstraint) {
    // 1 and 2 are vantage points
    // 3 is a regular tile
    // 4 and 5 is still uncertain, one of them is a vantage point
    std::set<int> haloPointSet1 = {1, 2, 3};
    std::set<int> haloPointSet2 = {2, 1};
    std::set<int> haloPointSet3 = {4, 5};

    constraintSet.addConstraint(2, haloPointSet1);
    constraintSet.addConstraint(2, haloPointSet2);
    constraintSet.addConstraint(1, haloPointSet3);

    EXPECT_EQ(constraintSet.identifiedRegularTiles.size(), 1);
    EXPECT_EQ(constraintSet.identifiedVantagePoints.size(), 2);

    if (constraintSet.identifiedRegularTiles.size() == 1) {
        EXPECT_EQ(constraintSet.identifiedRegularTiles[0], 3);
    }

    if (constraintSet.identifiedVantagePoints.size() == 2) {
        EXPECT_TRUE(constraintSet.identifiedVantagePoints[0] == 1 || constraintSet.identifiedVantagePoints[1] == 1);
        EXPECT_TRUE(constraintSet.identifiedVantagePoints[0] == 2 || constraintSet.identifiedVantagePoints[1] == 2);
    }

    auto masterSet = constraintSet.getMasterSet();

    EXPECT_EQ(masterSet.size(), 1);

    constraintSet.logMasterSet();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}