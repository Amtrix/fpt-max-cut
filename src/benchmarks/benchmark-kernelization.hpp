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
#include <mutex>
using namespace std;

class Benchmark_Kernelization : public BenchmarkAction {
public:
    const static bool kMakeWeightedAtEnd = false;

    Benchmark_Kernelization() {
    }



    bool KernelizeExec(MaxCutGraph &kernelized,
                      const vector<RuleIds>& provided_kernelization_order,
                      unordered_map<int, double>& times_all_components,
                      const bool reset_timestamps_each_time = false) {
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
                LogTime(times_all_components, t0, static_cast<int>(provided_kernelization_order[i]));
            }

            cout << ("|V(kernel)| = " + to_string(kernelized.GetRealNumNodes()) + "  |E(kernel)| = " + to_string(kernelized.GetRealNumEdges()) + " , cut_change = " + to_string(kernelized.GetInflictedCutChangeToKernelized())) << endl;

            if (!chg_happened) break; 
            else tot_chg_happened = true;
        }

        return tot_chg_happened;
    }



    bool use_signed_kernelization = true;
    bool use_weighted_kernelization = false;
    bool use_unweighted_kernelization = true;
    void Kernelize(MaxCutGraph &kernelized,
                   unordered_map<int, double>& times_all_components,
                   bool provide_order = false,
                   const vector<RuleIds>& provided_kernelization_order = {},
                   const bool force_timestampless_kernelization = false) {
        // First transform graph into unweighted. /////////////
        auto t0 = GetCurrentTime();
        //kernelized.MakeWeighted();

        auto selected_kernelization_order = kernelization_order;
        if (provide_order) selected_kernelization_order = provided_kernelization_order;

        bool total_finish = false;
        while (!total_finish) {
            total_finish = true;

            // Unweighted Reductions.
            if (use_unweighted_kernelization) { // default is true
                cout << "Perform unweighted kernelization." << endl;
                kernelized.MakeUnweighted();
                OutputDebugLog("Made Unweighted.");

                bool is_all_finished = false;
                while (!is_all_finished) {
                    is_all_finished = true;
                    LogTime(times_all_components, t0);
                    if (KernelizeExec(kernelized, selected_kernelization_order, times_all_components, false))
                        is_all_finished = false, total_finish = false;
                }
            }

            // Signed Reductions.
            if (use_signed_kernelization) { // default is false
                cout << "Perform signed kernelization." << endl;
                kernelized.MakeSigned();
                OutputDebugLog("Made Signed.");

                bool is_all_finished = false;
                while (!is_all_finished) {
                    is_all_finished = true;
                    if (KernelizeExec(kernelized, {RuleIds::Rule8Signed}, times_all_components, false))
                        is_all_finished = false, total_finish = false;
                }
            }

            // Weighted Reductions.
            if (use_weighted_kernelization) { // default is false
                cout << "Perform weighted kernelization." << endl;
                bool is_all_finished = false;
                kernelized.MakeWeighted();
                OutputDebugLog("Made Weighted.");

                while (!is_all_finished) {
                    is_all_finished = true;
                    LogTime(times_all_components, t0);
                    if (KernelizeExec(kernelized, {RuleIds::RuleS3Weighted, RuleIds::RuleS2Weighted, RuleIds::RuleWeightedTriag}, times_all_components, false))
                        is_all_finished = false, total_finish = false;
                }
            }
        }

        auto t_end_fast = std::chrono::high_resolution_clock::now();
        double time_fast_kernelization = std::chrono::duration_cast<std::chrono::microseconds> (t_end_fast - t0).count()/1000.;
        (void) time_fast_kernelization;
        OutputDebugLog("INITIAL -- FAST KERNELIZATION DONE! Time: " + to_string(time_fast_kernelization));

        // Section containing basically some checks and extended functionalities. Not relevant for correctness.
        {
            if (force_timestampless_kernelization) {
                KernelizeExec(kernelized, selected_kernelization_order, times_all_components, true);
            } else {
                custom_assert(KernelizeExec(kernelized, selected_kernelization_order, times_all_components, true) == false); // will only trigger if DEBUG defined, due to custom_assert definition.
            }
        }
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        // Finishers.
        if (use_unweighted_kernelization) { // default is true
            kernelized.MakeUnweighted();
            OutputDebugLog("Made Unweighted.");
            KernelizeExec(kernelized, finishing_rules_order, times_all_components, true);
        }
        ////////////////////////

        // GRAPH IS UNWEIGHTED AT THIS POINT IF UNWEIGHTED KERNELIZATION IS USED!
        // THAT IS WHY ONE NEEDS TO USE -force-weighted-result.
        t0 = GetCurrentTime();
        if (kMakeWeightedAtEnd || inputFlagToWeightedIsSet) {
            OutputDebugLog("Unweithed to weighted kernelization. |V| = " + to_string(kernelized.GetRealNumNodes()) + ", |E| = " + to_string(kernelized.GetRealNumEdges()));
            kernelized.MakeWeighted();
            OutputDebugLog("Unweithed to weighted kernelization: Done. |V| = " + to_string(kernelized.GetRealNumNodes()) + ", |E| = " + to_string(kernelized.GetRealNumEdges()));
        } else {
            OutputDebugLog("To weighted conversation is skipped.");
        }
        LogTime(times_all_components, t0);
        ////////////////////////////////////////
    }





    void Evaluate(InputParser &input, const MaxCutGraph &main_graph) {

        mtx_mixingid_gen.lock();
        {
            GenerateMissingMixingId(main_graph);
        }
        mtx_mixingid_gen.unlock();
        int mixingid = GetMixingId(main_graph);
        
        

        int num_iterations = 1;
        if (input.cmdOptionExists("-iterations")) {
            num_iterations = stoi(input.getCmdOption("-iterations"));
        }

        if (input.cmdOptionExists("-force-weighted-result")) {
            inputFlagToWeightedIsSet = true;
        } else {
            inputFlagToWeightedIsSet = false;
        }

        if (input.cmdOptionExists("-do-signed-reduction")) {
            use_signed_kernelization = main_graph.IsScaled() == false;
        } else {
            use_signed_kernelization = false;
        }

        if (input.cmdOptionExists("-do-weighted-reduction")) {
            use_weighted_kernelization = true;
        } else {
            use_weighted_kernelization = false;
        }

        if (input.cmdOptionExists("-dont-unweighted-reduction")) {
            use_unweighted_kernelization = false;
        } else {
            use_unweighted_kernelization = main_graph.IsScaled() == false;
        }

        vector<vector<double>> accum;
        for (int iteration = 1; iteration <= num_iterations; ++iteration) {
            MaxCutGraph G = main_graph;
            MaxCutGraph kernelized = G;

            
            unordered_map<int, double> times_all_components;
            auto t0_total = std::chrono::high_resolution_clock::now();
            Kernelize(kernelized, times_all_components);
            // Calculating spent time. From here on onwards, only O(1) operations allowed!!!!!!!!!!!!!!!!!!!!!
            auto t1_total = std::chrono::high_resolution_clock::now();
            double kernelization_time = std::chrono::duration_cast<std::chrono::microseconds> (t1_total - t0_total).count()/1000.;


            // Compute solver results.
            SolverEvaluation eval;
            eval.Evaluate(mixingid, input, kernelization_time, G, kernelized);
            

            // Some variables.
            double EE = G.GetEdwardsErdosBound();
            double EE_k = kernelized.GetEdwardsErdosBound();

            // Aggregating usages.
            mtx_aggregation.lock();
            {

                // Some output
                cout << "VERIFY CUT VAL:  localsearch(" << eval.local_search_cut_size << ", " << eval.local_search_cut_size_k
                    << ")   mqlib(" << eval.mqlib_cut_size << ", " << eval.mqlib_cut_size_k << ")" << endl;
                cout << "              G: " << G.GetRealNumNodes() << " " << G.GetRealNumEdges() << endl;
                cout << "     kernelized: " << kernelized.GetRealNumNodes() << " " << kernelized.GetRealNumEdges() << endl;

                auto case_coverage_cnt = kernelized.GetUsageVector();
                cout << "Spent time on kernelization[ms]: " << kernelization_time << endl;
                cout << "Case coverage (=number of applications) = ";
                for (unsigned int r = 0; r < case_coverage_cnt.size(); ++r)
                    cout << case_coverage_cnt[r] << " ";
                cout << endl;


            
                cout << setw(20) << "RULE" << setw(20) << "|USED|" << setw(20) << "|CHECKS|" << setw(20) << "|TIME|" << setw(20) << "|TIME|/|CHECKS|" << endl;
                for (auto rule : kAllRuleIds) {
                    double used_time = times_all_components[static_cast<int>(rule)];
                    int used_cnt = kernelized.GetRuleUsage(rule);
                    int check_cnt = kernelized.GetRuleChecks(rule);

                    tot_case_coverage_cnt[rule] += used_cnt;
                    tot_rule_checks_cnt[rule] += check_cnt;
                    total_times[static_cast<int>(rule)] += used_time;

                    
                    //- last_times_all[static_cast<int>(rule)];
                    cout << setw(20) << kRuleNames.at(rule) << setw(20) << used_cnt << setw(20) << check_cnt << setw(20) << used_time << setw(20) << (used_time/check_cnt) << endl;
                }
                total_times[-1] += times_all_components[-1];
                //last_times_all = times_all;

                double k_change = kernelized.GetInflictedCutChangeToKernelized();
                custom_assert(eval.biqmac_cut_size == eval.biqmac_cut_size_k || eval.biqmac_cut_size == -1 || eval.biqmac_cut_size_k == -1);
                
                OutputKernelization(input, main_graph.GetGraphNaming(),
                                    mixingid, iteration,
                                    G.GetRealNumNodes(), G.GetRealNumEdges(),
                                    kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(),
                                    -k_change,
                                    eval.mqlib_cut_size, eval.mqlib_cut_size_k, eval.mqlib_rate, eval.mqlib_rate_sddiff,
                                    eval.localsolver_cut_size, eval.localsolver_cut_size_k, eval.localsolver_rate, eval.localsolver_rate_sddiff,
                                    eval.local_search_cut_size, eval.local_search_cut_size_k, eval.local_search_rate, eval.local_search_rate_sddiff,

                                    eval.mqlib_time, eval.mqlib_time_k, 
                                    eval.localsolver_time, eval.localsolver_time_k, 
                                    eval.biqmac_time, eval.biqmac_time_k, 

                                    EE, EE_k, eval.MAXCUT_best_size, kernelization_time);
                
                accum.push_back({(double)mixingid, (double)iteration,
                                    (double)G.GetRealNumNodes(), (double)G.GetRealNumEdges(),
                                    (double)kernelized.GetRealNumNodes(), (double)kernelized.GetRealNumEdges(),
                                    -k_change,
                                    (double)eval.mqlib_cut_size, (double)eval.mqlib_cut_size_k, eval.mqlib_rate, eval.mqlib_rate_sddiff,
                                    eval.localsolver_cut_size, eval.localsolver_cut_size_k, eval.localsolver_rate, eval.localsolver_rate_sddiff,
                                    eval.local_search_cut_size, eval.local_search_cut_size_k, eval.local_search_rate, eval.local_search_rate_sddiff,
                                    eval.mqlib_time, eval.mqlib_time_k, 
                                    eval.localsolver_time, eval.localsolver_time_k, 
                                    eval.biqmac_time, eval.biqmac_time_k, 
                                    EE, EE_k, (double)eval.MAXCUT_best_size, kernelization_time});
                
                if (iteration == 1 && input.cmdOptionExists("-output-graphs-dir")) {
                    G.PrintGraph(input.getCmdOption("-output-graphs-dir") + to_string(mixingid), true);
                    kernelized.PrintGraph(input.getCmdOption("-output-graphs-dir") + to_string(mixingid) + "-kernelized", true);
                }
            }
            mtx_aggregation.unlock();
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

        OutputKernelization(input, main_graph.GetGraphNaming(), avg[0], avg[1], avg[2], avg[3], avg[4], avg[5], avg[6], avg[7], avg[8], avg[9], avg[10], avg[11], avg[12], avg[13], avg[14], avg[15],
            avg[16], avg[17], avg[18], avg[19], avg[20], avg[21], avg[22], avg[23], avg[24], avg[25], avg[26], avg[27], avg[28], "-avg");
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
            double used_time = total_times[static_cast<int>(rule)];
            tot_time += used_time;

            cout << setw(20) << kRuleNames.at(rule) << setw(20) << (used_cnt / (double) num_iterations)
                 << setw(20) << (check_cnt / (double) num_iterations) << setw(20) << (used_time / (double) num_iterations)
                 << setw(20) << (used_time/check_cnt) << endl; // this last value does not need to be divided by numm_iterations!!!
        }
        cout << "Time spent on other stuff: " << total_times[-1] << endl;
        cout << "TOTAL time (all iterations included): " << tot_time + total_times[-1] << endl;
        cout << endl;
        cout << endl;
    }

    const vector<RuleIds> kernelization_order = {
          RuleIds::RuleS2, RuleIds::Rule8, RuleIds::RuleS5, RuleIds::RuleS3
          
          /*
                    ON REMOVED RULES(!!!!):
                    RuleIds::RuleS2, RuleIds::Rule8, DUE TO INCLUSION IN "MEGA"
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
    std::mutex mtx_mixingid_gen;
    std::mutex mtx_aggregation;

    unordered_map<RuleIds, int> tot_case_coverage_cnt;
    unordered_map<RuleIds, int> tot_rule_checks_cnt;
    unordered_map<int,double> total_times;

    bool inputFlagToWeightedIsSet = false;
};