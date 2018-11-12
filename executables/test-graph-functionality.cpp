#define TESTING

#include "src/mc-graph.hpp"
#include "src/checks.hpp"

#include <bits/stdc++.h>
using namespace std;

void Prepare(vector<int>& anodes, vector<vector<int>>& bcomponents) {
    sort(anodes.begin(), anodes.end());
    for (auto& component : bcomponents) sort(component.begin(), component.end());
    sort(bcomponents.begin(), bcomponents.end());
}

std::function<void()> suite[] = {
    []{ // Checks if graph disconnections are noticed properly.
        // 0 1-(2)-3 4-5 6 7 8 9
        MaxCutGraph G(10);

        G.AddEdge(1,2);
        G.AddEdge(2,3);

        G.AddEdge(4,5);
        
        VERIFY(G.DoesDisconnect({0}), false);
        VERIFY(G.DoesDisconnect({1}), false);
        VERIFY(G.DoesDisconnect({2}), true);
        VERIFY(G.DoesDisconnect({1,2,3}), false);
    }
};

int main() {
    for (const auto test : suite) {
        test();
    }
}