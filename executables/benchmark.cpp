//#define SKIP_FAST_KERNELIZATION_CHECK 1

#include "src/mc-graph.hpp"
#include "src/one-way-reducers.hpp"
#include "src/two-way-reducers.hpp"
#include "src/input-parser.hpp"
#include "src/utils.hpp"
#include "src/output-filter.hpp"
#include "src/graph-database.hpp"
#include "src/colormod.hpp"

#include "src/benchmarks/benchmark-marked-set.hpp"
#include "src/benchmarks/benchmark-kernelization.hpp"
#include "src/benchmarks/benchmark-count-clique-with-internal.hpp"
#include "src/benchmarks/benchmark-playground.hpp"
#include "src/benchmarks/benchmark-linear-kernel-paper.hpp"

#include <thread>
#include <iostream>
using namespace std;

const bool kMultipleEdgesAreOk = true;

vector<int> tot_used_rules(10, 0);
int main(int argc, char **argv){
    // Init MPI
    MPI_Init(&argc, &argv);
    PEID rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //srand((unsigned)time(0));
    //ios_base::sync_with_stdio(false);

    Color::Modifier green(Color::FG_GREEN);
    Color::Modifier red  (Color::FG_RED);
    Color::Modifier defcol(Color::FG_DEFAULT);

    #ifdef DEBUG
        cout << red << "DEBUG is set to true." << defcol << endl;
    #endif
    
    #ifdef NDEBUG
        cout << red << "NDEBUG is set to true." << defcol << endl;
    #endif

    bool local_solver_exists = false;
    #ifdef LOCALSOLVER_EXISTS
        local_solver_exists = true;
    #endif

    bool skip_fast_kernelization_check = false;
    #ifdef SKIP_FAST_KERNELIZATION_CHECK
        skip_fast_kernelization_check = true;
    #endif

    std::cout << "Available number of threads: " << std::thread::hardware_concurrency() << endl;
    std::cout << std::fixed;
    std::cout << std::setprecision(5);
    InputParser input(argc, argv);
    InitOutputFiles(input);

    GraphDatabase::KagenGraphCollectionDescriptor::InitializeParamBounds(input);
    GraphDatabase graph_db(input);
    

    const string action = input.getCmdOption("-action");
    std::unique_ptr<BenchmarkAction> benchmark_action;
        
    if (action == "kernelization") {
        benchmark_action.reset(new Benchmark_Kernelization());
    } else if (action == "eval-marked-set") {
        benchmark_action.reset(new Benchmark_MarkedSet());
    } else if (action == "count-clique-internal") {
        benchmark_action.reset(new Benchmark_CountCliquesWithInternal());
    } else if (action == "playground") {
        benchmark_action.reset(new Benchmark_Playground());
    } else if (action == "linear-kernel") {
        benchmark_action.reset(new Benchmark_LinearKernelPaper());
    } else if (action == "graph-sampling-kernelization") {

    }
    else throw std::logic_error("Action flag not defined.");


    if (benchmark_action) {
        for (auto graph : graph_db) {
            cout << "================ RUNNING BENCHMARK ON " + graph.GetGraphNaming() + " ================ " << endl;
            cout << green << "   |V|:                           " << graph.GetRealNumNodes() << endl;
            cout << green << "   |E|:                           " << graph.GetRealNumEdges() << endl;
            cout << green << "   graph contains multiple edges: ";
            if (graph.info_mult_edge > 0) cout << red;
            cout << graph.info_mult_edge << defcol << endl;
            
            cout << green << "   Localsolver lib is provided: ";
            if (local_solver_exists) cout << "yes." << defcol << endl;
            else cout << red << "no." << defcol << endl;

            cout << green << "   Skip fast kernelization assertion check: ";
            if (!skip_fast_kernelization_check) cout << "no." << defcol << endl;
            else cout << red << "WARNING WARNING WARNING ------- this decision skips verification on performance ------- WARNING WARNING WARNING." << defcol << endl;


            custom_assert(kMultipleEdgesAreOk || graph.info_mult_edge == 0);

            benchmark_action->Evaluate(input, graph);
            tot_used_rules = VectorsAdd(tot_used_rules, benchmark_action->tot_used_rules, true);
            cout << "================================= END " + graph.GetGraphNaming() + " ================ " << endl << endl << endl;
        }

        benchmark_action->PostProcess(input);
    }

    cout << endl;
    cout << " ================ Analysis over all rules commulative ================ " << endl;
    for (int r = 1; r <= 7; ++r)
        cout << "Rule " << r << " was used " << tot_used_rules[r] << " times." << endl;
    cout << " ===================================================================== " << endl << endl;

    MPI_Finalize();
}