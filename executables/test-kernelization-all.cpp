/**
 * Tests specifically individual kernelization rules. Great for verifying wanted behavior where the rule is expected to happen.
 * Bad for verifying possible unintended reductions -- see -auto test for that.
 **/

#define TESTING

#include "src/mc-graph.hpp"
#include "src/checks.hpp"

#include <bits/stdc++.h>
using namespace std;

std::function<void()> suite[] = {
    []{ // Test rules on two triangles sharing a common vertex.
        MaxCutGraph G(10);

        G.AddEdge(1,2);
        G.AddEdge(2,3);

        G.AddEdge(1,4);
        G.AddEdge(4,2);

        G.AddEdge(2,5);
        G.AddEdge(5,3);
        
        auto res_r10 = G.GetR10Candidates();
        VERIFY(res_r10.size(), 4);

        auto res_r9x = G.GetR9XCandidates();
        VERIFY(res_r9x.size(), 0);

        auto res_r9 = G.GetR9Candidates();
        VERIFY(res_r9.size(), 1);

        auto res_r8 = G.GetR8Candidates();
        VERIFY(res_r8.size(), 2);
        {
            MaxCutGraph Gtmp = G;
            double chgtmp = 0;
            Gtmp.ApplyR8Candidate(res_r8[0]);
            chgtmp = Gtmp.GetInflictedCutChangeToKernelized();
            VERIFY(chgtmp, -2);
            Gtmp.ApplyR8Candidate(res_r8[1]);
            chgtmp = Gtmp.GetInflictedCutChangeToKernelized();
            VERIFY(chgtmp, -4);
        }
    },
    []{ // Test rules on two K4 sharing a common vertex.
        MaxCutGraph G(10);

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
        
        auto res_r10 = G.GetR10Candidates();
        VERIFY(res_r10.size(), 0);

        auto res_r9x = G.GetR9XCandidates();
        VERIFY(res_r9x.size(), 2);

        auto res_r9 = G.GetR9Candidates();
        VERIFY(res_r9.size(), 0);

        auto res_r8 = G.GetR8Candidates();
        VERIFY(res_r8.size(), 2);
        {
            MaxCutGraph Gtmp = G;
            double chgtmp = 0;
            Gtmp.ApplyR8Candidate(res_r8[0]);
            chgtmp = Gtmp.GetInflictedCutChangeToKernelized();
            VERIFY(chgtmp, -3);
            Gtmp.ApplyR8Candidate(res_r8[1]);
            chgtmp = Gtmp.GetInflictedCutChangeToKernelized();
            VERIFY(chgtmp, -6);
        }
    },
    []{ // Test chain of 5.   P1-2-3-4-5Q
        MaxCutGraph G(10);

        G.AddEdge(1,2);
        G.AddEdge(2,3);
        G.AddEdge(3,4);
        G.AddEdge(4,5);

        G.AddEdge(1,6);
        G.AddEdge(1,5);

        G.AddEdge(5,6);
        G.AddEdge(5,8);
        
        auto res_r10ast = G.GetR10ASTCandidates();
        VERIFY(res_r10ast.size(), 1);
    },
    []{ // Test almost K5 (missing an edge) => does not imply K4 almost, K3 almost. Degrading one external vertice to outside of component causes all internal vertices to become external!
        MaxCutGraph G(6);

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
        MaxCutGraph G(7);

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
    },
    []{
        MaxCutGraph G(6);

        for (int i = 0; i <= 5; ++i)
            for (int j = i + 1; j <= 5; ++j)
                G.AddEdge(i, j);
        
        auto kernelized = G;
        VERIFY(kernelized.PerformKernelization(RuleIds::RuleS6, {{2,true},{3,true},{4,true}}), true);

        MaxCutGraph G2(4);

        for (int i = 0; i < 4; ++i)
            for (int j = i + 1; j < 4; ++j)
                G2.AddEdge(i, j);
        
        auto kernelized2 = G2;
        VERIFY(kernelized2.PerformKernelization(RuleIds::RuleS6, {{0,true},{1,true}}), false);
    },
    []{ // Verify rule 8. K4 to 2 vertices and a single node to the same 2.
        MaxCutGraph G(20);

        for (int i = 0; i < 4; ++i)
            for (int j = i + 1; j < 4; ++j)
                G.AddEdge(i, j);

        G.AddEdge(4, 5);
        G.AddEdge(4, 6);

        for (int i = 0; i < 4; ++i) {
            G.AddEdge(i, 5);
            G.AddEdge(i, 6);
        }

        auto candidates = G.GetR8Candidates(false);
        VERIFY(candidates.size(), 1);

        G.AddEdge(4,3);
        candidates = G.GetR8Candidates(false);
        VERIFY(candidates.size(), 0);

        for (int i = 0; i < 3; ++i)
            G.AddEdge(i, 10);
        G.AddEdge(10, 3);
        G.AddEdge(10, 5);
        G.AddEdge(10, 6);
        candidates = G.GetR8Candidates(false);
        VERIFY(candidates.size(), 1);
    },
    []{
        MaxCutGraph G(4);
        G.AddEdge(0, 1);
        G.AddEdge(0, 2);
        G.AddEdge(1, 2);
        G.AddEdge(0, 3);
        G.AddEdge(1, 3);
        auto candidates = G.GetS2Candidates(false, false);
        cout << candidates.size() << " " << candidates[0] << endl;
    }
};

int main() {
    int testid = 1;
    for (const auto test : suite) {
        cout << "Running " << (testid++) << endl;
        test();
    }
}