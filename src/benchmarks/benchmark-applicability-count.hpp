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
        MaxCutGraph G(data_filepath);

        auto res_r9 = G.GetAllR9Candidates();
        auto res_r9x = G.GetAllR9XCandidates();
        auto res_r8 = G.GetAllR8Candidates();
        auto res_r10 = G.GetAllR10Candidates();

        cout << "(r8, r9, r9x, r10) = " << res_r8.size() << " " << res_r9.size() << " " << res_r9x.size() << " " << res_r10.size() << endl;
    }
};