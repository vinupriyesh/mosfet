#include "datastructures/constraint_set.h"
#include <gtest/gtest.h>
#include "game_env_config.h"
#include "symmetry_util.h"    

class ConstraintSetTest : public ::testing::Test {
    
    protected:
        int p1, p2, p3, p4, p5, p6;
        int p1Mirr, p2Mirr, p3Mirr, p4Mirr;
        ConstraintSet constraintSet;

        void SetUp() override {
            Logger::getInstance().enableLogging("../../test.log");
            GameEnvConfig::getInstance().mapWidth = 24;
            GameEnvConfig::getInstance().mapHeight = 24;
            GameEnvConfig::getInstance().teamId = 1;

            p1 = symmetry_utils::toID(1, 1);
            p2 = symmetry_utils::toID(1, 2);
            p3 = symmetry_utils::toID(1, 3);
            p4 = symmetry_utils::toID(1, 4);
            p5 = symmetry_utils::toID(1, 5);
            p6 = symmetry_utils::toID(1, 6);

            p1Mirr = symmetry_utils::toMirroredID(p1);
            p2Mirr = symmetry_utils::toMirroredID(p2);
            p3Mirr = symmetry_utils::toMirroredID(p3);
            p4Mirr = symmetry_utils::toMirroredID(p4);
        }

        void TearDown() override {
            // Cleanup code if needed
        }
};

TEST_F(ConstraintSetTest, TestSubsetThatCanResolve) {
    Logger::getInstance().setPlayerName("TestSubsetThatCanResolve");
    // 1 and 2 are vantage points
    // 3 is a regular tile
    // 4 and 5 is still uncertain, one of them is a vantage point
    std::set<int> haloPointSet1 = {p1, p2, p3};
    std::set<int> haloPointSet2 = {p2, p1Mirr};
    std::set<int> haloPointSet3 = {p4Mirr, p5};

    constraintSet.addConstraint(2, haloPointSet1);
    constraintSet.addConstraint(2, haloPointSet2);
    constraintSet.addConstraint(1, haloPointSet3);

    EXPECT_EQ(constraintSet.identifiedRegularTiles.size(), 2);
    EXPECT_EQ(constraintSet.identifiedVantagePoints.size(), 4);

    EXPECT_NE(constraintSet.identifiedRegularTiles.find(p3), constraintSet.identifiedRegularTiles.end());
    EXPECT_NE(constraintSet.identifiedRegularTiles.find(symmetry_utils::toMirroredID(p3)), constraintSet.identifiedRegularTiles.end());

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p1), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p2), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(symmetry_utils::toMirroredID(p1)), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(symmetry_utils::toMirroredID(p2)), constraintSet.identifiedVantagePoints.end());

    auto masterSet = constraintSet.getMasterSet();

    EXPECT_EQ(masterSet.size(), 1);

    constraintSet.logMasterSet();
}

TEST_F(ConstraintSetTest, TestaSubsetThatCannotResolve) {
    Logger::getInstance().setPlayerName("TestaSubsetThatCannotResolve");
    std::set<int> haloPointSet1 = {p1, p2, p3};
    std::set<int> haloPointSet2 = {p2, p1};
    std::set<int> haloPointSet3 = {p4, p5};
    std::set<int> haloPointSet4 = {p1, p2};
    std::set<int> haloPointSet5 = {p2, p1};
    std::set<int> haloPointSet6 = {p4, p5};  

    constraintSet.addConstraint(2, haloPointSet1);
    constraintSet.addConstraint(1, haloPointSet2);
    constraintSet.addConstraint(1, haloPointSet3);
    constraintSet.addConstraint(1, haloPointSet4);
    constraintSet.addConstraint(1, haloPointSet5);
    constraintSet.addConstraint(1, haloPointSet6);

    EXPECT_EQ(constraintSet.identifiedRegularTiles.size(), 0);
    EXPECT_EQ(constraintSet.identifiedVantagePoints.size(), 2);

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p3), constraintSet.identifiedVantagePoints.end());    

    auto masterSet = constraintSet.getMasterSet();

    EXPECT_EQ(masterSet.size(), 2);

    constraintSet.logMasterSet();
}

TEST_F(ConstraintSetTest, TestSupersetThatCanResolve) {
    Logger::getInstance().setPlayerName("TestSupersetThatCanResolve");
    std::set<int> haloPointSet1 = {p1, p2, p3}; //1
    std::set<int> haloPointSet2 = {p1, p3, p4, p5, p2}; //2
    std::set<int> haloPointSet3 = {p4, p5}; //1
    std::set<int> haloPointSet4 = {p4, p6}; //1
    std::set<int> haloPointSet5 = {p1, p2, p3, p6}; //2
    

    constraintSet.addConstraint(1, haloPointSet1);
    constraintSet.addConstraint(2, haloPointSet2);
    constraintSet.addConstraint(1, haloPointSet3);
    constraintSet.addConstraint(1, haloPointSet4);
    constraintSet.addConstraint(2, haloPointSet5);

    // Should have identified
    // 6 -> vantage point
    // 4 -> regular tile
    // 5 -> vantage point

    EXPECT_EQ(constraintSet.identifiedRegularTiles.size(), 2);
    EXPECT_EQ(constraintSet.identifiedVantagePoints.size(), 4);

    EXPECT_NE(constraintSet.identifiedRegularTiles.find(p4), constraintSet.identifiedRegularTiles.end());    

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p6), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p5), constraintSet.identifiedVantagePoints.end());    

    auto masterSet = constraintSet.getMasterSet();

    EXPECT_EQ(masterSet.size(), 1);

    constraintSet.logMasterSet();

    if (masterSet.size() == 1) {
        auto observation = masterSet[0];
        EXPECT_EQ(observation.pointsValue, 1);
        EXPECT_EQ(observation.haloPointSet.size(), 3);
        EXPECT_NE(observation.haloPointSet.find(p1), observation.haloPointSet.end());
        EXPECT_NE(observation.haloPointSet.find(p2), observation.haloPointSet.end());
        EXPECT_NE(observation.haloPointSet.find(p3), observation.haloPointSet.end());
    }

    // Lets continue the exploration
    constraintSet.clear();

    std::set<int> haloPointSet6 = {p2}; //1 - This should close everything

    constraintSet.addConstraint(1, haloPointSet6);

    // Should have identified
    // 2 -> vantage point
    // 1, 3 -> regular tile

    EXPECT_EQ(constraintSet.identifiedRegularTiles.size(), 4);
    EXPECT_EQ(constraintSet.identifiedVantagePoints.size(), 2);

    EXPECT_NE(constraintSet.identifiedRegularTiles.find(p1), constraintSet.identifiedRegularTiles.end());
    EXPECT_NE(constraintSet.identifiedRegularTiles.find(p3), constraintSet.identifiedRegularTiles.end());

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p2), constraintSet.identifiedVantagePoints.end());

    masterSet = constraintSet.getMasterSet();

    EXPECT_EQ(masterSet.size(), 0);

    constraintSet.logMasterSet();

}


TEST_F(ConstraintSetTest, TestMirrorSimplify) {
    Logger::getInstance().setPlayerName("TestMirrorSimplify");
    int a = symmetry_utils::toID(5, 1);
    int aMirr = symmetry_utils::toMirroredID(a);
    int b = symmetry_utils::toID(6, 1);
    int bMirr = symmetry_utils::toMirroredID(b);

    std::set<int> haloPointSet1 = {a, aMirr, b};

    constraintSet.addConstraint(2, haloPointSet1);
    auto masterSet = constraintSet.getMasterSet();

    EXPECT_EQ(masterSet.size(), 0);

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(a), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(aMirr), constraintSet.identifiedVantagePoints.end());

    EXPECT_NE(constraintSet.identifiedRegularTiles.find(b), constraintSet.identifiedRegularTiles.end());
    EXPECT_NE(constraintSet.identifiedRegularTiles.find(bMirr), constraintSet.identifiedRegularTiles.end());

    //    
}

TEST_F(ConstraintSetTest, TestMirrorSimplify2) {
    Logger::getInstance().setPlayerName("TestMirrorSimplify2");
    int a = symmetry_utils::toID(5, 1);
    int aMirr = symmetry_utils::toMirroredID(a);
    int b = symmetry_utils::toID(6, 1);
    int bMirr = symmetry_utils::toMirroredID(b);

    int cMirr = symmetry_utils::toID(19, 22);
    int c = symmetry_utils::toMirroredID(cMirr);
    int dMirr = symmetry_utils::toID(17, 22);
    int d = symmetry_utils::toMirroredID(dMirr);

    std::set<int> haloPointSet1 = {a, b, c, d, aMirr, bMirr};

    constraintSet.addConstraint(5,  haloPointSet1);

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(a), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(b), constraintSet.identifiedVantagePoints.end());

    auto masterSet = constraintSet.getMasterSet();
    EXPECT_EQ(masterSet.size(), 1);

    EXPECT_EQ(constraintSet.identifiedVantagePoints.find(c), constraintSet.identifiedVantagePoints.end());
    EXPECT_EQ(constraintSet.identifiedVantagePoints.find(d), constraintSet.identifiedVantagePoints.end());
    EXPECT_EQ(constraintSet.identifiedRegularTiles.find(c), constraintSet.identifiedRegularTiles.end());
    EXPECT_EQ(constraintSet.identifiedRegularTiles.find(d), constraintSet.identifiedRegularTiles.end());

    std::set<int> haloPointSet2 = {c, aMirr};

    constraintSet.addConstraint(2,  haloPointSet2);

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(c), constraintSet.identifiedVantagePoints.end());

    EXPECT_NE(constraintSet.identifiedRegularTiles.find(d), constraintSet.identifiedRegularTiles.end());

    masterSet = constraintSet.getMasterSet();
    EXPECT_EQ(masterSet.size(), 0);
}

TEST_F(ConstraintSetTest, TestSubsetWithMirrorPromotion) {
    Logger::getInstance().setPlayerName("TestSubsetWithMirrorPromotion");    

    std::set<int> haloPointSet1 = {p1, p2, p3, p1Mirr};
    constraintSet.addConstraint(3,  haloPointSet1);

    std::set<int> haloPointSet2 = {p1, p2};
    constraintSet.addConstraint(2,  haloPointSet2);

    constraintSet.logMasterSet();
    

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p1), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p1Mirr), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p2), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p2Mirr), constraintSet.identifiedVantagePoints.end());

    EXPECT_NE(constraintSet.identifiedRegularTiles.find(p3), constraintSet.identifiedRegularTiles.end());
    EXPECT_NE(constraintSet.identifiedRegularTiles.find(p3Mirr), constraintSet.identifiedRegularTiles.end());
}

TEST_F(ConstraintSetTest, TestSupersetWithMirrorPromotion) {
    Logger::getInstance().setPlayerName("TestSupersetWithMirrorPromotion");    

    std::set<int> haloPointSet1 = {p1, p2};
    constraintSet.addConstraint(2,  haloPointSet1);

    std::set<int> haloPointSet2 = {p1, p2, p3, p1Mirr};
    constraintSet.addConstraint(3,  haloPointSet2);

    constraintSet.logMasterSet();
    

    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p1), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p1Mirr), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p2), constraintSet.identifiedVantagePoints.end());
    EXPECT_NE(constraintSet.identifiedVantagePoints.find(p2Mirr), constraintSet.identifiedVantagePoints.end());

    EXPECT_NE(constraintSet.identifiedRegularTiles.find(p3), constraintSet.identifiedRegularTiles.end());
    EXPECT_NE(constraintSet.identifiedRegularTiles.find(p3Mirr), constraintSet.identifiedRegularTiles.end());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}