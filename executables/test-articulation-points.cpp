#include "src/mc-graph.hpp"
#include "src/checks.hpp"

#include <bits/stdc++.h>
using namespace std;

std::function<void()> suite[] = {
    []{ // Basic check
        MaxCutGraph G(10, 10);

        G.AddEdge(1,2);
        G.AddEdge(2,3);
        G.ComputeArticulationPoints();

        auto anodes = G.GetArticulationNodes();

        VERIFY_RETURN_ON_FAIL(anodes.size(),1);
        VERIFY(anodes[0], 2);
    },
    []{ // Two Chain
        MaxCutGraph G(10, 10);

        G.AddEdge(1,2);
        G.AddEdge(2,3);
        G.AddEdge(3,4);

        G.AddEdge(5,6);
        G.AddEdge(6,7);
        G.AddEdge(7,8);
        G.ComputeArticulationPoints();

        auto anodes = G.GetArticulationNodes();
        sort(anodes.begin(), anodes.end());

        VERIFY_RETURN_ON_FAIL(anodes.size(),4);
        VERIFY(anodes[0], 2);
        VERIFY(anodes[1], 3);
        VERIFY(anodes[2], 6);
        VERIFY(anodes[3], 7);
    }
};

int main() {
    for (const auto test : suite) {
        test();
    }
}