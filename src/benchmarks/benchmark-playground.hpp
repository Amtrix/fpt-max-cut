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

class Benchmark_Playground : public BenchmarkAction {
public:
    Benchmark_Playground() {
        accum_clique_size = vector<int>(10000, 0);
    }

    void Evaluate(InputParser& /* input */, const MaxCutGraph& main_graph /*, vector<int>& tot_used_rules*/) {
        //int num_iterations = 1;
        //if (input.cmdOptionExists("-iterations")) {
        //    num_iterations = stoi(input.getCmdOption("-num-iterations"));
        //}

        MaxCutGraph G = main_graph;
        MaxCutGraph processed = G;
        tot_num_edges += G.GetRealNumEdges();

        auto current_v = G.GetAllExistingNodes();
        int min_deg = 1e9;
        for (auto node : current_v) {
            auto adj = G.GetAdjacency(node);
            min_deg = min(min_deg, (int)adj.size());
        }

        cout << "Min degree: " << min_deg << endl;
            
        test_id++;
    }

    void PostProcess(InputParser& /* input */) override {
    }

private:
    int test_id = 1;
    vector<int> accum_clique_size;
    int rem_num_edges = 0;
    int tot_num_edges = 0;
};