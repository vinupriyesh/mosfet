## If you change anything here, dont forget to do `cmake ..` from the build directory!

cmake_minimum_required(VERSION 3.14)
project(mosfet)

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable static linking
set(BUILD_SHARED_LIBS OFF)

# Include FetchContent module
include(FetchContent)

# Fetch nlohmann/json
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3 
)

FetchContent_MakeAvailable(json)

# Include GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

# Add the tests directory
add_subdirectory(tests)

file(GLOB AGENT_SOURCES ${PROJECT_SOURCE_DIR}/agent/*.cc)
file(GLOB AGENT_ROLES_SOURCES ${PROJECT_SOURCE_DIR}/agent/roles/*.cc)
file(GLOB AGENT_PLANNING_SOURCES ${PROJECT_SOURCE_DIR}/agent/planning/*.cc)
file(GLOB VISUALIZER_SOURCES ${PROJECT_SOURCE_DIR}/visualizer/*.cc)
file(GLOB DATASTRUCTURE_SOURCES ${PROJECT_SOURCE_DIR}/datastructures/*.cc)

# Add your executable
add_executable(mosfet main.cc parser.cc config.cc ${AGENT_SOURCES} ${AGENT_ROLES_SOURCES} ${AGENT_PLANNING_SOURCES} ${VISUALIZER_SOURCES} ${DATASTRUCTURE_SOURCES})

# Include directories for header files
target_include_directories(mosfet PUBLIC ${PROJECT_SOURCE_DIR})


# Link nlohmann/json to your executable
target_link_libraries(mosfet PRIVATE nlohmann_json::nlohmann_json -static)
