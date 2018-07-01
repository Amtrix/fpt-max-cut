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

class Benchmark_Kernelization : public BenchmarkAction {
public:
    void Evaluate(InputParser& input, const string data_filepath /*, vector<int>& tot_used_rules*/) {
        cout << "================ RUNNING BENCHMARK ON " + data_filepath + " ================ " << endl;
        MaxCutGraph G(data_filepath);

        int k = 0;
        int rule_taken;
        OutputDebugLog("----------- START: APPLYING ONE-WAY REDUCTION RULES TO COMPUTE S -----------");
        MaxCutGraph G_processing_oneway = G; // ! make sure no pointers in G !
        while ((rule_taken = TryOneWayReduce(G_processing_oneway, k)) != -1) {
            OutputDebugLog("RULE: " + to_string(rule_taken));
            OutputDebugLog("-----------");
            tot_used_rules[rule_taken]++;
        }

        cout << "|V| = " << G.GetNumNodes() << endl;
        cout << "|E| = " << G.GetRealNumEdges() << endl; 
        cout << "EE = " << G.GetEdwardsErdosBound() << endl;
        cout << "k' = " << k << endl;

        G.SetMarkedVertices(G_processing_oneway.GetMarkedVerticesByOneWayRules());
        auto S = G.GetMarkedVerticesByOneWayRules();
        cout << "|S| = " << S.size() << endl;
        cout << "S: " << " ";
        for (auto node : S) cout << node << " ";
        cout << endl;
        OutputDebugLog("----------- DONE: APPLYING ONE-WAY REDUCTION RULES TO COMPUTE S -----------");

        // Try reduce size of S
        G.ReduceMarksetVertexSet();
        S = G.GetMarkedVerticesByOneWayRules();


        cout << "reduced |S| = " << S.size() << endl;

        sort(S.begin(), S.end());

    #ifdef DEBUG
        auto G_minus_S_vertex_set = SetSubstract(G.GetAllExistingNodes(), S);
        MaxCutGraph G_minus_S(G, G_minus_S_vertex_set);
        bool debug_iscliquef = G_minus_S.IsCliqueForest();
        OutputDebugLog("Is clique forest G - S: " + to_string(debug_iscliquef));
        assert(debug_iscliquef);
    #endif

        if (input.cmdOptionExists("-cc-brute")) { // temp flag since this is very slow
            int mx_sol = G.ComputeOptimalColoringBruteforce(S);
            cout << "mx_sol = " + to_string(mx_sol) << endl;
            cout << "Coloring: ";
            for (int color : G.GetMaxCutColoring()) cout << color << " ";
            cout << endl;

            auto allv = G.GetAllExistingNodes();
            auto sss = G.MaxCutExtension(allv, G.GetMaxCutColoring());
            cout << (get<0>(sss)) << endl;
        } else if (input.cmdOptionExists("-cc-brute-with-prunning")) {
            sort(S.begin(), S.end());
            int mx_sol = G.ComputeOptimalColoring(S);
            cout << "mx_sol = " + to_string(mx_sol) << endl;
        } else {
            int res;
            while ((res = ExhaustiveTwoWayReduce(G, S)) > -1) {
                cout << "Kernelization rule: " << res << " was applied." << endl;
                cout << "New G. Stats: " << "|V| = " << G.GetRealNumNodes() << " , |E| = " << G.GetRealNumEdges() << " , EE = " << G.GetEdwardsErdosBound() << endl;
            }

            if (input.cmdOptionExists("-print-kernalized-graph")) {
                const string output_filepath = input.getCmdOption("-print-kernalized-graph");
                ofstream out(output_filepath);
                G.PrintGraph(out);
                out.close();
                OutputDebugLog("Kernalized graph output is done.");
            }
        }
    }
};