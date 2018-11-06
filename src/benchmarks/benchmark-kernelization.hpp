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
    const static int kSolverRuntime = 1;
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
                while (kernelized.PerformKernelization(selected_kernelization_order.at(i))) { // exhaustively!
                    chg_happened = true;
                    OutputDebugLog("Trying the " + to_string(i) + "th kernelization rule... again");
                }
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
            OutputDebugLog("        Trying the " + to_string(i) + "th kernelization rule");
            kernelized.ResetTimestamps();
            custom_assert(kernelized.PerformKernelization(selected_kernelization_order.at(i)) == false);
        }
        OutputDebugLog("... done.");
#endif

        FlushTimes(local_times, false); // one more flush

        // Also kernelization here(!):
        OutputDebugLog("Unweithed to weighted kernelization. |V| = " + to_string(kernelized.GetNumNodes()) + ", |E| = " + to_string(kernelized.GetRealNumEdges()));
        kernelized.MakeWeighted();
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
        int mqlib_iterations = 1;
        if (input.cmdOptionExists("-mqlib-iterations")) {
            mqlib_iterations = stoi(input.getCmdOption("-mqlib-iterations"));
            cout << "Note: MQLIB solver iterations: " << mqlib_iterations << endl;
        }
        int localsolver_iterations = 1;
        if (input.cmdOptionExists("-localsolver-iterations")) {
            localsolver_iterations = stoi(input.getCmdOption("-localsolver-iterations"));
            cout << "Note: localsolver solver iterations: " << localsolver_iterations << endl;
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

            // Compute solver results.
            double local_search_cut_size = -1, local_search_cut_size_k = -1, local_search_sddiff = 0;
            double mqlib_cut_size = -1, mqlib_cut_size_k = -1, mqlib_sddiff = 0;
            double localsolver_cut_size = -1, localsolver_cut_size_k = -1, localsolver_sddiff = 0;

            int local_search_cut_size_best = -1, mqlib_cut_size_best = -1, localsolver_cut_size_best = -1;

            vector<int> tmp_def_param_trash;
            std::tie(local_search_cut_size, local_search_cut_size_k, local_search_sddiff, local_search_cut_size_best)
                = ComputeAverageAndDeviation(TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeLocalSearchCut, &G, tmp_def_param_trash)),
                                             TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeLocalSearchCut, &kernelized, tmp_def_param_trash), -k_change),
                                             locsearch_iterations);


            std::tie(mqlib_cut_size, mqlib_cut_size_k, mqlib_sddiff, mqlib_cut_size_best)
                = ComputeAverageAndDeviation(TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithMQLib, &G, kSolverRuntime)),
                                             TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithMQLib, &kernelized, kSolverRuntime), -k_change),
                                             mqlib_iterations);

#ifdef LOCALSOLVER_EXISTS
            std::tie(localsolver_cut_size, localsolver_cut_size_k, localsolver_sddiff, localsolver_cut_size_best)
                = ComputeAverageAndDeviation(TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithLocalsolver, &G, kSolverRuntime)),
                                             TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithLocalsolver, &kernelized, kSolverRuntime), -k_change),
                                             localsolver_iterations);
#endif

            // Some variables.
            double EE = G.GetEdwardsErdosBound();
            double EE_k = kernelized.GetEdwardsErdosBound();
            int MAXCUT_best_size = max(local_search_cut_size_best, max(mqlib_cut_size_best,localsolver_cut_size_best));

            // Some output
            cout << "VERIFY CUT VAL:  localsearch(" << local_search_cut_size << ", " << local_search_cut_size_k << ")   mqlib(" << mqlib_cut_size << ", " << mqlib_cut_size_k << ")" << endl;
            cout << "              G: " << G.GetRealNumNodes() << " " << G.GetRealNumEdges() << endl;
            cout << "     kernelized: " << kernelized.GetRealNumNodes() << " " << kernelized.GetRealNumEdges() << endl;

            auto case_coverage_cnt = kernelized.GetUsageVector();
            cout << "Spent time on kernelization[ms]: " << kernelization_time << endl;
            cout << "Case coverage (=number of applications) = ";
            for (unsigned int r = 0; r < case_coverage_cnt.size(); ++r)
                cout << case_coverage_cnt[r] << " ";
            cout << endl;


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
                                mqlib_cut_size, mqlib_cut_size_k, mqlib_sddiff,
                                localsolver_cut_size, localsolver_cut_size_k, localsolver_sddiff,
                                local_search_cut_size, local_search_cut_size_k, local_search_sddiff,
                                EE, EE_k, MAXCUT_best_size, kernelization_time);
            
            accum.push_back({(double)mixingid, (double)iteration,
                                (double)G.GetRealNumNodes(), (double)G.GetRealNumEdges(),
                                (double)kernelized.GetRealNumNodes(), (double)kernelized.GetRealNumEdges(),
                                -k_change,
                                (double)mqlib_cut_size, (double)mqlib_cut_size_k, (double)mqlib_sddiff,
                                localsolver_cut_size, localsolver_cut_size_k, localsolver_sddiff,
                                local_search_cut_size, local_search_cut_size_k, local_search_sddiff,
                                EE, EE_k, (double)MAXCUT_best_size, kernelization_time});
        }

        custom_assert(accum.size() > 0);
    
        vector<double> avg;
        avg.push_back(mixingid);
        avg.push_back(-1);
        const int vars_num = accum.at(0).size();
        for (int i = 2; i < vars_num; ++i) {
            double sum = 0;
            for (int k = 0; k < (int)accum.size(); ++k)
                sum += accum.at(k).at(i);
            avg.push_back(sum / accum.size());
        }

        OutputKernelization(input, main_graph.GetGraphNaming(), avg[0], avg[1], avg[2], avg[3], avg[4], avg[5], avg[6], avg[7], avg[8], avg[9], avg[10], avg[11], avg[12], avg[13], avg[14], avg[15], avg[16], avg[17], avg[18], avg[19], "-avg");
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
          RuleIds::RuleS2, RuleIds::Rule8/*,  RuleIds::Rule10AST , RuleIds::Rule10,, RuleIds::RuleS3,
          RuleIds::RuleS4, RuleIds::RuleS5, RuleIds::RuleS6,
          , RuleIds::Rule9X               EXCLUDED DUE TO INCLUSION:       RuleIds::Rule9*/
    };
    // RuleS2 covers Rule9 wholly.
    // Rule8 can imply a graph where RuleS2 may be further applicable after exhaustion.

private:
    unordered_map<RuleIds, int> tot_case_coverage_cnt;
    unordered_map<RuleIds, int> tot_rule_checks_cnt;
    unordered_map<int,double> last_times_all;
};