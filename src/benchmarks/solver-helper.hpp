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

enum Solvers {
    LocalSolver = 1,
    BiqMac = 2,
    Localsearch = 4,
    MqLib = 8,
    All = (1 << 20) - 1
};

/** EXAMPLE OUTPUT:

The input graph has 49 nodes and 126 edges, edge weights are integers.

Total number of branch-and-bound nodes: 1
Total time: 0.12 sec
cut value: 91.000000
one side of the cut:
 4 5 6 7 8 12 14 17 18 20 24 26 30 33 34 35 36 37 44 45 46 47
 1
Total time: 0.

 * */
double ParseBiqmacOutput_MxcCutSize(const string bm_output) {
    stringstream in(bm_output);
    string strline;
    const string subkey = "cut value: ";
    while (getline(in,strline)) {
        if (strline.substr(0, subkey.size()) == subkey) {
            return stod(strline.substr(subkey.size()));
        }
    }

    return -1;
}

struct SolverEvaluation {
    double local_search_cut_size = -1, local_search_cut_size_k = -1, local_search_rate = 0, local_search_rate_sddiff = 0;
    double mqlib_cut_size = -1, mqlib_cut_size_k = -1, mqlib_rate = 0, mqlib_rate_sddiff = 0;
    double localsolver_cut_size = -1, localsolver_cut_size_k = -1, localsolver_rate = 0, localsolver_rate_sddiff = 0;
    EdgeWeight local_search_cut_size_best = -1, mqlib_cut_size_best = -1, localsolver_cut_size_best = -1;

    EdgeWeight biqmac_cut_size = -1, biqmac_cut_size_k = -1;
    double biqmac_time = -1, biqmac_time_k = -1;
    double localsolver_time = -1, localsolver_time_k = -1;
    double mqlib_time = -1, mqlib_time_k = -1;

    EdgeWeight MAXCUT_best_size;
    EdgeWeight tmp_MAXCUT_best_size;

    void Evaluate(const int mixingid, InputParser &input, double already_spent_time_on_kernelization_seconds_ms, const MaxCutGraph& G, const MaxCutGraph& kernelized, int use_solver_mask = Solvers::All) {
        const double k_change = kernelized.GetInflictedCutChangeToKernelized();

        local_search_cut_size = -1, local_search_cut_size_k = -1, local_search_rate = 0, local_search_rate_sddiff = 0;
        mqlib_cut_size = -1, mqlib_cut_size_k = -1, mqlib_rate = 0, mqlib_rate_sddiff = 0;
        localsolver_cut_size = -1, localsolver_cut_size_k = -1, localsolver_rate = 0, localsolver_rate_sddiff = 0;
        local_search_cut_size_best = -1, mqlib_cut_size_best = -1, localsolver_cut_size_best = -1;

        biqmac_cut_size = -1, biqmac_cut_size_k = -1;
        
        biqmac_time = -1, biqmac_time_k = -1;
        localsolver_time = -1, localsolver_time_k = -1;
        mqlib_time = -1, mqlib_time_k = -1;

        MAXCUT_best_size = -1;
        tmp_MAXCUT_best_size = -1;

        int already_spent_time_on_kernelization_seconds_sec = already_spent_time_on_kernelization_seconds_ms / 1000.0;


        int total_time_seconds = -1;
        if (input.cmdOptionExists("-total-allowed-solver-time")) {
            total_time_seconds = stoi(input.getCmdOption("-total-allowed-solver-time"));
        } else {
            total_time_seconds = max(already_spent_time_on_kernelization_seconds_sec * 5, 10);
        }

        if (input.cmdOptionExists("-total-allowed-solver-time-range")) {
            total_time_seconds = stoi(input.getCmdOption("-total-allowed-solver-time-range"));
            total_time_seconds = rand() % total_time_seconds;
        }

        cout << ("Allocated total runtime for solvers (+kernelization): " + to_string(total_time_seconds) + " of which kernelization has used: " + to_string(already_spent_time_on_kernelization_seconds_sec) + " [seconds].") << endl;

        int locsearch_iterations = 1;
        if (input.cmdOptionExists("-locsearch-iterations")) {
            locsearch_iterations = stoi(input.getCmdOption("-locsearch-iterations"));
            cout << "Note: Local search iterations: " << locsearch_iterations << endl;
        }

        bool should_exit_G = false, should_exit_kernelized = false;
        if (ShouldExitEarly(&input, &G)) {
            cout << "Should exit early G: true." << endl;
            should_exit_G = true;
        }
        (void) should_exit_G; // surpress unusued variable
        (void) should_exit_kernelized; // surpress unusued variable

        if (ShouldExitEarly(&input, &kernelized)) {
            cout << "Should exit early kernelized: true." << endl;
            should_exit_kernelized = true;
        }

        if (use_solver_mask & Solvers::Localsearch) {
            vector<int> tmp_def_param_trash;
            std::tie(local_search_cut_size, local_search_cut_size_k, local_search_rate, local_search_rate_sddiff, local_search_cut_size_best)
                        = ComputeAverageAndDeviation(TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeLocalSearchCut, &G, tmp_def_param_trash)),
                                                    TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeLocalSearchCut, &kernelized, tmp_def_param_trash), -k_change),
                                                    locsearch_iterations);
        }

        if (total_time_seconds > already_spent_time_on_kernelization_seconds_sec && fabs(k_change) > 1e-9) {
        } else {
            cout << "Testing the solvers was skipped due to insufficient time or no kernelization done. Provided: " << total_time_seconds << "; spent on kernelization: " << already_spent_time_on_kernelization_seconds_sec << " [seconds]." << endl;
            cout << "Kernelization: " << -k_change << endl;
            cout << "Not skipped actually due to new change." << endl;
            //return;
        }

        Burer2002Callback mqlib_cb  (total_time_seconds, &input, G.GetGraphNaming(), mixingid, G.GetRealNumNodes(), G.GetRealNumEdges(), 0, 0, "mqlib");
        Burer2002Callback mqlib_cb_k(total_time_seconds, &input, kernelized.GetGraphNaming(), mixingid, kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(), already_spent_time_on_kernelization_seconds_sec, -k_change, "mqlib-kernelized");
        auto F_mqlib   = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithMQLib, &G, total_time_seconds, &mqlib_cb));
        auto F_mqlib_k = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithMQLib, &kernelized, total_time_seconds - already_spent_time_on_kernelization_seconds_sec, &mqlib_cb_k), -k_change);

        std::shared_ptr<std::thread> thread_mqlib, thread_mqlib_k;
        vector<double> res_mqlib, res_mqlib_k;

        if (!input.cmdOptionExists("-no-mqlib") && (use_solver_mask & Solvers::MqLib)) { // EVALUATE MQLIB
            OutputDebugLog("====> EVALUATE: MQLIB.");
        
            thread_mqlib = std::make_shared<std::thread>([&]{
                auto t0_total = std::chrono::high_resolution_clock::now();
                res_mqlib.push_back(F_mqlib());
                auto t1_total = std::chrono::high_resolution_clock::now();
                mqlib_time   = std::chrono::duration_cast<std::chrono::microseconds> (t1_total - t0_total).count()/1000.;
            });
            thread_mqlib_k = std::make_shared<std::thread>([&]{
                auto t0_total = std::chrono::high_resolution_clock::now();
                res_mqlib_k.push_back(F_mqlib_k());
                auto t1_total = std::chrono::high_resolution_clock::now();
                mqlib_time_k  = std::chrono::duration_cast<std::chrono::microseconds> (t1_total - t0_total).count()/1000.;
                if (mqlib_time_k >= 0) mqlib_time_k += already_spent_time_on_kernelization_seconds_ms;
            });
        }


        
        std::shared_ptr<std::thread> thread_biqmac, thread_biqmac_k;
#ifdef BIQMAC_EXISTS
        const string biqmac_binpath = BIQMAC_BINARY_PATH;
        const string project_build_dir = PROJECT_BUILD_DIR;
        if (!input.cmdOptionExists("-no-biqmac") && (use_solver_mask & Solvers::BiqMac)) { // EVALUATE BIQMAC
            OutputDebugLog("====> EVALUATE: BiqMac.");
            const long long scale = SCALED_FROM != -1 ? (SCALED_FROM) : 1;


            thread_biqmac = std::make_shared<std::thread>([&]{
                if (should_exit_G) {
                    biqmac_cut_size = -1;
                    biqmac_time = -1;
                    return;
                }

                std::ostringstream threadid;
                threadid << std::this_thread::get_id();
                const string filename = "out-tmp-graph-for-biqmac-t" + string(threadid.str());
                G.PrintGraph(filename, true, scale);
                auto res = exec_custom(biqmac_binpath, project_build_dir + "/" + filename, total_time_seconds);

                double dbl_biqmac_cut_size = ParseBiqmacOutput_MxcCutSize(get<0>(res));
                if (G.IsScaled() && dbl_biqmac_cut_size >= 0) dbl_biqmac_cut_size *= scale;
                biqmac_cut_size = (EdgeWeight) dbl_biqmac_cut_size;
                biqmac_time = get<1>(res);
            });

            thread_biqmac_k = std::make_shared<std::thread>([&]{
                if (should_exit_kernelized) {
                    biqmac_cut_size_k = -1;
                    biqmac_time_k = -1;
                    return;
                }

                std::ostringstream threadid;
                threadid << std::this_thread::get_id();
                const string filename = "out-tmp-graph-for-biqmac-kernelized-t" + string(threadid.str());
                kernelized.PrintGraph(filename, true, scale);
                auto res = exec_custom(biqmac_binpath, project_build_dir + "/" + filename, total_time_seconds - already_spent_time_on_kernelization_seconds_sec);

                double dbl_biqmac_cut_size_k = ParseBiqmacOutput_MxcCutSize(get<0>(res));
                if (kernelized.IsScaled() && dbl_biqmac_cut_size_k >= 0) dbl_biqmac_cut_size_k *= scale;
                if (dbl_biqmac_cut_size_k >= 0) dbl_biqmac_cut_size_k += (EdgeWeight)(-k_change);
                biqmac_cut_size_k = (EdgeWeight) dbl_biqmac_cut_size_k;
                biqmac_time_k  = get<1>(res);
                if (biqmac_time_k >= 0) biqmac_time_k += already_spent_time_on_kernelization_seconds_ms;
            }); 
        }
#endif
        

        std::shared_ptr<std::thread> thread_localsolver, thread_localsolver_k;
        vector<double> res_localsolver, res_localsolver_k;
#ifdef LOCALSOLVER_EXISTS
        // We cannot do multithreading here, as one license = one thread.
        LocalSolverCallback localsolver_cb  (total_time_seconds, &input, G.GetGraphNaming(), mixingid, G.GetRealNumNodes(), G.GetRealNumEdges(), 0, 0, "localsolver", G.IsScaled());
        LocalSolverCallback localsolver_cb_k(total_time_seconds, &input, kernelized.GetGraphNaming(), mixingid, kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(), already_spent_time_on_kernelization_seconds_sec, -k_change, "localsolver-kernelized", kernelized.IsScaled());
        auto F_localsolver   = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithLocalsolver, &G, total_time_seconds, &localsolver_cb));
        auto F_localsolver_k = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithLocalsolver, &kernelized, total_time_seconds - already_spent_time_on_kernelization_seconds_sec, &localsolver_cb_k), -k_change);
            
        if (!input.cmdOptionExists("-no-localsolver") && (use_solver_mask & Solvers::LocalSolver)) { // EVALUATE LOCALSOLVER
            OutputDebugLog("====> EVALUATE: LocalSolver.");

            thread_localsolver = std::make_shared<std::thread>([&]{
                auto t0_total = std::chrono::high_resolution_clock::now();
                res_localsolver.push_back(F_localsolver());
                auto t1_total = std::chrono::high_resolution_clock::now();
                localsolver_time   = std::chrono::duration_cast<std::chrono::microseconds> (t1_total - t0_total).count()/1000.;
            });

#ifndef LOCALSOLVER_USE_CONCURRENCY
            thread_localsolver->join();
#endif
            thread_localsolver_k = std::make_shared<std::thread>([&]{
                auto t0_total = std::chrono::high_resolution_clock::now();
                res_localsolver_k.push_back(F_localsolver_k());
                auto t1_total = std::chrono::high_resolution_clock::now();
                localsolver_time_k   = std::chrono::duration_cast<std::chrono::microseconds> (t1_total - t0_total).count()/1000.;
                if (localsolver_time_k >= 0) localsolver_time_k  += already_spent_time_on_kernelization_seconds_ms;
            });

            
        }
#endif

        if (thread_biqmac && thread_biqmac_k) {
            thread_biqmac->join(); thread_biqmac_k->join();

            // No timelimit exceeded but also no cut? => CRASH happened
            if (biqmac_cut_size < 0 && biqmac_time >= 0) biqmac_time = -2; // ERROR
            if (biqmac_cut_size_k < 0 && biqmac_time_k >= 0) biqmac_time_k = -2; // ERROR

            cout << "BIQMAC(G):  " << biqmac_cut_size << " " << biqmac_time << endl;
            cout << "BIQMAC(Gk): " << biqmac_cut_size_k << " " << biqmac_time_k << endl;
            cout << "BIQMAC(Gk-no_k_change): " << biqmac_cut_size_k + k_change << " " << biqmac_time_k << endl;
            
            tmp_MAXCUT_best_size = max(SolverEvaluation::local_search_cut_size_best, SolverEvaluation::localsolver_cut_size_best);
            tmp_MAXCUT_best_size = max(tmp_MAXCUT_best_size, biqmac_cut_size);
            tmp_MAXCUT_best_size = max(tmp_MAXCUT_best_size, biqmac_cut_size_k);

            mqlib_cb.SetTerminatingCutSize(tmp_MAXCUT_best_size);
            mqlib_cb_k.SetTerminatingCutSize(tmp_MAXCUT_best_size);
        }

        for (auto entry : {thread_mqlib, thread_mqlib_k, thread_localsolver, thread_localsolver_k})
            if (entry && entry->joinable())
                entry->join();
                
        if (thread_mqlib && thread_mqlib_k) {
            std::tie(mqlib_cut_size, mqlib_cut_size_k, mqlib_rate, mqlib_rate_sddiff, mqlib_cut_size_best)
                = ComputeAverageAndDeviation(res_mqlib, res_mqlib_k);
            
            cout << "MQLIB(G):  " << mqlib_cut_size   << " " << mqlib_time << " (timelimit exceeded: " << mqlib_cb.HasExceededTimelimit() << ")" << endl;
            cout << "MQLIB(Gk): " << mqlib_cut_size_k << " " << mqlib_time_k << " (timelimit exceeded: " << mqlib_cb_k.HasExceededTimelimit() << ")" << endl;
            cout << "MQLIB(Gk-no_k_change): " << mqlib_cut_size_k + k_change << " " << mqlib_time_k << " (timelimit exceeded: " << mqlib_cb_k.HasExceededTimelimit() << ")" << endl;

            if (input.cmdOptionExists("-no-biqmac")) {
                biqmac_time = biqmac_time_k = total_time_seconds;
            }
        }

        if (thread_localsolver && thread_localsolver_k) {
            std::tie(localsolver_cut_size, localsolver_cut_size_k, localsolver_rate, localsolver_rate_sddiff, localsolver_cut_size_best)
                    = ComputeAverageAndDeviation(res_localsolver, res_localsolver_k);

    #ifdef LOCALSOLVER_EXISTS
            cout << "LOCALSOLVER(G):  " << localsolver_cut_size   << " " << localsolver_time << " (timelimit exceeded: " << localsolver_cb.HasExceededTimelimit() << ")" << endl;
            cout << "LOCALSOLVER(Gk): " << localsolver_cut_size_k << " " << localsolver_time_k << " (timelimit exceeded: " << localsolver_cb_k.HasExceededTimelimit() << ")" << endl;
            cout << "LOCALSOLVER(Gk-no_k_change): " << localsolver_cut_size_k + k_change << " " << localsolver_time_k << " (timelimit exceeded: " << localsolver_cb_k.HasExceededTimelimit() << ")" << endl;
    #endif
        }

        MAXCUT_best_size = max(SolverEvaluation::local_search_cut_size_best, max(SolverEvaluation::mqlib_cut_size_best, SolverEvaluation::localsolver_cut_size_best));
        MAXCUT_best_size = max(MAXCUT_best_size, biqmac_cut_size);
        MAXCUT_best_size = max(MAXCUT_best_size, biqmac_cut_size_k);
        MAXCUT_best_size = max(MAXCUT_best_size, 0LL);

        cout << "MAXCUT_best_size = " << MAXCUT_best_size << endl;

        //if (MAXCUT_best_size != mqlib_cut_size   || mqlib_cb.HasExceededTimelimit())   mqlib_time   = -1;
        //if (MAXCUT_best_size != mqlib_cut_size_k || mqlib_cb_k.HasExceededTimelimit()) mqlib_time_k = -1;
#ifdef LOCALSOLVER_EXISTS
        if (localsolver_time   > total_time_seconds * 1000 || localsolver_cb.HasExceededTimelimit())   localsolver_time = -1;
        if (localsolver_time_k > total_time_seconds * 1000 || localsolver_cb_k.HasExceededTimelimit()) localsolver_time_k = -1;
#endif
        //if (MAXCUT_best_size != biqmac_cut_size)   biqmac_time   = -1;
        //if (MAXCUT_best_size != biqmac_cut_size_k) biqmac_time_k = -1;
    }
};