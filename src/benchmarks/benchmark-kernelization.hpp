#pragma once

#include "./benchmark-interface.hpp"
#include "../mc-graph.hpp"
#include "../one-way-reducers.hpp"
#include "../two-way-reducers.hpp"
#include "../input-parser.hpp"
#include "../utils.hpp"
#include "../output-filter.hpp"
#include "../graph-database.hpp"
#include "./solver-helper.hpp"

#include <iostream>
#include <thread>
using namespace std;

class Benchmark_Kernelization : public BenchmarkAction {
public:
    const static bool kMakeWeightedAtEnd = false;

    Benchmark_Kernelization() {
    }



    bool KernelizeExec(MaxCutGraph &kernelized, const vector<RuleIds>& provided_kernelization_order, const bool reset_timestamps_each_time = false) {
        cout << ("|V(kernel)| = " + to_string(kernelized.GetRealNumNodes()) + "  |E(kernel)| = " + to_string(kernelized.GetRealNumEdges()) + " ------------------------------------------- start!") << endl;
        cout << "Given list of kernelizations to execute" << endl;
        for (auto rule : provided_kernelization_order)
            cout << kRuleNames.at(rule) << " ";
        cout << endl;


        auto t0 = GetCurrentTime();
        bool tot_chg_happened = false;
        while (true) {

            if (reset_timestamps_each_time) {
                kernelized.ResetTimestamps(); // HEAVY SLOWDOWN!
            }
            
            bool chg_happened = false;
            for (int i = 0; i < (int)provided_kernelization_order.size() /* && !chg_happened**/; ++i) { // some basics tests have shown that performance remains same even if the !chg_happened part is removed. This helps detect inclusions!
                cout << ("Trying the " + kRuleNames.at(provided_kernelization_order.at(i)) + " kernelization rule. Timestamps: " + to_string(!reset_timestamps_each_time)) << endl;
                int cnt = 0;
                while (kernelized.PerformKernelization(provided_kernelization_order.at(i))) { // exhaustively!
                    chg_happened = true;
                    cnt++;
                }
                cout << ("         ... executed applications " + to_string(cnt) + " times.") << endl;
                cout << ("             cut_change =  " + to_string(kernelized.GetInflictedCutChangeToKernelized())) << endl;
                LogTime(t0, static_cast<int>(provided_kernelization_order[i]));
            }

            cout << ("|V(kernel)| = " + to_string(kernelized.GetRealNumNodes()) + "  |E(kernel)| = " + to_string(kernelized.GetRealNumEdges()) + " , cut_change = " + to_string(kernelized.GetInflictedCutChangeToKernelized())) << endl;

            if (!chg_happened) break; 
            else tot_chg_happened = true;
        }

        return tot_chg_happened;
    }




    void Kernelize(MaxCutGraph &kernelized, bool provide_order = false, const vector<RuleIds>& provided_kernelization_order = {}, const bool force_timestampless_kernelization = false) {
        // First transform graph into unweighted. /////////////
        auto t0 = GetCurrentTime();
        //kernelized.MakeWeighted();

        auto selected_kernelization_order = kernelization_order;
        if (provide_order) selected_kernelization_order = provided_kernelization_order;

        bool is_all_finished = false;
        while (!is_all_finished) {
            is_all_finished = true;
            
            // Signed reductions.
           // kernelized.MakeSigned();
          //  if (KernelizeExec(kernelized, {RuleIds::Rule8Signed}, false))
          //      is_all_finished = false;
        

            // Unweighted reductions.
            kernelized.MakeUnweighted();
            OutputDebugLog("Made unweighted");
            LogTime(t0);

            if (KernelizeExec(kernelized, selected_kernelization_order, false))
                is_all_finished = false;
        }

        auto t_end_fast = std::chrono::high_resolution_clock::now();
        double time_fast_kernelization = std::chrono::duration_cast<std::chrono::microseconds> (t_end_fast - t0).count()/1000.;
        (void) time_fast_kernelization;
        OutputDebugLog("INITIAL -- FAST KERNELIZATION DONE! Time: " + to_string(time_fast_kernelization));

        if (force_timestampless_kernelization) {
            KernelizeExec(kernelized, selected_kernelization_order, true);
        } else {
#ifndef SKIP_FAST_KERNELIZATION_CHECK
            custom_assert(KernelizeExec(kernelized, selected_kernelization_order, true) == false); // will only trigger if DEBUG defined, due to custom_assert definition.
#else
            KernelizeExec(kernelized, selected_kernelization_order, true);
#endif
        }

        KernelizeExec(kernelized, finishing_rules_order, true);

        // Also kernelization here(!):
        t0 = GetCurrentTime();
        if (kMakeWeightedAtEnd || inputFlagToWeightedIsSet) {
            OutputDebugLog("Unweithed to weighted kernelization. |V| = " + to_string(kernelized.GetRealNumNodes()) + ", |E| = " + to_string(kernelized.GetRealNumEdges()));
            kernelized.MakeWeighted();
            OutputDebugLog("Unweithed to weighted kernelization: Done. |V| = " + to_string(kernelized.GetRealNumNodes()) + ", |E| = " + to_string(kernelized.GetRealNumEdges()));
        } else {
            OutputDebugLog("To weighted conversation is skipped.");
        }
        LogTime(t0);
        ////////////////////////////////////////
    }





    void Evaluate(InputParser &input, const MaxCutGraph &main_graph) {
        GenerateMissingMixingId(main_graph);
        int mixingid = GetMixingId(main_graph);
        

        int num_iterations = 1;
        if (input.cmdOptionExists("-iterations")) {
            num_iterations = stoi(input.getCmdOption("-iterations"));
        }

        if (input.cmdOptionExists("-support-weighted-result")) {
            inputFlagToWeightedIsSet = true;
        } else {
            inputFlagToWeightedIsSet = false;
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


            // Compute solver results.
            int sub_on_kernelized_runtime = 0;
            sub_on_kernelized_runtime = round(kernelization_time / 1000.0);            
            SolverEvaluation::Evaluate(mixingid, input, sub_on_kernelized_runtime, G, kernelized);
            

            // Some variables.
            double EE = G.GetEdwardsErdosBound();
            double EE_k = kernelized.GetEdwardsErdosBound();

            // Some output
            cout << "VERIFY CUT VAL:  localsearch(" << SolverEvaluation::local_search_cut_size << ", " << SolverEvaluation::local_search_cut_size_k
                 << ")   mqlib(" << SolverEvaluation::mqlib_cut_size << ", " << SolverEvaluation::mqlib_cut_size_k << ")" << endl;
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

                int used_cnt = kernelized.GetRuleUsage(rule);
                int check_cnt = kernelized.GetRuleChecks(rule);
                double used_time = times_all[static_cast<int>(rule)] - last_times_all[static_cast<int>(rule)];
                cout << setw(20) << kRuleNames.at(rule) << setw(20) << used_cnt << setw(20) << check_cnt << setw(20) << used_time << setw(20) << (used_time/check_cnt) << endl;
            }
            last_times_all = times_all;

            double k_change = kernelized.GetInflictedCutChangeToKernelized();
            custom_assert(SolverEvaluation::biqmac_cut_size == SolverEvaluation::biqmac_cut_size_k + int(-k_change) || SolverEvaluation::biqmac_cut_size == -1 || SolverEvaluation::biqmac_cut_size_k == -1);
            
            OutputKernelization(input, main_graph.GetGraphNaming(),
                                mixingid, iteration,
                                G.GetRealNumNodes(), G.GetRealNumEdges(),
                                kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(),
                                -k_change,
                                SolverEvaluation::mqlib_cut_size, SolverEvaluation::mqlib_cut_size_k, SolverEvaluation::mqlib_rate, SolverEvaluation::mqlib_rate_sddiff,
                                SolverEvaluation::localsolver_cut_size, SolverEvaluation::localsolver_cut_size_k, SolverEvaluation::localsolver_rate, SolverEvaluation::localsolver_rate_sddiff,
                                SolverEvaluation::local_search_cut_size, SolverEvaluation::local_search_cut_size_k, SolverEvaluation::local_search_rate, SolverEvaluation::local_search_rate_sddiff,
                                SolverEvaluation::biqmac_time, SolverEvaluation::biqmac_time_k, 
                                EE, EE_k, SolverEvaluation::MAXCUT_best_size, kernelization_time);
            
            accum.push_back({(double)mixingid, (double)iteration,
                                (double)G.GetRealNumNodes(), (double)G.GetRealNumEdges(),
                                (double)kernelized.GetRealNumNodes(), (double)kernelized.GetRealNumEdges(),
                                -k_change,
                                (double)SolverEvaluation::mqlib_cut_size, (double)SolverEvaluation::mqlib_cut_size_k, SolverEvaluation::mqlib_rate, SolverEvaluation::mqlib_rate_sddiff,
                                SolverEvaluation::localsolver_cut_size, SolverEvaluation::localsolver_cut_size_k, SolverEvaluation::localsolver_rate, SolverEvaluation::localsolver_rate_sddiff,
                                SolverEvaluation::local_search_cut_size, SolverEvaluation::local_search_cut_size_k, SolverEvaluation::local_search_rate, SolverEvaluation::local_search_rate_sddiff,
                                SolverEvaluation::biqmac_time, SolverEvaluation::biqmac_time_k, 
                                EE, EE_k, (double)SolverEvaluation::MAXCUT_best_size, kernelization_time});
            
            if (iteration == 1 && input.cmdOptionExists("-output-graphs-dir")) {
                G.PrintGraph(input.getCmdOption("-output-graphs-dir") + to_string(mixingid), true);
                kernelized.PrintGraph(input.getCmdOption("-output-graphs-dir") + to_string(mixingid) + "-kernelized", true);
            }
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

        OutputKernelization(input, main_graph.GetGraphNaming(), avg[0], avg[1], avg[2], avg[3], avg[4], avg[5], avg[6], avg[7], avg[8], avg[9], avg[10], avg[11], avg[12], avg[13], avg[14], avg[15], avg[16], avg[17], avg[18], avg[19], avg[20], avg[21], avg[22], avg[23], avg[24], "-avg");
    }

    void Evaluate(InputParser& input, const string data_filepath) {
        MaxCutGraph G(data_filepath);
        Evaluate(input, G);
    }

    void PostProcess(InputParser& input) override {
        int num_iterations = 1;
        if (input.cmdOptionExists("-iterations")) {
            num_iterations = stoi(input.getCmdOption("-iterations"));
        }

        cout << "TOTAL analysis follows. (time in milliseconds, all values divided by number of iterations[" << num_iterations << "])" << endl; // ordered according kAllRuleIds
        cout << setw(20) << "RULE" << setw(20) << "|USED|" << setw(20) << "|CHECKS|" << setw(20) << "|TIME|" << setw(20) << "|TIME|/|CHECKS|" << endl;

        double tot_time = 0;
        for (auto rule : kAllRuleIds) {
            int used_cnt = tot_case_coverage_cnt[rule];
            int check_cnt = tot_rule_checks_cnt[rule];
            double used_time = times_all[static_cast<int>(rule)];
            tot_time += used_time;

            cout << setw(20) << kRuleNames.at(rule) << setw(20) << (used_cnt / (double) num_iterations)
                 << setw(20) << (check_cnt / (double) num_iterations) << setw(20) << (used_time / (double) num_iterations)
                 << setw(20) << (used_time/check_cnt) << endl; // this last value does not need to be divided by numm_iterations!!!
        }
        cout << "Time spent on other stuff: " << times_all[-1] << endl;
        cout << "TOTAL time (all iterations included): " << tot_time + times_all[-1] << endl;
        cout << endl;
        cout << endl;
    }

    const vector<RuleIds> kernelization_order = {
          RuleIds::RuleS2, RuleIds::Rule8, RuleIds::RuleS5, RuleIds::RuleS3/*

                    ON REMOVED RULES(!!!!):
                         EXCLUDED DUE TO INCLUSION:       RuleIds::Rule9 (S2), RuleIds::Rule9X (S2), RuleIds::Rule10AST (S5)
                         EXCLUDED (see below reasons):    RuleIds::Rule10
                         VERY LITTLE USAGE: RuleIds::RuleS4   (argue in thesis though why you left it out (if you do it))
                         
                         */
    };

    const vector<RuleIds> finishing_rules_order = {
        RuleIds::RuleS6
    };

    // IMPORTANT INFORMATION:
    // RuleS2 covers Rule9 wholly.
    // Rule8 can imply a graph where RuleS2 may be further applicable after exhaustion.
    // RuleS6 and RuleS3 are two opposites. They should not be mixed => infinite loop!!
    // Rule10: The non-bridge case is trivially covered by S2. What about the bridge case? It seems to be bullshit to even consider that as a special case. Why do they differentiate bridge/non-bridge case in the paper? Doesn't seme to make sense.

private:
    unordered_map<RuleIds, int> tot_case_coverage_cnt;
    unordered_map<RuleIds, int> tot_rule_checks_cnt;
    unordered_map<int,double> last_times_all;
    bool inputFlagToWeightedIsSet = false;
};