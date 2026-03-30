cmake_minimum_required(VERSION 3.16)
project(GameManager_211645361_000000000)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(GameManager_211645361_000000000 SHARED
    # userCommon
    ${CMAKE_SOURCE_DIR}/UserCommon/Block.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Board.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Mine.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/MyBattleInfo.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/MySatelliteView.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Node.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Shell.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Tank.cpp
    ${CMAKE_SOURCE_DIR}/UserCommon/Wall.cpp
    ${CMAKE_SOURCE_DIR}/GameManager/MyGameManager_211645361_000000000.cpp
)

set_target_properties(GameManager_211645361_000000000 PROPERTIES
    PREFIX ""             # no “lib” at front
    SUFFIX ".so"          # ensure .so suffix
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/GameManager"
)

target_include_directories(GameManager_211645361_000000000 PRIVATE
    ${CMAKE_SOURCE_DIR}/UserCommon
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/GameManager
)


