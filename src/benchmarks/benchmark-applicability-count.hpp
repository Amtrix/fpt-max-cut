#pragma once

#include "./benchmark-interface.hpp"
#include "../mc-graph.hpp"
#include "../one-way-reducers.hpp"
#include "../two-way-reducers.hpp"
#include "../input-parser.hpp"
#include "../utils.hpp"
#include "../output-filter.hpp"

#include <iostream>
using namespace std;

class Benchmark_ApplicabilityCount : public BenchmarkAction {
public:
    void Evaluate(InputParser& input, const string data_filepath) {
        (void) input;
        cout << "================ RUNNING BENCHMARK ON " + data_filepath + " ================ " << endl;
        MaxCutGraph G(data_filepath);

        auto res_r9 = G.GetAllR9Candidates();
        auto res_r9x = G.GetAllR9XCandidates();
        auto res_r8 = G.GetAllR8Candidates();

        cout << "(r8, r9, r9x) = " << res_r8.size() << " " << res_r9.size() << " " << res_r9x.size() << endl;
    }
};