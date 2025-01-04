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

TEST_F(ConstraintSetTest, TestSubsetThatCanResolve) {
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

    EXPECT_NE(constraintSet.identifiedRegularTiles.find(3), constraintSet.identifiedRegularTiles.end());    

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(1), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(2), constraintSet.identifiedVantagePoints.end());    

    auto masterSet = constraintSet.getMasterSet();

    EXPECT_EQ(masterSet.size(), 1);

    constraintSet.logMasterSet();
}

TEST_F(ConstraintSetTest, TestaSubsetThatCannotResolve) {
    
    std::set<int> haloPointSet1 = {1, 2, 3};
    std::set<int> haloPointSet2 = {2, 1};
    std::set<int> haloPointSet3 = {4, 5};
    std::set<int> haloPointSet4 = {1, 2};
    std::set<int> haloPointSet5 = {2, 1};
    std::set<int> haloPointSet6 = {4, 5};  

    constraintSet.addConstraint(2, haloPointSet1);
    constraintSet.addConstraint(1, haloPointSet2);
    constraintSet.addConstraint(1, haloPointSet3);
    constraintSet.addConstraint(1, haloPointSet4);
    constraintSet.addConstraint(1, haloPointSet5);
    constraintSet.addConstraint(1, haloPointSet6);

    EXPECT_EQ(constraintSet.identifiedRegularTiles.size(), 0);
    EXPECT_EQ(constraintSet.identifiedVantagePoints.size(), 1);

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(3), constraintSet.identifiedVantagePoints.end());    

    auto masterSet = constraintSet.getMasterSet();

    EXPECT_EQ(masterSet.size(), 2);

    constraintSet.logMasterSet();
}

TEST_F(ConstraintSetTest, TestSupersetThatCanResolve) {
    
    std::set<int> haloPointSet1 = {1, 2, 3}; //1
    std::set<int> haloPointSet2 = {1, 3, 4, 5, 2}; //2
    std::set<int> haloPointSet3 = {4, 5}; //1
    std::set<int> haloPointSet4 = {4, 6}; //1
    std::set<int> haloPointSet5 = {1, 2, 3, 6}; //2
    

    constraintSet.addConstraint(1, haloPointSet1);
    constraintSet.addConstraint(2, haloPointSet2);
    constraintSet.addConstraint(1, haloPointSet3);
    constraintSet.addConstraint(1, haloPointSet4);
    constraintSet.addConstraint(2, haloPointSet5);

    // Should have identified
    // 6 -> vantage point
    // 4 -> regular tile
    // 5 -> vantage point

    EXPECT_EQ(constraintSet.identifiedRegularTiles.size(), 1);
    EXPECT_EQ(constraintSet.identifiedVantagePoints.size(), 2);

    EXPECT_NE(constraintSet.identifiedRegularTiles.find(4), constraintSet.identifiedRegularTiles.end());    

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(6), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(5), constraintSet.identifiedVantagePoints.end());    

    auto masterSet = constraintSet.getMasterSet();

    EXPECT_EQ(masterSet.size(), 1);

    constraintSet.logMasterSet();

    if (masterSet.size() == 1) {
        auto observation = masterSet[0];
        EXPECT_EQ(observation.pointsValue, 1);
        EXPECT_EQ(observation.haloPointSet.size(), 3);
        EXPECT_NE(observation.haloPointSet.find(1), observation.haloPointSet.end());
        EXPECT_NE(observation.haloPointSet.find(2), observation.haloPointSet.end());
        EXPECT_NE(observation.haloPointSet.find(3), observation.haloPointSet.end());
    }

    // Lets continue the exploration
    constraintSet.clear();

    std::set<int> haloPointSet6 = {2}; //1 - This should close everything

    constraintSet.addConstraint(1, haloPointSet6);

    // Should have identified
    // 2 -> vantage point
    // 1, 3 -> regular tile

    EXPECT_EQ(constraintSet.identifiedRegularTiles.size(), 2);
    EXPECT_EQ(constraintSet.identifiedVantagePoints.size(), 1);

    EXPECT_NE(constraintSet.identifiedRegularTiles.find(1), constraintSet.identifiedRegularTiles.end());
    EXPECT_NE(constraintSet.identifiedRegularTiles.find(3), constraintSet.identifiedRegularTiles.end());

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(2), constraintSet.identifiedVantagePoints.end());

    masterSet = constraintSet.getMasterSet();

    EXPECT_EQ(masterSet.size(), 0);

    constraintSet.logMasterSet();

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}