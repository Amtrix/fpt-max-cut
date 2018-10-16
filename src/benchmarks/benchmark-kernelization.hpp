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
    Benchmark_Kernelization() {
        tot_case_coverage_cnt = vector<int>(kAllRuleIds.size(), 0);
    }

    inline void LogTime(vector<double> &times, auto &t0) {
        auto t1 = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000.);
        t0 = t1;
    }

    inline void FlushTimes(vector<double> &times, const bool print = true) {
        if (print) {
            cout << "Times from previous run: ";
            double cumm = 0;
            for (unsigned int i = 0; i < times.size(); ++i) {
                cout << times[i] << " ";
                cumm += times[i];
            }
            cout << " = " << cumm << endl;
        }
        times.clear();
    }

    void Evaluate(InputParser& input, const string data_filepath /*, vector<int>& tot_used_rules*/) {
        int num_iterations = 1;
        if (input.cmdOptionExists("-iterations")) {
            num_iterations = stoi(input.getCmdOption("-iterations"));
        }

        vector<vector<double>> accum;
        for (int iteration = 1; iteration <= num_iterations; ++iteration) {
            MaxCutGraph G(data_filepath);
            MaxCutGraph kernelized = G;

            kernelized.MakeUnweighted();

            vector<double> times;
            auto t0 = std::chrono::high_resolution_clock::now();
            while (true) {
                FlushTimes(times, false);

                auto res_rs2 = kernelized.GetS2Candidates(true);
                LogTime(times, t0);
                if (!res_rs2.empty()) {
                    kernelized.ApplyS2Candidate(res_rs2[0]);
                    continue;
                }

                auto res_r9x = kernelized.GetAllR9XCandidates(true);
                LogTime(times, t0);
                if (!res_r9x.empty()) {
                    kernelized.ApplyR9XCandidate(res_r9x[0]);
                    continue;
                }
                
                auto res_r8 = kernelized.GetAllR8Candidates(true);
                LogTime(times, t0);
                if (!res_r8.empty()) {
                    kernelized.ApplyR8Candidate(res_r8[0]);
                    continue;
                }
                
                auto res_r10 = kernelized.GetAllR10Candidates(true);
                LogTime(times, t0);
                if (!res_r10.empty()) {
                    kernelized.ApplyR10Candidate(res_r10[0]);
                    continue;
                }
                
                auto res_r9 = kernelized.GetAllR9Candidates(true);
                LogTime(times, t0);
                if (!res_r9.empty()) {
                    kernelized.ApplyR9Candidate(res_r9[0]);
                    continue;
                }

                auto res_r10ast = kernelized.GetAllR10ASTCandidates(true);
                LogTime(times, t0);
                if (!res_r10ast.empty()) {
                    kernelized.ApplyR10ASTCandidate(res_r10ast[0]);
                    continue;
                }

                auto res_s5 = kernelized.GetAllS5Candidates(true);
                LogTime(times, t0);
                if (!res_s5.empty()) {
                    kernelized.ApplyS5Candidate(res_s5[0]);
                    continue;
                }

                auto res_s4 = kernelized.GetAllS4Candidates(true);
                LogTime(times, t0);
                LogTime(times, t0);
                if (!res_s4.empty()) {
                    kernelized.ApplyS4Candidate(res_s4[0]);
                    continue;
                }

                auto res_rs3 = kernelized.GetS3Candidates(true);
                LogTime(times, t0);
                if (!res_rs3.empty()) {
                    kernelized.ApplyS3Candidate(res_rs3[0]);
                    continue;
                }
                
                break;
            }
            FlushTimes(times);

            kernelized.MakeWeighted();

            double k_change = kernelized.GetInflictedCutChangeToKernelized();
            double local_search_cut_size = G.ComputeLocalSearchCut().first;
            double local_search_cut_size_k = kernelized.ComputeLocalSearchCut().first;
            auto heur_sol = G.ComputeMaxCutWithMQLib(1);
            auto heur_sol_k = kernelized.ComputeMaxCutWithMQLib(1);
            double EE = G.GetEdwardsErdosBound();
            double EE_k = kernelized.GetEdwardsErdosBound();
            //double k = (heur_sol.first - EE);

            cout << "VERIFY CUT VAL: " << heur_sol_k.first - k_change << " =?= " << heur_sol.first << endl;
            //cout << k << " --- " << k_k << " same value proves correctness! (But different does not incorrectness!)" << endl;
            cout << G.GetRealNumNodes() << " " << G.GetRealNumEdges() << endl;
            cout << kernelized.GetRealNumNodes() << " " << kernelized.GetRealNumEdges() << endl;

            auto case_coverage_cnt = kernelized.GetUsageVector();
            cout << "Case coverage (=number of applications) = ";
            for (unsigned int r = 0; r < case_coverage_cnt.size(); ++r) {
                cout << case_coverage_cnt[r] << " ";
                tot_case_coverage_cnt[r] += case_coverage_cnt[r];
            }
            cout << endl;

            

            cout << "G  = " << local_search_cut_size << " <= " << heur_sol.first << " (something weird if not)." << endl;
            cout << "Gk = " << local_search_cut_size_k << " <= " << heur_sol_k.first << " (something weird if not)." << endl;
            

            OutputKernelization(input, data_filepath,
                                test_id, iteration,
                                G.GetRealNumNodes(), G.GetRealNumEdges(),
                                kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(),
                                -k_change,
                                heur_sol.first, heur_sol_k.first - k_change,
                                local_search_cut_size, local_search_cut_size_k - k_change,
                                EE, EE_k);
            
            accum.push_back({(double)test_id, (double)iteration,
                                (double)G.GetRealNumNodes(), (double)G.GetRealNumEdges(),
                                (double)kernelized.GetRealNumNodes(), (double)kernelized.GetRealNumEdges(),
                                -k_change,
                                (double)heur_sol.first, (double)heur_sol_k.first - k_change,
                                local_search_cut_size, local_search_cut_size_k - k_change,
                                EE, EE_k});
        }
        
        vector<double> avg;
        avg.push_back(test_id);
        avg.push_back(-1);
        for (int i = 2; i < 17; ++i) {
            double sum = 0;
            for (int k = 0; k < (int)accum.size(); ++k)
                sum += accum[k][i];
            avg.push_back(sum / accum.size());
        }

        OutputKernelization(input, data_filepath, avg[0], avg[1], avg[2], avg[3], avg[4], avg[5], avg[6], avg[7], avg[8], avg[9], avg[10], avg[11], avg[12], "-avg");

        test_id++;
    }

    void PostProcess(InputParser& /* input */) override {
        cout << "Total case coverage: " << endl;
        for (int i = 0; i < (int)tot_case_coverage_cnt.size(); ++i)
            cout << tot_case_coverage_cnt[i] << " ";
        cout << endl;
    }

private:
    vector<int> tot_case_coverage_cnt;
    int test_id = 1;
};