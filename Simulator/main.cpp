#include "CommandLineParser.h"
#include "ComparativeSimulator.h"
#include "CompetitiveSimulator.h"

int main(int argc, char* argv[]) {
    CommandLineParser parser(argc, argv);
    Arguments args = parser.getArgs();

    std::unique_ptr<BaseSimulator> sim;
    if (args.mode == Arguments::Comparative)
        sim = std::make_unique<ComparativeSimulator>(args);
    else
        sim = std::make_unique<CompetitiveSimulator>(args);

    sim->simulate();
    return 0;
}
