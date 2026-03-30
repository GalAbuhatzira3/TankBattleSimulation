#include "BaseSimulator.h"

BaseSimulator::BaseSimulator(const Arguments& args):
        args(args),
        algorithmRegistrar(AlgorithmRegistrar::getAlgorithmRegistrar()),
        gameManagerRegistrar(GameManagerRegistrar::getGameManagerRegistrar()){}

BaseSimulator::~BaseSimulator() = default;

void BaseSimulator::simulate() {
    setLogger();
    scanSharedObjects(); 
    loadSharedObjects();
    runMatches();
    cleanup();
}

void BaseSimulator::setLogger(){
    this->uniqueTime = Utils::getUniqueTime();
    std::string logPrefix = cfg.simLogFilePrefix.value_or(Constants::simulatorLoggerPrefix);
    this->logger = std::make_unique<Logger>(logPrefix+this->uniqueTime+".log",
                                            cfg.simLogPrintToConsole.value_or(false),
                                            cfg.simLogIncludeTID.value_or(true),
                                            cfg.simLogLevel.value_or(LogLevel::UNKNOWN));
    std::string logText = "Config.args";
    if(this->args.mode==Arguments::Comparative){
        logText+= " mode=comparative game_map="+this->args.gameMap
                  + " game_managers_folder="+this->args.gameManagersFolder
                  + " algorithm1="+this->args.algorithms[0]
                  + " algorithm2="+this->args.algorithms[1];
    } else if(this->args.mode==Arguments::Competitive){
        logText+= " mode=competition game_map_folder="+this->args.gameManagersFolder
                  + " game_manager="+this->args.gameManager
                  + " algorithm_folder="+this->args.algorithmsFolder;
    }
    if(this->args.numThreads)
        logText+=" num_threads="+std::to_string(this->args.numThreads.value());
    logText += std::string(" verbose=") + (this->args.verbose ? "true" : "false");
    this->logger->debug(logText);
}

void BaseSimulator::logBoardData(BoardData &boardData){
    this->logger->debug(
            "Config.map name="+boardData.mapName
            +" maxSteps="+std::to_string(boardData.maxSteps)
            +" numShells="+std::to_string(boardData.numShells)
            +" rows="+std::to_string(boardData.mapHeight)
            +" columns="+std::to_string(boardData.mapWidth));
}

void BaseSimulator::loadSharedObjects() {
    std::set<std::string> names;
    auto handleBadReg = [&](auto& registrar, const char* kind, const std::string& name, void* handle) {
        std::string reason = std::string(registrar.isDuplicate(name) ? "duplicated" : "invalid")+ " .so " + kind + " file '" + name + "'";
        Files::pushToFile(Constants::errorFilePath, reason);
        int rc = dlclose(handle);
        if(rc!=0)
            this->logger->error("SO.files name="+name+" action=unload result=FAILED reason="+dlerror());
        else
            this->logger->info("SO.files name="+name+" action=unload result=OK");
        registrar.removeLast();
    };
    auto load = [&](const std::vector<std::string>& paths, bool isAlgorithm) {
        for (auto const& soFilePath : paths) { // as long as there are .so files - load them
            std::string name = fs::path(soFilePath).stem().string();
            if(isAlgorithm)
                this->algorithmRegistrar.createAlgorithmFactoryEntry(name);
            else
                this->gameManagerRegistrar.createGameManagerFactoryEntry(name);
            void* handle = dlopen(soFilePath.c_str(), RTLD_LAZY | RTLD_LOCAL); // load the .so
            if (!handle) {
                Files::pushToFile(Constants::errorFilePath, "dlopen failed loading '" + name + "'. Reason=" + dlerror() + ".\n");
                this->logger->error("SO.files name="+name+" action=load result=FAILED reason="+dlerror());
                if(isAlgorithm)
                    this->algorithmRegistrar.removeLast();
                else
                    this->gameManagerRegistrar.removeLast();
                continue;
            }else
                this->logger->info("SO.files name="+name+" action=load result=OK");
            try {
                if(isAlgorithm){
                    this->algorithmRegistrar.validateLastRegistration();
                    this->openedAlgorithmsHandles.push_back(handle);
                }
                else{
                    this->gameManagerRegistrar.validateLastRegistration();
                    this->openedGameManagerHandles.push_back(handle);
                }
            }
            catch (const AlgorithmRegistrar::BadRegistrationException& e) {
                handleBadReg(this->algorithmRegistrar, "Algorithm", name, handle);
            }
            catch (const GameManagerRegistrar::BadRegistrationException&){
                handleBadReg(this->gameManagerRegistrar, "GameManager", name, handle);
            }
        }
    };
    load(algorithmSOs, true);
    load(gameManagerSOs, false);
    // check if the number of successfully loaded .so files meets the minimum:
    if(this->openedAlgorithmsHandles.size() < Constants::minLoadedAlgorithmSO){
        this->logger->critical("SO.files reason=not enough loaded Algorithms .so files");
        Files::error("there aren't enough (provided"+std::to_string(this->openedGameManagerHandles.size())+") loaded Algorithm .so files in order to operate the game (required"+std::to_string(Constants::minLoadedAlgorithmSO)+").");
    }
    if(this->openedGameManagerHandles.size() < Constants::minLoadedGameManagerSO){
        this->logger->critical("SO.files reason=not enough loaded GameManager .so files");
        Files::error("there aren't enough (provided:"+std::to_string(this->openedGameManagerHandles.size())+") loaded GameManager .so files in order to operate the game (required"+std::to_string(Constants::minLoadedGameManagerSO)+").");
    }
    // all good, we can run the simulation now.
}

void BaseSimulator::cleanup() {
    this->algorithmRegistrar.clear();
    this->gameManagerRegistrar.clear();
    auto unload = [&](std::vector<void*> lst, const std::string type){
        for (auto h : lst){
            int rc = dlclose(h);
            const char* err = dlerror();
            if(rc!=0)
                this->logger->error(std::string("SO.files action=unload type="+type+" result=FAILED reason=") +(err ? err : "unknown"));
            else
                this->logger->info("SO.files action=unload type="+type+" result=OK");
        }
        lst.clear();
    };
    unload(openedAlgorithmsHandles, "Algorithm");
    unload(openedGameManagerHandles, "GameManager");
}

// This function maybe a little long, but it's due to the Threads documentation and stats bonus. 
void BaseSimulator::runWithThreads(std::vector<Task> &tasks) {
    using clock = std::chrono::steady_clock;
    const auto t0 = clock::now();
    //const size_t hw = std::max<size_t>(1, std::thread::hardware_concurrency()); // this will make sure we don't create more threads then we can handle
    size_t num_threads = this->args.numThreads.value_or(1); // the provided num in num_threads=<num> or 1
    size_t maxUseful  = tasks.size(); // this is the most threads we need, no more tasks to give others
    size_t requested = (num_threads == 1) ? 0 : num_threads;
    //size_t workers    = std::min({requested, maxUseful, hw}); // https://moodle.tau.ac.il/mod/forum/discuss.php?d=118153
    size_t workers    = std::min({requested, maxUseful});
    // single thread:
    if (workers == 0) {
        for (auto &t : tasks)t();
        const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now()-t0).count();
        this->logger->debug("Threads.finish mode=single executed=" + std::to_string(tasks.size()) + " elapsed_ms="+std::to_string(dt));
        return;
    }
    // parallel path: setting up the shared task queue and synchronization:
    std::atomic<size_t> taken{0}, inProgress{0}, peakConcurrent{0};
    size_t maxQueueSeen = 0;
    std::vector<size_t> donePerWorker(workers, 0);
    std::vector<int> cpuPerWorker(workers, -1);
    std::queue<Task> queue;
    std::mutex lock; // guards access to queue and doneAdding
    std::condition_variable cv;  // lets worker threads sleep until there's work, or we've finished adding
    bool doneAdding = false; // flag for when the main thread has enqueued all tasks
    // worker lambda function:
    auto worker = [&](size_t wid) {
        return [&,wid](){
            this->logger->debug("Threads.info id="+std::to_string(wid) + " status=start");
            while(true){
                Task task;
                size_t qsize_before = 0;{
                    std::unique_lock<std::mutex> Ik(lock);
                    cv.wait(Ik, [&]{return doneAdding || !queue.empty();}); // wait until either there’s work, or we’re completely done adding:
                    if(queue.empty() && doneAdding){ // if no tasks left, and we won’t add more, exit:
                        // printing: thread with id=<id> is done, and completed <tasksCompleted> tasks during its lifetime.
                        this->logger->debug("Threads.info id="+std::to_string(wid) + " status=done tasksCompleted=" + std::to_string(donePerWorker[wid]));
                        return; // no more work
                    }
                    qsize_before = queue.size();
                    if (qsize_before > maxQueueSeen) maxQueueSeen = qsize_before;
                    // otherwise, grab the next work item:
                    task = std::move(queue.front());
                    queue.pop();
                }
                // running
                const size_t nowIn = inProgress.fetch_add(1) + 1;
                // track peak concurrency
                size_t peek = peakConcurrent.load();
                while (nowIn > peek && !peakConcurrent.compare_exchange_weak(peek, nowIn)) {}
                if (cpuPerWorker[wid] == -1) cpuPerWorker[wid] = sched_getcpu();
                // run the task outside the lock:
                task();
                inProgress.fetch_sub(1);
                ++donePerWorker[wid];
                taken.fetch_add(1, std::memory_order_relaxed);
            }
        };
    };
    // create workers threads, each running the worker lambda
    std::vector<std::thread> pool;
    pool.reserve(workers);
    for(size_t i = 0; i < workers; i++)
        pool.emplace_back(worker(i));
    {// enqueue all tasks at once:
        std::lock_guard<std::mutex> lk(lock);
        for (auto& t : tasks) queue.push(std::move(t));
        doneAdding = true; // Done!
    }
    this->logger->debug("Threads.enqueue requested="+std::to_string(requested) +
                        " tasks="+std::to_string(tasks.size()) +
                        " maxUseful="+std::to_string(maxUseful) +
                        " workers="+std::to_string(workers));
    for (size_t i = 0; i < std::min(tasks.size(), workers); ++i) // notify only those who are needed! not everyone!
        cv.notify_one();
    //cv.notify_all(); // wake any sleeping workers, so they can start grabbing tasks
    // join all threads (waiting for each worker to finish)
    for (auto& th : pool) th.join();
    // summary:
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t0).count();
    const double tps = ms ? (1000.0 * taken.load()) / ms : 0.0;
    logger->debug("Threads.finish mode=parallel executed=" + std::to_string(taken.load()) // how many tasks were executed
                  + " elapsed_ms=" + std::to_string(ms) // the time that took for the whole run to be completed (milliseconds)
                  + " throughput_tps=" + std::to_string((long long)tps) // Tasks Per Second
                  + " peak_concurrency=" + std::to_string(peakConcurrent.load()) // the maximum number of tasks running at the same instant
                  + " max_queue=" + std::to_string(maxQueueSeen)); // the largest queue size observed
}

bool BaseSimulator::isValidResult(const GameResult &result, const std::string& gmName) {
    if(result.winner != 0 && result.winner != 1 && result.winner != 2) { // must be a draw or a win by one of the two players
        this->logger->warn("Game.result GameManager="+gmName+" field=winner result=INVALID reason=unsupported winner (not 0,1,2)");
        return false;
    }
    switch(result.reason){
        case GameResult::ALL_TANKS_DEAD:
        case GameResult::MAX_STEPS:
        case GameResult::ZERO_SHELLS:break;
        default: {
            this->logger->warn("Game.result GameManager="+gmName+" field=result result=INVALID reason=unsupported result");
            return false;
        }
    }
    if(result.remaining_tanks.size() < Constants::numOfPlayers) { // should have a cell for at least 2 players
        this->logger->warn("Game.result GameManager="+gmName+" field=remaining tanks result=INVALID reason=array size"+std::to_string(result.remaining_tanks.size())+"(req=2)");
        return false;
    }
    if(!result.gameState){ // the view of the board can't be null
        this->logger->warn("Game.result GameManager="+gmName+" field=gameState result=INVALID reason=NULL");
        return false;
    }
    // we didn't check the game rounds because it can be 0 (for example, no tanks at all in the map - the game will not start)
    return true;
}

void BaseSimulator::trimLine(std::string& line) {
    if (!line.empty() && (line.back() == '\r' || line.back() == '\0'))
        line.pop_back();
}

std::vector<std::string> BaseSimulator::readLines(const std::string& filename) {
    std::ifstream in(filename);
    if (!in)
        throw std::runtime_error("Cannot open '" + filename + "'");

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        trimLine(line);
        lines.push_back(line);
    }
    return lines;
}

size_t BaseSimulator::parseVariables(const std::vector<std::string>& lines, BoardData& boardData) {
    const std::vector<std::string> EXPECTED = {"MapName", "MaxSteps", "NumShells", "Rows", "Cols"};
    std::regex text(R"(^\s*(.*)\s*$)");
    std::regex pat(R"(^\s*([A-Za-z]\w*)\s*=\s*(\d+)\s*$)");
    std::smatch m;

    size_t idx = 0;
    size_t lastVarLine = 0;
    for (size_t i = 0; i < lines.size() && idx < EXPECTED.size(); ++i) {
        const auto& line = lines[i];
        // Reject blank line before all variables are read
        if (idx < EXPECTED.size() && line.find_first_not_of(" \t") == std::string::npos)
            throw std::runtime_error("Blank line before reading all variables");

        if (idx == 0){ // look for the name of the map
            if(!std::regex_match(line, m, text))
                throw std::runtime_error("Expected <" + EXPECTED[idx] + "> at line " + std::to_string(i+1) + " but got unidentified '" + line + "'.");
            boardData.mapName = line;
            idx ++;
            continue;
        }

        if (!std::regex_match(line, m, pat)) // look for the other arguments
            throw std::runtime_error("Expected " + EXPECTED[idx] + "=<number> at line " + std::to_string(i+1) + " but got unidentified '" + line + "'.");

        const std::string name = m[1];
        int value = std::stoi(m[2]);

        if (name != EXPECTED[idx])
            throw std::runtime_error("Expected " + EXPECTED[idx] + "=<number> at line " + std::to_string(i+1) + " but got '" + name + "'.");

        switch (idx) {
            case 1: boardData.maxSteps  = value; break;
            case 2: boardData.numShells = value; break;
            case 3: boardData.mapHeight = value; break;
            case 4: boardData.mapWidth  = value; break;
        }
        idx++;
        lastVarLine = i;
    }
    if (idx != 5)
        throw std::runtime_error("Missing or out-of-order variables");
    return lastVarLine;
}

void BaseSimulator::captureBoard(const std::vector<std::string>& lines, size_t lastVarLine, BoardData& boardData) {
    for (size_t i = lastVarLine + 1; i < lines.size(); ++i) {
        std::string row = lines[i];
        trimLine(row);
        boardData.boardLines.push_back(row);
    }
}

void BaseSimulator::warnRowCount(BoardData& boardData) {
    int actual = static_cast<int>(boardData.boardLines.size());
    if (actual < boardData.mapHeight) {
        auto msg = "[Warning: missing rows] only " + std::to_string(actual) +" board rows found, but Rows=" + std::to_string(boardData.mapHeight);
        Files::pushToFile(Constants::errorFilePath, msg);
    } else if (actual > boardData.mapHeight) {
        auto msg = "[Warning: extra rows] " + std::to_string(actual) +" board rows found, but Rows=" + std::to_string(boardData.mapHeight);
        Files::pushToFile(Constants::errorFilePath, msg);
        for (size_t r = boardData.mapHeight; r < actual; ++r) {
            msg = "[Warning: extra row details] row-index="+std::to_string(r)+" policy=ignore ignored='"+boardData.boardLines[r]+"'";
            Files::pushToFile(Constants::errorFilePath, msg);
        }
    }
}

void BaseSimulator::warnColumnCounts(BoardData& boardData) {
    bool overPrinted = false, underPrinted = false;
    std::string msg;
    int rows = static_cast<int>(boardData.boardLines.size());
    for (int r = 0; r < rows; ++r) {
        int len = static_cast<int>(boardData.boardLines[r].size());
        if(len > boardData.mapWidth){
            if (len != boardData.mapWidth){
                if(!overPrinted){
                    msg = "[Warning: extra columns] there are cases of rows with more columns than stated. Columns="+std::to_string(boardData.mapWidth)+"";
                    Files::pushToFile(Constants::errorFilePath, msg);
                    overPrinted = true;
                }
                msg = "[Warning: extra column details] row-index=" + std::to_string(r) + " policy=ignore ignored='"+boardData.boardLines[r].substr(boardData.mapWidth)+"'";
                Files::pushToFile(Constants::errorFilePath, msg);
            }
        } else if(len<boardData.mapWidth && !underPrinted){
            msg = "[Warning: missing columns] there are cases of rows with less columns than stated. Columns="+std::to_string(boardData.mapWidth)+"";
            Files::pushToFile(Constants::errorFilePath, msg);
            underPrinted = true;
        }
    }
}

std::vector<std::vector<char>> BaseSimulator::buildMatrix(BoardData& boardData) {
    std::vector<std::vector<char>> mat(boardData.mapHeight, std::vector<char>(boardData.mapWidth, ' '));
    int actual = static_cast<int>(boardData.boardLines.size());
    for (int r = 0; r < boardData.mapHeight; ++r) {
        if (r >= actual) break;
        const auto& row = boardData.boardLines[r];
        int limit = std::min((int)boardData.mapWidth, static_cast<int>(row.size()));
        for (int c = 0; c < limit; ++c)
            mat[r][c] = row[c];
    }
    return mat;
}

BaseSimulator::BoardData BaseSimulator::readBoard(const std::string &inputFilePath) {
    std::string line, mapName;
    BoardData boardData;
    std::vector<std::vector<char>> matrix;
    try{
        auto lines = readLines(inputFilePath);
        size_t lastVarLine = parseVariables(lines, boardData);
        captureBoard(lines, lastVarLine, boardData);
        warnRowCount(boardData); // let the user know about any extra/missing rows
        warnColumnCounts(boardData); // the same with columns
        matrix = buildMatrix(boardData); // build the matrix out of the extracted data
    }
    catch(const std::exception& e){
        this->logger->critical("IO.Error type=fatal action=reading board note=more info on input_errors.txt");
        Files::error(e.what()); // an unrecoverable error has occurred - exit the program.
    }
    boardData.map = std::make_unique<MySatelliteView>(matrix);
    return boardData;
}

// This function maybe a little long, but it's due to the Game maps generation system bonus.
std::string BaseSimulator::generateMap() {
    auto sanitize = [](std::optional<size_t> v, size_t def, size_t upperExclusive){
        size_t x = v.value_or(def);
        return (x <= 0 || x >= upperExclusive) ? def : x;
    };
    size_t maxRows = sanitize(cfg.rowsMax, Constants::maxRows, 20);
    size_t maxCols = sanitize(cfg.colsMax, Constants::maxCols, 100);
    size_t maxShells = sanitize(cfg.shellsMax, Constants::maxShells, 500);
    size_t maxSteps = sanitize(cfg.stepsMax, Constants::maxSteps, 10000);
    size_t wallPct = sanitize(cfg.wallPct, Constants::wallPct, 100);
    size_t minePct = sanitize(cfg.minePct, Constants::minePct, 100);
    size_t emptyPct = sanitize(cfg.emptyPct, Constants::emptyPct, 100);
    size_t p1Pct = sanitize(cfg.p1Pct, Constants::p1Pct, 100);
    size_t p2Pct = sanitize(cfg.p2Pct, Constants::p2Pct, 100);
    std::vector<std::pair<char,double>> alphabet = {
            {Symbols::wall, wallPct},
            {Symbols::mine,  minePct},
            {Symbols::players[0],  p1Pct},
            {Symbols::players[1], p2Pct},
            {Symbols::empty, emptyPct}
    };
    std::vector<char>   symbols; symbols.reserve(alphabet.size());
    std::vector<double> weights; weights.reserve(alphabet.size());
    for (const auto& [symbol, weight] : alphabet) {
        symbols.push_back(symbol);
        weights.push_back(weight);
    }
    std::mt19937 rng;
    std::optional<unsigned> seed = std::nullopt;
    if (seed) rng.seed(*seed);
    else      rng.seed(std::random_device{}());
    auto pickUniform1toN = [&](size_t n) -> int {
        int hi = (n == 0 || n > static_cast<size_t>(std::numeric_limits<int>::max()))? 1: static_cast<int>(n);
        std::uniform_int_distribution<int> dist(1, hi);
        return dist(rng);
    };
    // Random dimensions (1..max)
    const int steps = pickUniform1toN(maxSteps);
    const int shells = pickUniform1toN(maxShells);
    const int rows = pickUniform1toN(maxRows);
    const int cols = pickUniform1toN(maxCols);
    // Weighted picker
    std::discrete_distribution<size_t> pick(weights.begin(), weights.end());
    std::filesystem::path dir  = cfg.mapDir.value_or(Constants::mapDir);
    std::error_code ec;
    if (!dir.empty())
        std::filesystem::create_directories(dir, ec);
    std::string id = Utils::getUniqueTime();
    std::filesystem::path file = dir / ("generated_map_" + id + ".txt");
    // Write file
    std::ofstream out(file);
    if (!out.is_open())
        Files::error("Couldn't generate the map files. Please try again");
    out << "generated_map_" << id << '\n';
    out << "MaxSteps=" << steps << '\n';
    out << "NumShells=" << shells << '\n';
    out << "Rows=" << rows << '\n';
    out << "Cols=" << cols << '\n';
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c)
            out << symbols[pick(rng)];
        out << '\n';
    }
    out.flush();
    if(args.mode == Arguments::Competitive)
        return cfg.mapDir.value_or(Constants::mapDir); // this is the folder the files were created in
    else
        return cfg.mapDir.value_or(Constants::mapDir) + "/generated_map_" + id + ".txt"; // this is the file that was created
}
