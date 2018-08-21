#define TESTING

#include "src/mc-graph.hpp"
#include "src/checks.hpp"

#include <bits/stdc++.h>
using namespace std;

std::function<void()> suite[] = {
    []{ // Test rules on two triangles sharing a common vertex.
        MaxCutGraph G(10, 6);

        G.AddEdge(1,2);
        G.AddEdge(2,3);

        G.AddEdge(1,4);
        G.AddEdge(4,2);

        G.AddEdge(2,5);
        G.AddEdge(5,3);
        
        auto res_r10 = G.GetAllR10Candidates();
        VERIFY(res_r10.size(), 4);

        auto res_r9x = G.GetAllR9XCandidates();
        VERIFY(res_r9x.size(), 0);

        auto res_r9 = G.GetAllR9Candidates();
        VERIFY(res_r9.size(), 1);

        auto res_r8 = G.GetAllR8Candidates();
        VERIFY(res_r8.size(), 2);
    },
    []{ // Test rules on two K4 sharing a common vertex.
        MaxCutGraph G(10, 12);

        G.AddEdge(1,2);
        G.AddEdge(2,3);
        G.AddEdge(3,4);
        G.AddEdge(4,1);
        G.AddEdge(1,3);
        G.AddEdge(2,4);

        G.AddEdge(4,5);
        G.AddEdge(5,6);
        G.AddEdge(6,7);
        G.AddEdge(7,4);
        G.AddEdge(4,6);
        G.AddEdge(5,7);
        
        auto res_r10 = G.GetAllR10Candidates();
        VERIFY(res_r10.size(), 0);

        auto res_r9x = G.GetAllR9XCandidates();
        VERIFY(res_r9x.size(), 2);

        auto res_r9 = G.GetAllR9Candidates();
        VERIFY(res_r9.size(), 0);

        auto res_r8 = G.GetAllR8Candidates();
        VERIFY(res_r8.size(), 2);
    },
    []{ // Test chain of 5.   P1-2-3-4-5Q
        MaxCutGraph G(10, 12);

        G.AddEdge(1,2);
        G.AddEdge(2,3);
        G.AddEdge(3,4);
        G.AddEdge(4,5);

        G.AddEdge(1,6);
        G.AddEdge(1,5);

        G.AddEdge(5,6);
        G.AddEdge(5,8);
        
        auto res_r10ast = G.GetAllR10ASTCandidates();
        VERIFY(res_r10ast.size(), 1);
    }
};

int main() {
    for (const auto test : suite) {
        test();
    }
}