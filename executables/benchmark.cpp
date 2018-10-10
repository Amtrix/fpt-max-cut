#include "src/mc-graph.hpp"
#include "src/one-way-reducers.hpp"
#include "src/two-way-reducers.hpp"
#include "src/input-parser.hpp"
#include "src/utils.hpp"
#include "src/output-filter.hpp"

#include "src/benchmarks/benchmark-marked-set.hpp"
#include "src/benchmarks/benchmark-kernelization.hpp"
#include "src/benchmarks/benchmark-kernelization-by-clique.hpp"
#include "src/benchmarks/benchmark-kernelization-applicability-count.hpp"
#include "src/benchmarks/benchmark-count-clique-with-internal.hpp"
#include "src/benchmarks/benchmark-playground.hpp"
#include "src/benchmarks/benchmark-linear-kernel-paper.hpp"

#include <iostream>
using namespace std;

const int kDataSetCount = 1;
const string paths[] = {
   //"../data/biqmac/ising",
   // "../data/biqmac/rudy",
  //  "../data/custom",
  //  "../data/KaGen/ba",
  //  "../data/KaGen/gnp_undirected",
  //  "../data/KaGen/rhg",
  //  "../data/KaGen-2/rhg_1024_g2.4"//,

    "../data/thesis-tests/kagen-small-sparse/tests"
};

/*
used to evaluate cliques with internal node reductions:
    "../data/biqmac/rudy",
    "../data/KaGen-2/ba_1024",
    "../data/KaGen-2/gnm_undirected_128",
    "../data/KaGen-2/gnm_undirected_1024",
    "../data/KaGen-2/rdg_2d_1024",
    "../data/KaGen-2/rdg_3d_1024",
    "../data/KaGen-2/rhg_128_g2.1",
    "../data/KaGen-2/rhg_1024_g2.4",
    "../data/KaGen-2/rhg_1024_g5.4",
*/

vector<int> tot_used_rules(10, 0);
int main(int argc, char **argv){
    //srand((unsigned)time(0));
    //ios_base::sync_with_stdio(false);
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
    std::unique_ptr<BenchmarkAction> benchmark_action;
        
    if (action == "clique-kernelization") {
        benchmark_action.reset(new Benchmark_KernelizationByClique());
    } else if (action == "kernelization") {
        benchmark_action.reset(new Benchmark_Kernelization());
    } else if (action == "eval-marked-set") {
        benchmark_action.reset(new Benchmark_MarkedSet());
    } else if (action == "kernelization-applicability-count") {
        benchmark_action.reset(new Benchmark_KernelizationApplicabilityCount());
    } else if (action == "test-cschulz-to-normalized") {
        for (string data_filepath : all_sets_to_evaluate) {
            ifstream in(data_filepath);
            vector<string> sparams = ReadLine(in);
            int n = stoi(sparams[0]), m = stoi(sparams[1]);
            (void)n;
            cout << "CURR: " << m << endl;
            while (in.eof() == false) {
                int a,b; in >> a >> b;
                m--;
            }

            cout << "M-check: " << m << endl;
        }
    } else if (action == "count-clique-internal") {
        benchmark_action.reset(new Benchmark_CountCliquesWithInternal());
    } else if (action == "playground") {
        benchmark_action.reset(new Benchmark_Playground());
    } else if (action == "linear-kernel") {
        benchmark_action.reset(new Benchmark_LinearKernelPaper());
    }
    else throw std::logic_error("Action flag not defined.");

    if (benchmark_action) {
        for (string data_filepath : all_sets_to_evaluate) {
            cout << "================ RUNNING BENCHMARK ON " + data_filepath + " ================ " << endl;
            
            benchmark_action->Evaluate(input, data_filepath);
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