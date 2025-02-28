#cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .

cmake_minimum_required(VERSION 3.14)
project(mosfet)

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add debug symbols if it is a debug build
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

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

# Add your library with a different name
add_library(libmosfet STATIC ${AGENT_SOURCES} ${AGENT_ROLES_SOURCES} ${AGENT_PLANNING_SOURCES} ${VISUALIZER_SOURCES} ${DATASTRUCTURE_SOURCES} main.cc parser.cc config.cc logger.cc)

# Include directories for header files
target_include_directories(libmosfet PUBLIC ${PROJECT_SOURCE_DIR})

# Link nlohmann/json to your library
target_link_libraries(libmosfet PRIVATE nlohmann_json::nlohmann_json)

# Set the output name of the library
set_target_properties(libmosfet PROPERTIES OUTPUT_NAME "libmosfet")
# Add your executable
# add_executable(mosfet main.cc parser.cc config.cc logger.cc ${AGENT_SOURCES} ${AGENT_ROLES_SOURCES} ${AGENT_PLANNING_SOURCES} ${VISUALIZER_SOURCES} ${DATASTRUCTURE_SOURCES})
add_executable(mosfet main.cc)

# Include directories for header files
target_include_directories(mosfet PUBLIC ${PROJECT_SOURCE_DIR})

# Link nlohmann/json statically to your executable
target_link_libraries(mosfet PRIVATE libmosfet nlohmann_json::nlohmann_json -static)
