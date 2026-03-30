cmake_minimum_required(VERSION 3.16)
project(TankSimAll)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Only these subfolders get built:
add_subdirectory(Algorithm)
add_subdirectory(GameManager)
add_subdirectory(Simulator)
