#pragma once

#include "./benchmark-interface.hpp"
#include "../mc-graph.hpp"
#include "../one-way-reducers.hpp"
#include "../two-way-reducers.hpp"
#include "../input-parser.hpp"
#include "../utils.hpp"
#include "../output-filter.hpp"
#include "../graph-database.hpp"

#include <iostream>
using namespace std;

class Benchmark_Kernelization : public BenchmarkAction {
public:
    const static int kMQLibRunTime = 1;
    Benchmark_Kernelization() {
    }

    void Kernelize(MaxCutGraph &kernelized, bool provide_order = false, const vector<RuleIds>& provided_kernelization_order = {}) {
        // First transform graph into unweighted. /////////////
        auto t0 = std::chrono::high_resolution_clock::now();
        kernelized.MakeUnweighted();
        OutputDebugLog("Made unweighted");

        vector<pair<double,int>> local_times;
        LogTime(local_times, t0);
        ////////////////////////////////////////

        // Reductions ////////////////////////////////////////
        auto selected_kernelization_order = kernelization_order;
        if (provide_order) selected_kernelization_order = provided_kernelization_order;

        OutputDebugLog("|E(kernel)| = " + to_string(kernelized.GetRealNumEdges()) + " --- start!");

        while (true) {
            FlushTimes(local_times, false);
            
            bool chg_happened = false;
            for (int i = 0; i < (int)selected_kernelization_order.size() && !chg_happened; ++i) {
                OutputDebugLog("Trying the " + to_string(i) + "th kernelization rule");
                while (kernelized.PerformKernelization(selected_kernelization_order.at(i))) // exhaustively!
                    chg_happened = true;
                LogTime(local_times, t0, static_cast<int>(selected_kernelization_order[i]));
            }

            OutputDebugLog("|E(kernel)| = " + to_string(kernelized.GetRealNumEdges()));

            if (!chg_happened)
                break; 
        }

#ifdef DEBUG
        // With the following we make sure that our timestamping did not interferre with applicability.
        OutputDebugLog("Verifying that no more kernelization is possible when timestamps reset.");
        for (int i = 0; i < (int)selected_kernelization_order.size(); ++i) {
            kernelized.ResetTimestamps();
            custom_assert(kernelized.PerformKernelization(selected_kernelization_order.at(i)) == false);
        }
        OutputDebugLog("... done.");
#endif

        FlushTimes(local_times, false); // one more flush

        // Also kernelization here(!):
        OutputDebugLog("Unweithed to weighted kernelization. |V| = " + to_string(kernelized.GetNumNodes()) + ", |E| = " + to_string(kernelized.GetRealNumEdges()));
       // kernelized.MakeWeighted();
        OutputDebugLog("Made weighted");
        LogTime(local_times, t0);
        FlushTimes(local_times, false);
        OutputDebugLog("Unweithed to weighted kernelization: Done. |V| = " + to_string(kernelized.GetNumNodes()) + ", |E| = " + to_string(kernelized.GetRealNumEdges()));
        ////////////////////////////////////////
    }

    void Evaluate(InputParser &input, const MaxCutGraph &main_graph) {
        int mixingid = GetMixingId(main_graph);
        BenchmarkAction::Evaluate(input, main_graph);

        int num_iterations = 1;
        if (input.cmdOptionExists("-iterations")) {
            num_iterations = stoi(input.getCmdOption("-iterations"));
        }

        int locsearch_iterations = 1;
        if (input.cmdOptionExists("-locsearch-iterations")) {
            locsearch_iterations = stoi(input.getCmdOption("-locsearch-iterations"));
            cout << "Note: Local search iterations: " << locsearch_iterations << endl;
        }

        vector<vector<double>> accum;
        for (int iteration = 1; iteration <= num_iterations; ++iteration) {
            MaxCutGraph G = main_graph;
            MaxCutGraph kernelized = G;

            
            auto t0_total = std::chrono::high_resolution_clock::now();
            Kernelize(kernelized);
            // Calculating spent time. From here on onwards, only O(1) operations allowed!!!!!!!!!!!!!!!!!!!!!
            auto t1_total = std::chrono::high_resolution_clock::now();
            double kernelization_time = std::chrono::duration_cast<std::chrono::microseconds> (t1_total - t0_total).count()/1000.;


            // Already needed for upcoming sections.
            double k_change = kernelized.GetInflictedCutChangeToKernelized();

            // Compute local search results.
            vector<double> locsearch_res, locsearch_res_k;
            for (int it = 1; it <= locsearch_iterations; ++it) {
                locsearch_res.push_back(G.ComputeLocalSearchCut().first);
                locsearch_res_k.push_back(kernelized.ComputeLocalSearchCut().first - k_change); // adjust right away.
            }
            double local_search_cut_size = GetAverage(locsearch_res);
            double local_search_cut_size_k = GetAverage(locsearch_res_k);
            double local_search_sddiff = GetStandardDeviation(SubVectorVal(locsearch_res, locsearch_res_k));

            auto vec = SubVectorVal(locsearch_res, locsearch_res_k);
            cout << "Local search diffs: ";
            for (int i = 0; i < (int)vec.size(); ++i)
                cout << vec[i] << " ";
            cout << " = " << local_search_sddiff << endl;

            // Some variables.
            auto heur_sol = G.ComputeMaxCutWithMQLib(kMQLibRunTime);
            auto heur_sol_k = kernelized.ComputeMaxCutWithMQLib(kMQLibRunTime);
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
            cout << setw(20) << "RULE" << setw(20) << "|USED|" << setw(20) << "|CHECKS|" << setw(20) << "|TIME|" << setw(20) << "|TIME|/|CHECKS|" << endl;
            for (auto rule : kAllRuleIds) {
                tot_case_coverage_cnt[rule] += kernelized.GetRuleUsage(rule);
                tot_rule_checks_cnt[rule] += kernelized.GetRuleChecks(rule);

                int used_cnt = tot_case_coverage_cnt[rule];
                int check_cnt = tot_rule_checks_cnt[rule];
                double used_time = times_all[static_cast<int>(rule)] - last_times_all[static_cast<int>(rule)];
                cout << setw(20) << kRuleNames.at(rule) << setw(20) << used_cnt << setw(20) << check_cnt << setw(20) << used_time << setw(20) << (used_time/check_cnt) << endl;
            }
            last_times_all = times_all;


            OutputKernelization(input, main_graph.GetGraphNaming(),
                                mixingid, iteration,
                                G.GetRealNumNodes(), G.GetRealNumEdges(),
                                kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(),
                                -k_change,
                                heur_sol.first, heur_sol_k.first - k_change,
                                local_search_cut_size, local_search_cut_size_k, local_search_sddiff,
                                EE, EE_k, kernelization_time);
            
            accum.push_back({(double)mixingid, (double)iteration,
                                (double)G.GetRealNumNodes(), (double)G.GetRealNumEdges(),
                                (double)kernelized.GetRealNumNodes(), (double)kernelized.GetRealNumEdges(),
                                -k_change,
                                (double)heur_sol.first, (double)heur_sol_k.first - k_change,
                                local_search_cut_size, local_search_cut_size_k, local_search_sddiff,
                                EE, EE_k, kernelization_time});
        }
    
        vector<double> avg;
        avg.push_back(mixingid);
        avg.push_back(-1);
        for (int i = 2; i < 18; ++i) {
            double sum = 0;
            for (int k = 0; k < (int)accum.size(); ++k)
                sum += accum[k][i];
            avg.push_back(sum / accum.size());
        }

        OutputKernelization(input, main_graph.GetGraphNaming(), avg[0], avg[1], avg[2], avg[3], avg[4], avg[5], avg[6], avg[7], avg[8], avg[9], avg[10], avg[11], avg[12], avg[13], avg[14], "-avg");
    }

    void Evaluate(InputParser& input, const string data_filepath) {
        MaxCutGraph G(data_filepath);
        Evaluate(input, G);
    }

    void PostProcess(InputParser& /* input */) override {
        cout << "TOTAL analysis follows. " << endl; // ordered according kAllRuleIds
        cout << setw(20) << "RULE" << setw(20) << "|USED|" << setw(20) << "|CHECKS|" << setw(20) << "|TIME|" << setw(20) << "|TIME|/|CHECKS|" << endl;
        for (auto rule : kAllRuleIds) {
            int used_cnt = tot_case_coverage_cnt[rule];
            int check_cnt = tot_rule_checks_cnt[rule];
            double used_time = times_all[static_cast<int>(rule)];
            cout << setw(20) << kRuleNames.at(rule) << setw(20) << used_cnt << setw(20) << check_cnt << setw(20) << used_time << setw(20) << (used_time/check_cnt) << endl;
        }
        cout << "Time spent on other stuff: " << times_all[-1] << endl;
        cout << endl;
        cout << endl;
    }

    const vector<RuleIds> kernelization_order = {
          RuleIds::RuleS2/*, RuleIds::Rule9, RuleIds::Rule10, RuleIds::Rule10AST, RuleIds::RuleS3,
          RuleIds::RuleS4, RuleIds::RuleS5, RuleIds::RuleS6,
          RuleIds::Rule8, RuleIds::Rule9X*/
    };

private:
    unordered_map<RuleIds, int> tot_case_coverage_cnt;
    unordered_map<RuleIds, int> tot_rule_checks_cnt;
    unordered_map<int,double> last_times_all;
};