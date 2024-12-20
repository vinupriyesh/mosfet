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

# Add test files
add_executable(test_parser test_parser.cc)

# Link libraries
target_link_libraries(test_parser gtest gtest_main nlohmann_json::nlohmann_json)

# Enable testing
enable_testing()

# Add test
gtest_discover_tests(test_parser)
