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
    },
    []{ // Test almost K5 (missing an edge) => does not imply K4 almost, K3 almost. Degrading one external vertice to outside of component causes all internal vertices to become external!
        MaxCutGraph G(6, 9);

        G.AddEdge(1,2);
        G.AddEdge(1,3);
        G.AddEdge(1,4);
        G.AddEdge(1,5);

        G.AddEdge(2,3);
        G.AddEdge(2,4);
        G.AddEdge(2,5);

        G.AddEdge(3,4);
        G.AddEdge(3,5);

        //G.AddEdge(4,5);
        
        
        unordered_map<int, bool> is_external;
        auto res_s3 = G.GetS3Candidates(true, is_external);
        VERIFY(res_s3.size(), 1);

        
        is_external[1] = true;
        is_external[2] = true;
        is_external[3] = true;
        // 4,5 are internal
        res_s3 = G.GetS3Candidates(true, is_external);
        VERIFY(res_s3.size(), 1);

        is_external[4] = true;
        res_s3 = G.GetS3Candidates(true, is_external);
        VERIFY(res_s3.size(), 0);
    },
    []{ // Test almost K6 (first three internal, then 2 internal) => implies K4 almost, K3 almost.
        MaxCutGraph G(7, 9);

        G.AddEdge(1,2);
        G.AddEdge(1,3);
        G.AddEdge(1,4);
        G.AddEdge(1,5);
        G.AddEdge(1,6);

        G.AddEdge(2,3);
        G.AddEdge(2,4);
        G.AddEdge(2,5);
        G.AddEdge(2,6);

        G.AddEdge(3,4);
        G.AddEdge(3,5);
        G.AddEdge(3,6);

        G.AddEdge(4,5);
        G.AddEdge(4,6);
        
        
        unordered_map<int, bool> is_external;
        auto res_s3 = G.GetS3Candidates(true, is_external);
        VERIFY(res_s3.size(), 1);

        
        is_external[1] = true;
        is_external[2] = true;
        is_external[3] = true;
        res_s3 = G.GetS3Candidates(true, is_external); // does this hold?! 3 internal in K6, one missing edge, edge can be added? yes.
        VERIFY(res_s3.size(), 1);


        is_external[4] = true;
        // 5,6 are internal, but clique even!
        res_s3 = G.GetS3Candidates(true, is_external);
        VERIFY(res_s3.size(), 0);
    }
};

int main() {
    int testid = 1;
    for (const auto test : suite) {
        cout << "Running " << (testid++) << endl;
        test();
    }
}