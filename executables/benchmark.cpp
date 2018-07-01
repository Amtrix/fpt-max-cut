#include "src/mc-graph.hpp"
#include "src/one-way-reducers.hpp"
#include "src/two-way-reducers.hpp"
#include "src/input-parser.hpp"
#include "src/utils.hpp"
#include "src/output-filter.hpp"

#include "src/benchmarks/benchmark-marked-set.hpp"
#include "src/benchmarks/benchmark-kernelization.hpp"
#include "src/benchmarks/benchmark-kernelization-by-clique.hpp"

#include <iostream>
using namespace std;

const int kDataSetCount = 3;
const string paths[] = {
    "../data/biqmac/ising",
    "../data/biqmac/rudy",
    "../data/custom"
};

vector<int> tot_used_rules(10, 0);
int main(int argc, char **argv){
    srand((unsigned)time(0));
    ios_base::sync_with_stdio(false);
    InputParser input(argc, argv);
    InitOutputFiles(input);

    vector<string> all_sets_to_evaluate;
    if (input.cmdOptionExists("-f")) {
        const string data_filepath = input.getCmdOption("-f");
        all_sets_to_evaluate.push_back(data_filepath);
    } else {
        // get all datasets from data/
        for (unsigned int i = 0; i < kDataSetCount; ++i) {
            auto sets = GetAllDatasets(paths[i]);
            for (unsigned int i = 0; i < sets.size(); ++i)
                all_sets_to_evaluate.push_back(sets[i]);
        }
    }

    const string action = input.getCmdOption("-action");

    for (string data_filepath : all_sets_to_evaluate) {
        std::unique_ptr<BenchmarkAction> benchmark_action;
        
        if (action == "clique-kernelization") {
            benchmark_action.reset(new Benchmark_KernelizationByClique());
        } else if (action == "kernelization") {
            benchmark_action.reset(new Benchmark_Kernelization());
        } else if (action == "eval-marked-set") {
            benchmark_action.reset(new Benchmark_MarkedSet());
        }
        else throw std::logic_error("Action flag not defined.");
        
        benchmark_action->Evaluate(input, data_filepath);

        tot_used_rules = VectorsAdd(tot_used_rules, benchmark_action->tot_used_rules);
    }

    cout << endl;
    cout << " ================ Analysis over all rules commulative ================ " << endl;
    for (int r = 1; r <= 7; ++r)
        cout << "Rule " << r << " was used " << tot_used_rules[r] << " times." << endl;
    cout << " ===================================================================== " << endl << endl;
}