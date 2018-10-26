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


class Benchmark_MarkedSet : public BenchmarkAction {
public:
    void Evaluate(InputParser& input, const MaxCutGraph& main_graph) {
        MaxCutGraph G = main_graph;

        int k = 0, rule_taken;
        MaxCutGraph G_processing_oneway = G; // ! make sure no pointers in G !
        while ((rule_taken = TryOneWayReduce(G_processing_oneway, k)) != -1) {
            OutputDebugLog("RULE: " + to_string(rule_taken));
            OutputDebugLog("-----------");
        }

        G.SetMarkedVertices(G_processing_oneway.GetMarkedVerticesByOneWayRules());
        const int s_size_oneway = G.GetMarkedVerticesByOneWayRules().size();


        // Try reduce size of S
        G.ReduceMarksetVertexSet();
        const int s_size_oneway_with_reverse = G.GetMarkedVerticesByOneWayRules().size();
        const int s_size_adhoc = G.Algorithm3MarkedComputation_Randomized();


        OutputFilterMarkedVertices(input, main_graph.GetGraphNaming(), G.GetNumNodes(), G.GetRealNumEdges(), s_size_oneway, s_size_oneway_with_reverse, s_size_adhoc);
    }
};