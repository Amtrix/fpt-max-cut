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
                cout << ("         ... tried further " + to_string(cnt) + " times again.") << endl;
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
        kernelized.MakeSigned();
        KernelizeExec(kernelized, {RuleIds::Rule8Signed}, false);


        kernelized.MakeUnweighted();
        OutputDebugLog("Made unweighted");
        LogTime(t0);
        ////////////////////////////////////////

        // Reductions ////////////////////////////////////////
        auto selected_kernelization_order = kernelization_order;
        if (provide_order) selected_kernelization_order = provided_kernelization_order;

        
        KernelizeExec(kernelized, selected_kernelization_order, false);

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


            // Already needed for upcoming sections.
            double k_change = kernelized.GetInflictedCutChangeToKernelized();

            // Compute solver results.
            double local_search_cut_size = -1, local_search_cut_size_k = -1, local_search_rate = 0, local_search_rate_sddiff = 0;
            double mqlib_cut_size = -1, mqlib_cut_size_k = -1, mqlib_rate = 0, mqlib_rate_sddiff = 0;
            double localsolver_cut_size = -1, localsolver_cut_size_k = -1, localsolver_rate = 0, localsolver_rate_sddiff = 0;
            int local_search_cut_size_best = -1, mqlib_cut_size_best = -1, localsolver_cut_size_best = -1;

            vector<int> tmp_def_param_trash;
            std::tie(local_search_cut_size, local_search_cut_size_k, local_search_rate, local_search_rate_sddiff, local_search_cut_size_best)
                = ComputeAverageAndDeviation(TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeLocalSearchCut, &G, tmp_def_param_trash)),
                                             TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeLocalSearchCut, &kernelized, tmp_def_param_trash), -k_change),
                                             locsearch_iterations);

            
            int sub_on_kernelized_runtime = 0;
            sub_on_kernelized_runtime = round(kernelization_time / 1000.0);

            int total_time = -1;
            if (input.cmdOptionExists("-total-allowed-solver-time")) {
                total_time = stoi(input.getCmdOption("-total-allowed-solver-time"));
            } else {
                total_time = max(sub_on_kernelized_runtime * 5, 10);
            }

            if (total_time > sub_on_kernelized_runtime && fabs(k_change) > 1e-9) {
                OutputDebugLog("Allocated total runtime for solvers (+kernelization): " + to_string(total_time) + " of which kernelization has used: " + to_string(sub_on_kernelized_runtime) + " [seconds].");

                Burer2002Callback mqlib_cb  (total_time, &input, G.GetGraphNaming(), mixingid, G.GetRealNumNodes(), G.GetRealNumEdges(), 0, 0, "mqlib");
                Burer2002Callback mqlib_cb_k(total_time, &input, kernelized.GetGraphNaming(), mixingid, kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(), sub_on_kernelized_runtime, -k_change, "mqlib-kernelized");

                auto F_mqlib   = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithMQLib, &G, total_time, &mqlib_cb));
                auto F_mqlib_k = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithMQLib, &kernelized, total_time - sub_on_kernelized_runtime, &mqlib_cb_k), -k_change);

                vector<double> res_mqlib, res_mqlib_k;
                std::thread thread_mqlib ([&]{
                    for (int i = 0; i < mqlib_iterations; ++i) {
                        res_mqlib.push_back(F_mqlib());
                    }
                });
                std::thread thread_mqlib_k ([&]{
                    for (int i = 0; i < mqlib_iterations; ++i) {
                        res_mqlib_k.push_back(F_mqlib_k());
                    }
                });

                

#ifdef LOCALSOLVER_EXISTS
                LocalSolverCallback localsolver_cb  (total_time, &input, G.GetGraphNaming(), mixingid, G.GetRealNumNodes(), G.GetRealNumEdges(), 0, 0, "localsolver");
                LocalSolverCallback localsolver_cb_k(total_time, &input, kernelized.GetGraphNaming(), mixingid, kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(), sub_on_kernelized_runtime, -k_change, "localsolver-kernelized");

                auto F_localsolver   = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithLocalsolver, &G, total_time, &localsolver_cb));
                auto F_localsolver_k = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithLocalsolver, &kernelized, total_time - sub_on_kernelized_runtime, &localsolver_cb_k), -k_change);

                vector<double> res_localsolver, res_localsolver_k;
               // std::thread thread_localsolver ([&]{
                    for (int i = 0; i < localsolver_iterations; ++i) {
                        res_localsolver.push_back(F_localsolver());
                    }
               // });
               // std::thread thread_localsolver_k ([&]{
                    for (int i = 0; i < localsolver_iterations; ++i) {
                        res_localsolver_k.push_back(F_localsolver_k());
                    }
               // });

               // thread_localsolver.join(); thread_localsolver_k.join();

                std::tie(localsolver_cut_size, localsolver_cut_size_k, localsolver_rate, localsolver_rate_sddiff, localsolver_cut_size_best)
                    = ComputeAverageAndDeviation(res_localsolver, res_localsolver_k);
#endif

                thread_mqlib.join(); thread_mqlib_k.join();

                std::tie(mqlib_cut_size, mqlib_cut_size_k, mqlib_rate, mqlib_rate_sddiff, mqlib_cut_size_best)
                    = ComputeAverageAndDeviation(res_mqlib, res_mqlib_k);
            } else {
                cout << "Testing the solvers was skipped due to insufficient time or no kernelization done. Provided: " << total_time << "; spent on kernelization: " << sub_on_kernelized_runtime << " [seconds]." << endl;
                cout << "Kernelization: " << -k_change << endl;
            }

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

                int used_cnt = kernelized.GetRuleUsage(rule);
                int check_cnt = kernelized.GetRuleChecks(rule);
                double used_time = times_all[static_cast<int>(rule)] - last_times_all[static_cast<int>(rule)];
                cout << setw(20) << kRuleNames.at(rule) << setw(20) << used_cnt << setw(20) << check_cnt << setw(20) << used_time << setw(20) << (used_time/check_cnt) << endl;
            }
            last_times_all = times_all;


            OutputKernelization(input, main_graph.GetGraphNaming(),
                                mixingid, iteration,
                                G.GetRealNumNodes(), G.GetRealNumEdges(),
                                kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(),
                                -k_change,
                                mqlib_cut_size, mqlib_cut_size_k, mqlib_rate, mqlib_rate_sddiff,
                                localsolver_cut_size, localsolver_cut_size_k, localsolver_rate, localsolver_rate_sddiff,
                                local_search_cut_size, local_search_cut_size_k, local_search_rate, local_search_rate_sddiff,
                                EE, EE_k, MAXCUT_best_size, kernelization_time);
            
            accum.push_back({(double)mixingid, (double)iteration,
                                (double)G.GetRealNumNodes(), (double)G.GetRealNumEdges(),
                                (double)kernelized.GetRealNumNodes(), (double)kernelized.GetRealNumEdges(),
                                -k_change,
                                (double)mqlib_cut_size, (double)mqlib_cut_size_k, mqlib_rate, mqlib_rate_sddiff,
                                localsolver_cut_size, localsolver_cut_size_k, localsolver_rate, localsolver_rate_sddiff,
                                local_search_cut_size, local_search_cut_size_k, local_search_rate, local_search_rate_sddiff,
                                EE, EE_k, (double)MAXCUT_best_size, kernelization_time});
            
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

        OutputKernelization(input, main_graph.GetGraphNaming(), avg[0], avg[1], avg[2], avg[3], avg[4], avg[5], avg[6], avg[7], avg[8], avg[9], avg[10], avg[11], avg[12], avg[13], avg[14], avg[15], avg[16], avg[17], avg[18], avg[19], avg[20], avg[21], avg[22], "-avg");
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