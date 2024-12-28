cmake_minimum_required(VERSION 3.14)
project(mosfetTests)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Link GoogleTest
include(GoogleTest)

# Include directories for header files
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${json_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/..)


# Add test executable for test_parser
add_executable(test_parser test_parser.cc ${PROJECT_SOURCE_DIR}/../parser.cc)
target_link_libraries(test_parser gtest gtest_main nlohmann_json::nlohmann_json)

# Add test executable for test_pathing
add_executable(test_pathing test_pathing.cc ${PROJECT_SOURCE_DIR}/../agent/pathing.cc ${PROJECT_SOURCE_DIR}/../agent/game_map.cc)
target_link_libraries(test_pathing gtest gtest_main pthread)

# Enable testing
enable_testing()

# Add test
gtest_discover_tests(test_parser)
gtest_discover_tests(test_pathing)
