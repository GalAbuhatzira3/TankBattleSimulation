cmake_minimum_required(VERSION 3.16)
project(simulator_211645361_000000000)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SIM_SRCS
    ${CMAKE_SOURCE_DIR}/Simulator/AlgorithmRegistrar.cpp
    ${CMAKE_SOURCE_DIR}/Simulator/TankAlgorithmRegistration.cpp
    ${CMAKE_SOURCE_DIR}/Simulator/PlayerRegistration.cpp
    ${CMAKE_SOURCE_DIR}/Simulator/GameManagerRegistrar.cpp
    ${CMAKE_SOURCE_DIR}/Simulator/GameManagerRegistration.cpp  
    ${CMAKE_SOURCE_DIR}/Simulator/BaseSimulator.cpp
    ${CMAKE_SOURCE_DIR}/Simulator/ComparativeSimulator.cpp
    ${CMAKE_SOURCE_DIR}/Simulator/CompetitiveSimulator.cpp
    ${CMAKE_SOURCE_DIR}/Simulator/main.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Block.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Board.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Mine.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/MyBattleInfo.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/MySatelliteView.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Node.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Shell.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Tank.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Wall.cpp
)

add_executable(simulator_211645361_000000000 ${SIM_SRCS})

set_target_properties(simulator_211645361_000000000 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Simulator"
    ENABLE_EXPORTS ON
)

target_include_directories(simulator_211645361_000000000 PRIVATE
    ${CMAKE_SOURCE_DIR}/UserCommon
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/Simulator
)

find_package(Threads REQUIRED)
target_link_libraries(simulator_211645361_000000000 PUBLIC
    dl
    Threads::Threads
    stdc++fs
)
