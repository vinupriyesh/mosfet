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

# Link the main project library
add_executable(test_parser test_parser.cc)
add_executable(test_pathing test_pathing.cc)
add_executable(test_constraint_set test_constraint_set.cc)
add_executable(test_energy_estimator test_energy_estimator.cc)

target_link_libraries(test_parser libmosfet nlohmann_json::nlohmann_json gtest gtest_main)
target_link_libraries(test_pathing libmosfet pthread gtest gtest_main)
target_link_libraries(test_constraint_set libmosfet pthread gtest gtest_main)
target_link_libraries(test_energy_estimator libmosfet pthread gtest gtest_main)

# Enable testing
enable_testing()

# Add test
gtest_discover_tests(test_parser)
gtest_discover_tests(test_pathing)
gtest_discover_tests(test_constraint_set)
gtest_discover_tests(test_energy_estimator)
