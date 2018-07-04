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
        for (int sz = 2; sz <= 90; ++sz) {
            vector<int> previous;
            for (int r = 0; r < 10000; ++r) {
                //auto clique = G.GetAClique(sz, 2000);
                auto clique = G.GetAnyR8Clique();
                if (clique.size() == 0) {
                    cout << "RULE 8 NOT APPLICABLE" << endl;
                    return;
                }
              //  clique = vector<int>{ 55,60 ,54, 79, 66, 50, 72 ,78 ,45 ,76 ,40, 63, 38, 62, 37, 39, 59, 32, 31, 30, 33, 36, 21, 22, 19, 77, 70, 14, 13, 26, 20, 10, 9, 48, 8, 7, 44, 3 ,28, 2, 57, 1};
                sort(clique.begin(), clique.end());
                string key = "";
                for (auto node : clique) key = key + to_string(node) + ",";
                if (visited[key]) continue;
                visited[key] = true;

                cout << "SZ: " << clique.size() << " = ";
                for (auto node : clique)
                    cout << node << " ";
                
                

                bool ok = true;
                vector<int> neighs = G.GetAdjacency(clique[0]);
                neighs = SetSubstract(neighs, clique);
                for (auto node : clique) {
                    auto adj = G.GetAdjacency(node);

                    int pre = neighs.size();
                    neighs = SetUnion(neighs, adj);
                    neighs = SetSubstract(neighs, clique);

                    if (pre != (int)neighs.size()) {
                        ok = false;
                        break;
                    }
                }

                cout << "(";
                for (auto node : neighs)
                    cout << node << " ";
                cout << ")";
                cout << endl;

                cout << clique.size() << " " << neighs.size() << " " << ok << endl;
                if (clique.size() > neighs.size() && ok) {
                    cout << "RULE 8 APPLICABLE" << endl;
                    assert(false);
                }

                /*
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
                G.ReduceMarksetVertexSet();
                S = G.GetMarkedVerticesByOneWayRules();
                cout << "|S| = " << S.size() << endl;
                

                MaxCutGraph wG = G;
                int res;
                while ((res = ExhaustiveTwoWayReduce(wG, S)) > -1) {
                    cout << "Kernelization rule: " << res << " was applied." << endl;
                    cout << "New G. Stats: " << "|V| = " << G.GetRealNumNodes() << " , |E| = " << wG.GetRealNumEdges() << " , EE = " << wG.GetEdwardsErdosBound() << endl;
                    assert(false);
                }
                */
            }
        }
    }
};