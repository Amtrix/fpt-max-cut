#pragma once

#include "../mc-graph.hpp"
#include "../one-way-reducers.hpp"
#include "../two-way-reducers.hpp"
#include "../input-parser.hpp"
#include "../utils.hpp"
#include "../output-filter.hpp"

#include <iostream>
using namespace std;



// WRONG AS OF NOW, AS IT WORKS WITH VERTEX COVER, BUT SHOULD USE EDGE COVER
void EvaluateDatasetCliqueDecomposition(InputParser& input, const string data_filepath) {
    (void) input;
    cout << "================ RUNNING BENCHMARK ON " + data_filepath + " ================ " << endl;
    MaxCutGraph G(data_filepath);

    auto cdecomposition = G.DecomposeIntoCliques();
    cout << cdecomposition.size() << endl;

    vector<int> cnt(G.GetNumNodes(), 0); // how many times node i contained in a clique
    for (auto component : cdecomposition) {
        for (auto node : component)
            cnt[node]++;
    }

    int mxcnt = 0;
    vector<int> cntrev(G.GetNumNodes(), 0); // how many times is a node contained in only i cliques.
    vector<vector<int>> cntrev_content(G.GetNumNodes()); // all nodes contained in only i cliques.
    for (int i = 0; i < G.GetNumNodes(); ++i) {
        cntrev[cnt[i]]++;
        cntrev_content[cnt[i]].push_back(i);
        mxcnt = max(mxcnt, cnt[i]);
    }

    long long fac = 1;
    for (int i = 1; i <= mxcnt; ++i) {
        MaxCutGraph newG(G, cntrev_content[i]);
        auto connected_components = newG.GetAllConnectedComponents();
        cout << "There are " << cntrev[i] << " nodes shared among " << i << " cliques. They are spread out into " << connected_components.size() << " connected components." << endl;

        for (auto component : connected_components)
            fac *= (component.size() + 1);
    }

    cout << "Estimated calculation steps: " << fac << endl;
    OutputCliqueDecompositionIntersection(input, data_filepath, G.GetNumNodes(), G.GetRealNumEdges(), fac);
}