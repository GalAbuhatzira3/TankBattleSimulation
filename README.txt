This repository contains a modular C++ tank-battle simulation platform built around three main components: a game engine, an algorithm layer, and a simulator. The system supports deterministic single-game execution as well as large-scale comparative and competition simulations, with multithreaded simulation and dynamic loading of game managers and player algorithms through shared libraries. The project was designed with clean component separation, extensibility, and performance in mind.

Running:
in order to create the .so files and the executable, all we need to do is run the commands: 

rm -rf build && mkdir build && cd build && cmake .. && make

then, from the original directory: for example:
Comparative mode:
./Simulator/simulator_211645361_000000000 \
  -comparative \
  game_map=input.txt \
  game_managers_folder=GameManager \
  algorithm1=Algorithm/Algorithm_211645361_000000000.so \
  algorithm2=Algorithm/Algorithm_211645361_000000000.so \
  num_threads=10 \
  -verbose

Competition mode:
./Simulator/simulator_211645361_000000000 \
  -competition \
  game_maps_folder=maps \
  game_manager=GameManager/GameManager_211645361_000000000.so \
  algorithms_folder=Algorithm \
  num_threads=700 \
  -verbose

Command-line-arguments note: 
I've added support in the optional command line argument gen_maps=<num> (more details in features.txt)
