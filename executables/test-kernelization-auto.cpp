/**
 * When testing change diff on specific rule it might be better to remove (comment out) unrelated rules to have higher coverage for that single rule and better correctness report.
 * */

#define TESTING
#define DEBUG 0

#include "src/mc-graph.hpp"
#include "src/checks.hpp"
#include "src/utils.hpp"
#include "src/benchmarks/benchmark-kernelization.hpp"

#include <bits/stdc++.h>
using namespace std;

const int kDataSetCount = 1;
const string paths[] = {
    "../data/auto-tests/tests",
};

const bool kTakePredefinedKernelizationOrder = true;
const bool cDEBUG = false;

string serializestr(vector<int> vec) {
    string ret = "";
    for (auto node : vec)
        ret += to_string(node+1) + " ";
    return ret;
}

const vector<RuleIds> kernelization_order = {
    RuleIds::RuleS2, RuleIds::Rule8, RuleIds::RuleS5, RuleIds::RuleS3
//    RuleIds::Rule8, RuleIds::Rule9, RuleIds::Rule9X, RuleIds::Rule10, RuleIds::Rule10AST, RuleIds::RuleS2, RuleIds::RuleS3, RuleIds::RuleS4, RuleIds::RuleS5, RuleIds::RuleS6
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

        GraphDatabase graph_db(all_sets_to_evaluate);

        double k_change_hash_tot = 0;
        vector<int> case_coverage_cnt(kAllRuleIds.size(), 0); // hacky size init.
        //for (string data_filepath : all_sets_to_evaluate) {
        for (auto G : graph_db) {
            cout << "============================== RUNNING TEST INSTANCE ON " + G.GetGraphNaming() + " ================================ " << endl;

            // Kernelization here.
            MaxCutGraph kernelized = G;
            Benchmark_Kernelization kernelization_suite;
            if (kTakePredefinedKernelizationOrder) kernelization_suite.Kernelize(kernelized, true, kernelization_order, true);
            else kernelization_suite.Kernelize(kernelized);


            // Stats here.
            double k_change = kernelized.GetInflictedCutChangeToKernelized();
            auto heur_sol = G.ComputeMaxCutWithMQLib(2);
            auto heur_sol_k = kernelized.ComputeMaxCutWithMQLib(2);
            cout << "Non-kernelized: " << heur_sol.first << endl;
            cout << "Kernelized: " << heur_sol_k.first - k_change << endl;
            VERIFY_RETURN_ON_FAIL(heur_sol.first, heur_sol_k.first - k_change);

            auto rule_usages = kernelized.GetUsageVector();
            for (unsigned int i = 0 ; i < rule_usages.size(); ++i)
                case_coverage_cnt[i] += rule_usages[i];

            if (cDEBUG) cout << "DOUBLE k_change: " << k_change << endl;
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
        InitializeTest();
        test();
    }

    return HAS_FAILED_ONCE;
}