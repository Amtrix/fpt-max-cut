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
    []{ // Basic check
        // 0 1-(2)-3 4-5 6 7 8 9
        MaxCutGraph G(10);

        G.AddEdge(1,2);
        G.AddEdge(2,3);

        G.AddEdge(4,5);
        
        G.ComputeArticulationAndBiconnected();

        auto anodes = G.GetArticulationNodes();
        auto bicomponents = G.GetBiconnectedComponents();
        Prepare(anodes, bicomponents);

        VERIFY_RETURN_ON_FAIL(anodes.size(),1);
        VERIFY(anodes[0], 2);

        
        VERIFY(bicomponents.size(), 8);

        VERIFY(bicomponents[0], (vector<int>{0}));
        VERIFY(bicomponents[1], (vector<int>{1,2}));
        VERIFY(bicomponents[2], (vector<int>{2,3}));
        VERIFY(bicomponents[3], (vector<int>{4,5}));
        VERIFY(bicomponents[4], (vector<int>{6}));

    },
    []{ // Two Chain
        // 0 1-(2)-(3)-4 5-(6)-(7)-8 9
        MaxCutGraph G(10);

        G.AddEdge(1,2);
        G.AddEdge(2,3);
        G.AddEdge(3,4);

        G.AddEdge(5,6);
        G.AddEdge(6,7);
        G.AddEdge(7,8);
        G.ComputeArticulationAndBiconnected();

        auto anodes = G.GetArticulationNodes();
        auto bicomponents = G.GetBiconnectedComponents();
        Prepare(anodes, bicomponents);

        VERIFY_RETURN_ON_FAIL(anodes.size(),4);
        VERIFY(anodes[0], 2);
        VERIFY(anodes[1], 3);
        VERIFY(anodes[2], 6);
        VERIFY(anodes[3], 7);

        VERIFY(G.IsBridgeBetween(0,1), false);
        VERIFY(G.IsBridgeBetween(1,2), true);
        VERIFY(G.IsBridgeBetween(2,3), true);
        VERIFY(G.IsBridgeBetween(3,4), true);
        VERIFY(G.IsBridgeBetween(4,5), false);
        VERIFY(G.IsBridgeBetween(5,6), true);
        VERIFY(G.IsBridgeBetween(6,7), true);
        VERIFY(G.IsBridgeBetween(7,8), true);
        VERIFY(G.IsBridgeBetween(8,9), false);
        
        VERIFY(bicomponents.size(), 8);
    },
    []{ // The four triplets (docs/test-articulation-and-biconnected_3.jpg)
        MaxCutGraph G(9);

        G.AddEdge(0,1);
        G.AddEdge(1,2);

        G.AddEdge(0,3);
        G.AddEdge(3,4);
        G.AddEdge(4,0);

        G.AddEdge(2,7);
        G.AddEdge(7,8);
        G.AddEdge(8,2);

        G.AddEdge(1,5);
        G.AddEdge(5,6);
        G.AddEdge(6,1);
        G.ComputeArticulationAndBiconnected();

        auto anodes = G.GetArticulationNodes();
        auto bicomponents = G.GetBiconnectedComponents();
        Prepare(anodes, bicomponents);

        VERIFY_RETURN_ON_FAIL(anodes.size(),3);
        VERIFY(anodes[0], 0);
        VERIFY(anodes[1], 1);
        VERIFY(anodes[2], 2);

        VERIFY(G.IsBridgeBetween(0, 1), true);
        VERIFY(G.IsBridgeBetween(1, 2), true);
        VERIFY(bicomponents.size(), 5);

        // More spice:
        G.AddEdge(2,0);
        G.ComputeArticulationAndBiconnected();
        
        anodes = G.GetArticulationNodes();
        bicomponents = G.GetBiconnectedComponents();
        Prepare(anodes, bicomponents);
        
        VERIFY_RETURN_ON_FAIL(anodes.size(),3);
        VERIFY(anodes[0], 0);
        VERIFY(anodes[1], 1);
        VERIFY(anodes[2], 2);
        VERIFY(G.IsBridgeBetween(0, 1), false);
        VERIFY(G.IsBridgeBetween(1, 2), false);
        VERIFY_RETURN_ON_FAIL(bicomponents.size(), 4);

        /*
        for (auto component : bicomponents) {
            cout << "COMPONENT: " << endl;
            for (int i = 0; i < component.size(); ++i)
                cout << component[i] << " "; cout << endl;
        }
        */

        VERIFY(bicomponents[0], (vector<int>{0, 1 ,2}));
        VERIFY(bicomponents[1], (vector<int>{0, 3 ,4}));
        VERIFY(bicomponents[2], (vector<int>{1, 5 ,6}));
        VERIFY(bicomponents[3], (vector<int>{2, 7 ,8}));

        // MORE spice:
        G.AddEdge(8, 6);
        G.ComputeArticulationAndBiconnected();
        
        anodes = G.GetArticulationNodes();
        bicomponents = G.GetBiconnectedComponents();
        Prepare(anodes, bicomponents);
        
        VERIFY_RETURN_ON_FAIL(anodes.size(),1);
        VERIFY_RETURN_ON_FAIL(bicomponents.size(),2);

        // MORE AND MORE spice:
        G.AddEdge(8, 4);
        G.ComputeArticulationAndBiconnected();
        
        anodes = G.GetArticulationNodes();
        bicomponents = G.GetBiconnectedComponents();
        Prepare(anodes, bicomponents);
        
        VERIFY_RETURN_ON_FAIL(anodes.size(),0);
        VERIFY_RETURN_ON_FAIL(bicomponents.size(),1);
    }
};

int main() {
    for (const auto test : suite) {
        test();
    }
}