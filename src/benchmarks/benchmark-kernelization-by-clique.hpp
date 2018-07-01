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

class Benchmark_KernelizationByClique : public BenchmarkAction {
public:
    void Evaluate(InputParser& input, const string data_filepath) {
        (void) input;
        cout << "================ RUNNING BENCHMARK ON " + data_filepath + " ================ " << endl;
        MaxCutGraph G(data_filepath);

        unordered_map<string, bool> visited;
        for (int sz = 70; sz <= 90; ++sz) {
            vector<int> previous;
            for (int r = 0; r < 200; ++r) {
                auto clique = G.GetAClique(sz, 2000);
                sort(clique.begin(), clique.end());
                string key = "";
                for (auto node : clique) key = key + to_string(node) + ",";
                if (visited[key]) continue;
                visited[key] = true;

                int isz = SetIntersection(clique, previous).size();
                cout << "Intersect with previous(" << previous.size() <<") size: " << isz << " (total: " << previous.size() + clique.size() - isz << ")" << endl;
                previous = clique;
                cout << "SZ: " << clique.size() << " = ";
                for (auto node : clique)
                    cout << node << " ";
                cout << endl;

                vector<int> S = G.GetAllExistingNodes();
                S = SetSubstract(S, clique);

                cout << "|S| = " << S.size() << endl;
                G.SetMarkedVertices(S);
    /*            G.ReduceMarksetVertexSet();
                S = G.GetMarkedVerticesByOneWayRules();
                cout << "|S| = " << S.size() << endl;*/
                

                MaxCutGraph wG = G;
                int res;
                while ((res = ExhaustiveTwoWayReduce(wG, S)) > -1) {
                    cout << "Kernelization rule: " << res << " was applied." << endl;
                    cout << "New G. Stats: " << "|V| = " << G.GetRealNumNodes() << " , |E| = " << wG.GetRealNumEdges() << " , EE = " << wG.GetEdwardsErdosBound() << endl;
                    assert(false);
                }
            }
        }
    }
};