#define TESTING

#include "src/mc-graph.hpp"
#include "src/checks.hpp"
#include "src/utils.hpp"

#include <bits/stdc++.h>
using namespace std;

const int kDataSetCount = 1;
const string paths[] = {
    "../data/auto-tests",
};

std::function<void()> suite[] = {
    []{ // Test rules on two triangles sharing a common vertex.
        srand((unsigned)time(0));
        
        vector<string> all_sets_to_evaluate;
        for (unsigned int i = 0; i < kDataSetCount; ++i) {
            auto sets = GetAllDatasets(paths[i]);
            for (unsigned int i = 0; i < sets.size(); ++i)
                all_sets_to_evaluate.push_back(sets[i]);
        }

        vector<int> case_coverage_cnt(10, 0);
        for (string data_filepath : all_sets_to_evaluate) {
            cout << "================ RUNNING TEST INSTANCE ON " + data_filepath + " ================ " << endl;

            MaxCutGraph G(data_filepath);
            MaxCutGraph kernelized = G;
            kernelized.SaveNumOfComponentsForEdwardsErdosBound();
            G.SaveNumOfComponentsForEdwardsErdosBound();

            double k_change = 0;
            while (true) {
                auto res_r9 = kernelized.GetAllR9Candidates();
                if (!res_r9.empty()) {
                    kernelized.ApplyR9Candidate(res_r9[0]);
                    case_coverage_cnt[0]++;
                    continue;
                }

                auto res_r8 = kernelized.GetAllR8Candidates();
                if (!res_r8.empty()) {
                    kernelized.ApplyR8Candidate(res_r8[0]);
                    case_coverage_cnt[1]++;
                    continue;
                }

                auto res_r9x = kernelized.GetAllR9XCandidates();
                if (!res_r9x.empty()) {
                    kernelized.ApplyR9XCandidate(res_r9x[0], k_change);
                    case_coverage_cnt[2]++;
                    continue;
                }

                auto res_r10 = kernelized.GetAllR10Candidates();
                if (!res_r10.empty()) {
                    kernelized.ApplyR10Candidate(res_r10[0], k_change);
                    case_coverage_cnt[3]++;
                    continue;
                }

                break;
            }

            auto heur_sol = G.ComputeMaxCutHeuristically();
            auto heur_sol_k = kernelized.ComputeMaxCutHeuristically();
            double EE = G.GetEdwardsErdosBound();
            double EE_k = kernelized.GetEdwardsErdosBound();
            double k = (heur_sol.first - EE);
            double k_k = (heur_sol_k.first - EE_k) - k_change/4.0;
            VERIFY_RETURN_ON_FAIL(k, k_k);
            //double local_search_cut_size = G.ComputeLocalSearchCut().first;
            //double local_search_cut_size_k = kernelized.ComputeLocalSearchCut().first;
            
            /*
            cout << "STATS: " << endl;
            cout << "Nodes: " << G.GetAllExistingNodes().size() << " " << kernelized.GetAllExistingNodes().size() << endl;
            cout << "Edges: " << G.GetAllExistingEdges().size() << " " << kernelized.GetAllExistingEdges().size() << endl;
            cout << "MQLib sol G: " << heur_sol.first << endl;
            cout << "MQLib sol G': " << heur_sol_k.first << endl;
            cout << "EE': " << EE_k << endl;
            cout << "EE: " << EE << endl;
            cout << "LOCAL_SEARCH(G) = " << local_search_cut_size << endl;
            cout << "LOCAL_SEARCH(G') = " << local_search_cut_size_k << endl;
            cout << "LOW_BOUND_CUT_SIZE_G'(G) = " << local_search_cut_size_k + (EE - EE_k) + -k_change/4.0 << endl;
            */

            cout << "k = " << k << endl;
            cout << "k' = " << k_k << endl;
        }

        cout << "Case coverage (=number of applications) = ";
        for (int r = 0; r < 4; ++r) cout << case_coverage_cnt[r] << " ";
        cout << endl;
    }
};

int main() {
    for (const auto test : suite) {
        test();
    }
}