#pragma once

#include "./benchmark-interface.hpp"
#include "../mc-graph.hpp"
#include "../one-way-reducers.hpp"
#include "../two-way-reducers.hpp"
#include "../input-parser.hpp"
#include "../utils.hpp"
#include "../output-filter.hpp"

#include <iostream>
using namespace std;

class Benchmark_Kernelization : public BenchmarkAction {
public:
    void Evaluate(InputParser& input, const string data_filepath /*, vector<int>& tot_used_rules*/) {
        MaxCutGraph G(data_filepath);
        MaxCutGraph kernelized = G;
        kernelized.SaveNumOfComponentsForEdwardsErdosBound();
        G.SaveNumOfComponentsForEdwardsErdosBound();

        srand(123);
        double k_change = 0;
        vector<int> case_coverage_cnt(10, 0);
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

        double local_search_cut_size = G.ComputeLocalSearchCut().first;
        double local_search_cut_size_k = kernelized.ComputeLocalSearchCut().first;
        auto heur_sol = G.ComputeMaxCutHeuristically();
        auto heur_sol_k = kernelized.ComputeMaxCutHeuristically();
        double EE = G.GetEdwardsErdosBound();
        double EE_k = kernelized.GetEdwardsErdosBound();
        double k = (heur_sol.first - EE);
        double k_k = (heur_sol_k.first - EE_k) - k_change/4.0;

        cout << k << " --- " << k_k << " same value proves correctness! (But different does not incorrectness!)" << endl;
        cout << G.GetRealNumNodes() << " " << G.GetRealNumEdges() << endl;
        cout << kernelized.GetRealNumNodes() << " " << kernelized.GetRealNumEdges() << endl;
        cout << "Case coverage (=number of applications) = ";
        for (int r = 0; r < 4; ++r) cout << case_coverage_cnt[r] << " ";
        cout << endl;

        

        cout << "G  = " << local_search_cut_size << " <= " << heur_sol.first << " (something weird if not)." << endl;
        cout << "Gk = " << local_search_cut_size_k << " <= " << heur_sol_k.first << " (something weird if not)." << endl;
        

        OutputKernelization(input, data_filepath, G.GetRealNumNodes(), G.GetRealNumEdges(),
            kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(), k, k_k, heur_sol.first, heur_sol_k.first, local_search_cut_size, local_search_cut_size_k, local_search_cut_size_k + (EE - (EE_k + k_change/4.0)), EE, EE_k, -k_change/4.0);
    }
};