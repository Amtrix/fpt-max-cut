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

class Benchmark_CountCliquesWithInternal : public BenchmarkAction {
public:
    Benchmark_CountCliquesWithInternal() {
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

        /*
        while(1) {
            // Chose one(!):
            //auto cliques = processed.GetCliquesWithAtLeastOneInternal();
            auto cliques = processed.GetS2Candidates();
            vector<vector<int>> non_singleton;
            for (auto clique : cliques)
                if (clique.size() != 1)
                    non_singleton.push_back(clique);
            cliques = non_singleton;

            if (cliques.empty()) break;

            for (auto component : cliques) {
                cout << "Found size: " << component.size() << endl;
                accum_clique_size[component.size()]++;
                processed.RemoveEdgesInComponent(component);
                rem_num_edges += (component.size() * (((int)component.size()) - 1)) / 2;
            }
        }*/
            
        test_id++;
    }

    void PostProcess(InputParser& /* input */) override {
        int sz = ((int)accum_clique_size.size()) - 1;
        while (sz >= 0 && accum_clique_size[sz] == 0) sz--;

        cout << "Found cliques:" << endl;
        for (int i = 1; i <= sz; ++i) {
            cout << i << " " << accum_clique_size[i] << "   (" << (i*(i-1)/2) * accum_clique_size[i] << endl;
        }
        cout << "-- end --" << endl;
        cout << "rem, tot edges: " << rem_num_edges << " " << tot_num_edges << endl;
    }

private:
    int test_id = 1;
    vector<int> accum_clique_size;
    int rem_num_edges = 0;
    int tot_num_edges = 0;
};