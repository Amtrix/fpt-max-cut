#include "src/mc-graph.hpp"
#include "src/one-way-reducers.hpp"
#include "src/two-way-reducers.hpp"
#include "src/input-parser.hpp"
#include "src/utils.hpp"
#include "src/output-filter.hpp"
#include "src/graph-database.hpp"

#include "src/benchmarks/benchmark-marked-set.hpp"
#include "src/benchmarks/benchmark-kernelization.hpp"
#include "src/benchmarks/benchmark-count-clique-with-internal.hpp"
#include "src/benchmarks/benchmark-playground.hpp"
#include "src/benchmarks/benchmark-linear-kernel-paper.hpp"

#include <iostream>
using namespace std;

vector<int> tot_used_rules(10, 0);
int main(int argc, char **argv){
    //srand((unsigned)time(0));
    //ios_base::sync_with_stdio(false);
    InputParser input(argc, argv);
    InitOutputFiles(input);

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
            benchmark_action->Evaluate(input, graph);
            tot_used_rules = VectorsAdd(tot_used_rules, benchmark_action->tot_used_rules, true);
        }

        benchmark_action->PostProcess(input);
    }

    cout << endl;
    cout << " ================ Analysis over all rules commulative ================ " << endl;
    for (int r = 1; r <= 7; ++r)
        cout << "Rule " << r << " was used " << tot_used_rules[r] << " times." << endl;
    cout << " ===================================================================== " << endl << endl;
}