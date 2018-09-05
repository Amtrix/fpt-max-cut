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

            double k_change = 0;
            while (true) {
                auto res_s5 = kernelized.GetAllS5Candidates();
                if (!res_s5.empty()) {
                    kernelized.ApplyS5Candidate(res_s5[0], k_change);
                    case_coverage_cnt[8]++;
                    continue;
                }

                auto res_s4 = kernelized.GetAllS4Candidates();
                if (!res_s4.empty()) {
                    kernelized.ApplyS4Candidate(res_s4[0], k_change);
                    case_coverage_cnt[7]++;
                    continue;
                }

                auto res_rs2 = kernelized.GetS2Candidates(true);
                if (!res_rs2.empty()) {
                    kernelized.ApplyS2Candidate(res_rs2[0], k_change);
                    case_coverage_cnt[0]++;
                    continue;
                }

                auto res_rs3 = kernelized.GetS3Candidates(true);
                if (!res_rs3.empty()) {
                    kernelized.ApplyS3Candidate(res_rs3[0], k_change);
                    case_coverage_cnt[6]++;
                    continue;
                }

                auto res_r9x = kernelized.GetAllR9XCandidates();
                if (!res_r9x.empty()) {
                    kernelized.ApplyR9XCandidate(res_r9x[0], k_change);
                    case_coverage_cnt[1]++;
                    continue;
                }
                
                auto res_r8 = kernelized.GetAllR8Candidates();
                if (!res_r8.empty()) {
                    kernelized.ApplyR8Candidate(res_r8[0], k_change);
                    case_coverage_cnt[2]++;
                    continue;
                }
                
                auto res_r10 = kernelized.GetAllR10Candidates();
                if (!res_r10.empty()) {
                    kernelized.ApplyR10Candidate(res_r10[0], k_change);
                    case_coverage_cnt[3]++;
                    continue;
                }
                
                
                auto res_r9 = kernelized.GetAllR9Candidates();
                if (!res_r9.empty()) {
                    kernelized.ApplyR9Candidate(res_r9[0], k_change);
                    case_coverage_cnt[4]++;
                    continue;
                }

                
                auto res_r10ast = kernelized.GetAllR10ASTCandidates();
                if (!res_r10ast.empty()) {
                    kernelized.ApplyR10ASTCandidate(res_r10ast[0], k_change); // THERE SEEMS TO BE SOMETHING OFF HERE, BUT I HAVE NO CLUE WHAT
                    case_coverage_cnt[5]++;
                    continue;
                }

                break;
            }

            auto heur_sol = G.ComputeMaxCutHeuristically();
            auto heur_sol_k = kernelized.ComputeMaxCutHeuristically();
            VERIFY_RETURN_ON_FAIL(heur_sol.first, heur_sol_k.first - k_change);
        }

        cout << "Case coverage (=number of applications) = ";
        for (int r = 0; r < 10; ++r) cout << case_coverage_cnt[r] << " ";
        cout << endl;
    }
};

int main() {
    for (const auto test : suite) {
        test();
    }
}