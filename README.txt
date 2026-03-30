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


Algorithm: 
Aggressive policy: Prioritizes destroying enemy tanks over survival.
Enemy discovery: Upon receiving MyBattleInfo, it scans the SatelliteView (BFS) to collect enemy positions.
Path evaluation:
 •	If no safe path exists to any enemy (i.e., a route free of shells, mines, or teammates tanks):
   o	If the tank is in danger (some shell has a direct line to it, not necessarily on its current trajectory. The algorithm is provided with directionless satellite screenshot of the board), it evaluates all 8 neighboring cells, ordered by minimal rotate+move cost, and moves to the first safe candidate.
   o	If not under threat, it prepares a shot: rotates/repositions as needed and fires when a clear shot is available. If no meaningful shot exists, it may try up to two shots along the current cannon direction, then rotate and yield control, hoping the board state changes (e.g., a wall is destroyed).
 •	If a safe path exists, it advances along that path. At each step (and after each half/full rotation), it checks whether its current cannon direction already targets an enemy and fires when it’s able (tracks cooldowns) safe (no teammates in the line of fire) and worthwhile (a clear shot at an enemy).
Friendly-fire safeguards: Never push a SHOOT command unless checks the shot will probably not hit a teammate or itself (e.g., through “invisible tunnel” scenarios).
Primary objective: Close distance to an enemy and fire from the shortest feasible range to maximize accuracy and ensure a clear line of fire.


Command-line-arguments note: 
I've added support in the optional command line argument gen_maps=<num> (more details in features.txt)
