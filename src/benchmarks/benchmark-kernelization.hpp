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
    }

    void Evaluate(InputParser &input, const MaxCutGraph &main_graph) {
        int mixingid = GetMixingId(main_graph);
        BenchmarkAction::Evaluate(input, main_graph);

        int num_iterations = 1;
        if (input.cmdOptionExists("-iterations")) {
            num_iterations = stoi(input.getCmdOption("-iterations"));
        }

        vector<vector<double>> accum;
        for (int iteration = 1; iteration <= num_iterations; ++iteration) {
            MaxCutGraph G = main_graph;
            MaxCutGraph kernelized = G;

            vector<pair<double,int>> local_times;
            auto t0 = std::chrono::high_resolution_clock::now();
            auto t0_total = std::chrono::high_resolution_clock::now();

            // First transform graph into unweighted. /////////////
            kernelized.MakeUnweighted();
            OutputDebugLog("Made unweighted");
            LogTime(local_times, t0);
            ////////////////////////////////////////

            // Reductions ////////////////////////////////////////            
            while (true) {
                FlushTimes(local_times, false);
                
                bool chg_happened = false;
                for (int i = 0; i < (int)kernelization_order.size() && !chg_happened; ++i) {
                    if (kernelized.PerformKernelization(kernelization_order.at(i)))
                        chg_happened = true;
                    LogTime(local_times, t0, static_cast<int>(kernelization_order[i]));
                }

                if (!chg_happened)
                    break; 
            }
            FlushTimes(local_times, false); // one more flush

            // Also kernelization here(!):
            OutputDebugLog("Unweithed to weighted kernelization. |V| = " + to_string(kernelized.GetNumNodes()) + ", |E| = " + to_string(kernelized.GetRealNumEdges()));
            kernelized.MakeWeighted();
            OutputDebugLog("Made weighted");
            LogTime(local_times, t0);
            FlushTimes(local_times, false);
            OutputDebugLog("Unweithed to weighted kernelization: Done. |V| = " + to_string(kernelized.GetNumNodes()) + ", |E| = " + to_string(kernelized.GetRealNumEdges()));
            ////////////////////////////////////////


            // Calculating spent time. From here on onwards, only O(1) operations allowed!!!!!!!!!!!!!!!!!!!!!
            auto t1_total = std::chrono::high_resolution_clock::now();
            double kernelization_time = std::chrono::duration_cast<std::chrono::microseconds> (t1_total - t0_total).count()/1000.;


            double k_change = kernelized.GetInflictedCutChangeToKernelized();
            double local_search_cut_size = G.ComputeLocalSearchCut().first;
            double local_search_cut_size_k = kernelized.ComputeLocalSearchCut().first;
            auto heur_sol = G.ComputeMaxCutWithMQLib(1);
            auto heur_sol_k = kernelized.ComputeMaxCutWithMQLib(1);
            double EE = G.GetEdwardsErdosBound();
            double EE_k = kernelized.GetEdwardsErdosBound();


            // Some output
            cout << "VERIFY CUT VAL: " << heur_sol_k.first - k_change << " =?= " << heur_sol.first << endl;
            cout << G.GetRealNumNodes() << " " << G.GetRealNumEdges() << endl;
            cout << kernelized.GetRealNumNodes() << " " << kernelized.GetRealNumEdges() << endl;

            auto case_coverage_cnt = kernelized.GetUsageVector();
            cout << "Spent time on kernelization[ms]: " << kernelization_time << endl;
            cout << "Case coverage (=number of applications) = ";
            for (unsigned int r = 0; r < case_coverage_cnt.size(); ++r)
                cout << case_coverage_cnt[r] << " ";
            cout << endl;

            cout << "G  = " << local_search_cut_size << " <= " << heur_sol.first << " (something weird if not)." << endl;
            cout << "Gk = " << local_search_cut_size_k << " <= " << heur_sol_k.first << " (something weird if not)." << endl;


            // Aggregating usages.
            for (auto rule : kAllRuleIds)
                tot_case_coverage_cnt[rule] += kernelized.GetRuleUsage(rule);
        

            OutputKernelization(input, main_graph.GetGraphNaming(),
                                mixingid, iteration,
                                G.GetRealNumNodes(), G.GetRealNumEdges(),
                                kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(),
                                -k_change,
                                heur_sol.first, heur_sol_k.first - k_change,
                                local_search_cut_size, local_search_cut_size_k - k_change,
                                EE, EE_k, kernelization_time);
            
            accum.push_back({(double)mixingid, (double)iteration,
                                (double)G.GetRealNumNodes(), (double)G.GetRealNumEdges(),
                                (double)kernelized.GetRealNumNodes(), (double)kernelized.GetRealNumEdges(),
                                -k_change,
                                (double)heur_sol.first, (double)heur_sol_k.first - k_change,
                                local_search_cut_size, local_search_cut_size_k - k_change,
                                EE, EE_k, kernelization_time});
        }
    
        vector<double> avg;
        avg.push_back(mixingid);
        avg.push_back(-1);
        for (int i = 2; i < 17; ++i) {
            double sum = 0;
            for (int k = 0; k < (int)accum.size(); ++k)
                sum += accum[k][i];
            avg.push_back(sum / accum.size());
        }

        OutputKernelization(input, main_graph.GetGraphNaming(), avg[0], avg[1], avg[2], avg[3], avg[4], avg[5], avg[6], avg[7], avg[8], avg[9], avg[10], avg[11], avg[12], avg[13], "-avg");
    }

    void Evaluate(InputParser& input, const string data_filepath) {
        MaxCutGraph G(data_filepath);
        Evaluate(input, G);
    }

    void PostProcess(InputParser& /* input */) override {
        cout << "Total case coverage: " << endl; // ordered according kAllRuleIds
        cout << setw(20) << "RULE" << setw(20) << "|CNT|" << setw(20) << "|TIME|" << setw(20) << "|TIME|/|CNT|" << endl;
        for (auto rule : kAllRuleIds) {
            int used_cnt = tot_case_coverage_cnt[rule];
            double used_time = times_all[static_cast<int>(rule)];
            cout << setw(20) << kRuleNames.at(rule) << setw(20) << used_cnt << setw(20) << used_time << setw(20) << (used_time/used_cnt) << endl;
        }
        cout << "Time spent on other stuff: " << times_all[-1] << endl;
        cout << endl;
        cout << endl;
    }

    const vector<RuleIds> kernelization_order = {
        RuleIds::Rule8, RuleIds::Rule9, RuleIds::Rule9X, RuleIds::Rule10, RuleIds::Rule10AST, RuleIds::RuleS2, RuleIds::RuleS3, RuleIds::RuleS4, RuleIds::RuleS5, RuleIds::RuleS6
    };

private:
    unordered_map<RuleIds, int> tot_case_coverage_cnt;
    
};