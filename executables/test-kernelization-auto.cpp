/**
 * When testing change diff on specific rule it might be better to remove (comment out) unrelated rules to have higher coverage for that single rule and better correctness report.
 * */

#define TESTING

#include "src/mc-graph.hpp"
#include "src/checks.hpp"
#include "src/utils.hpp"

#include <bits/stdc++.h>
using namespace std;

const int kDataSetCount = 1;
const string paths[] = {
    "../data/auto-tests/tests",
};

const bool DEBUG = false;

string serializestr(vector<int> vec) {
    string ret = "";
    for (auto node : vec)
        ret += to_string(node+1) + " ";
    return ret;
}

std::function<void()> suite[] = {
    []{ // Test rules on two triangles sharing a common vertex.
        srand((unsigned)time(0));
        
        vector<string> all_sets_to_evaluate;
        for (unsigned int i = 0; i < kDataSetCount; ++i) {
            auto sets = GetAllDatasets(paths[i]);
            for (unsigned int i = 0; i < sets.size(); ++i)
                all_sets_to_evaluate.push_back(sets[i]);
        }

        double k_change_hash_tot = 0;
        vector<int> case_coverage_cnt(kAllRuleIds.size(), 0); // hacky size init.
        for (string data_filepath : all_sets_to_evaluate) {
            cout << "================ RUNNING TEST INSTANCE ON " + data_filepath + " ================ " << endl;

            MaxCutGraph G(data_filepath);
            MaxCutGraph kernelized = G;

            while (true) {
                auto res_s5 = kernelized.GetAllS5Candidates();
                if (!res_s5.empty()) {
                    kernelized.ApplyS5Candidate(res_s5[0]);
                    if (DEBUG) cout << "Rule S5 " << endl;
                    continue;
                }

                auto res_s4 = kernelized.GetAllS4Candidates();
                if (!res_s4.empty()) {
                    kernelized.ApplyS4Candidate(res_s4[0]);
                    if (DEBUG) cout << "Rule S4 " << endl;
                    continue;
                }

                auto res_rs2 = kernelized.GetS2Candidates(true);
                if (!res_rs2.empty()) {
                    kernelized.ApplyS2Candidate(res_rs2[0]);
                    if (DEBUG) cout << "Rule S2 " << serializestr(res_rs2[0]) << endl;
                    continue;
                }

                auto res_rs3 = kernelized.GetS3Candidates(true);
                if (!res_rs3.empty()) {
                    kernelized.ApplyS3Candidate(res_rs3[0]);
                    continue;
                }

                auto res_r9x = kernelized.GetAllR9XCandidates();
                if (!res_r9x.empty()) {
                    kernelized.ApplyR9XCandidate(res_r9x[0]);
                    continue;
                }
                
                auto res_r8 = kernelized.GetAllR8Candidates();
                if (!res_r8.empty()) {
                    kernelized.ApplyR8Candidate(res_r8[0]);
                    if (DEBUG) cout << "Rule R8 " << serializestr(res_r8[0]) << endl;
                    continue;
                }
                
                auto res_r10 = kernelized.GetAllR10Candidates();
                if (!res_r10.empty()) {
                    kernelized.ApplyR10Candidate(res_r10[0]);
                    continue;
                }
                
                
                auto res_r9 = kernelized.GetAllR9Candidates();
                if (!res_r9.empty()) {
                    kernelized.ApplyR9Candidate(res_r9[0]);
                    continue;
                }

                
                auto res_r10ast = kernelized.GetAllR10ASTCandidates();
                if (!res_r10ast.empty()) {
                    kernelized.ApplyR10ASTCandidate(res_r10ast[0]); // THERE SEEMS TO BE SOMETHING OFF HERE, BUT I HAVE NO CLUE WHAT
                    continue;
                }

                break;
            }

            kernelized.MakeWeighted();
            kernelized.MakeUnweighted();
            kernelized.MakeWeighted();

            double k_change = kernelized.GetInflictedCutChangeToKernelized();
            auto heur_sol = G.ComputeMaxCutWithMQLib();
            auto heur_sol_k = kernelized.ComputeMaxCutWithMQLib();
            VERIFY_RETURN_ON_FAIL(heur_sol.first, heur_sol_k.first - k_change);

            auto rule_usages = kernelized.GetUsageVector();
            for (unsigned int i = 0 ; i < rule_usages.size(); ++i)
                case_coverage_cnt[i] += rule_usages[i];

            if (DEBUG) cout << "DOUBLE k_change: " << k_change << endl;
            k_change_hash_tot += k_change;
        }

        cout << "Total k_change: " << k_change_hash_tot << endl;
        cout << "Case coverage (=number of applications) = ";
        for (unsigned int r = 0; r < kAllRuleIds.size(); ++r) cout << case_coverage_cnt[r] << " ";
        cout << endl;
    }
};

int main() {
    for (const auto test : suite) {
        test();
    }
}