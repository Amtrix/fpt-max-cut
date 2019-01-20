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

namespace SolverEvaluation {

enum Solvers {
    LocalSolver = 1,
    BiqMac = 2,
    Localsearch = 4,
    MqLib = 8,
    All = (1 << 20) - 1
};

double local_search_cut_size = -1, local_search_cut_size_k = -1, local_search_rate = 0, local_search_rate_sddiff = 0;
double mqlib_cut_size = -1, mqlib_cut_size_k = -1, mqlib_rate = 0, mqlib_rate_sddiff = 0;
double localsolver_cut_size = -1, localsolver_cut_size_k = -1, localsolver_rate = 0, localsolver_rate_sddiff = 0;
int local_search_cut_size_best = -1, mqlib_cut_size_best = -1, localsolver_cut_size_best = -1;

int biqmac_cut_size = -1, biqmac_cut_size_k = -1;
double biqmac_time = -1, biqmac_time_k = -1;
double localsolver_time = -1, localsolver_time_k = -1;
double mqlib_time = -1, mqlib_time_k = -1;

int MAXCUT_best_size;
int tmp_MAXCUT_best_size;


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

void Evaluate(const int mixingid, InputParser &input, int already_spent_time_on_kernelization_seconds, const MaxCutGraph& G, const MaxCutGraph& kernelized, int use_solver_mask = Solvers::All) {
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


    int total_time_seconds = -1;
    if (input.cmdOptionExists("-total-allowed-solver-time")) {
        total_time_seconds = stoi(input.getCmdOption("-total-allowed-solver-time"));
    } else {
        total_time_seconds = max(already_spent_time_on_kernelization_seconds * 5, 10);
    }

    OutputDebugLog("Allocated total runtime for solvers (+kernelization): " + to_string(total_time_seconds) + " of which kernelization has used: " + to_string(already_spent_time_on_kernelization_seconds) + " [seconds].");

    int locsearch_iterations = 1;
    if (input.cmdOptionExists("-locsearch-iterations")) {
        locsearch_iterations = stoi(input.getCmdOption("-locsearch-iterations"));
        cout << "Note: Local search iterations: " << locsearch_iterations << endl;
    }

    if (use_solver_mask & Solvers::Localsearch) {
        vector<int> tmp_def_param_trash;
        std::tie(local_search_cut_size, local_search_cut_size_k, local_search_rate, local_search_rate_sddiff, local_search_cut_size_best)
                    = ComputeAverageAndDeviation(TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeLocalSearchCut, &G, tmp_def_param_trash)),
                                                TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeLocalSearchCut, &kernelized, tmp_def_param_trash), -k_change),
                                                locsearch_iterations);
    }

    if (total_time_seconds > already_spent_time_on_kernelization_seconds && fabs(k_change) > 1e-9) {
    } else {
        cout << "Testing the solvers was skipped due to insufficient time or no kernelization done. Provided: " << total_time_seconds << "; spent on kernelization: " << already_spent_time_on_kernelization_seconds << " [seconds]." << endl;
        cout << "Kernelization: " << -k_change << endl;
        return;
    }


    Burer2002Callback mqlib_cb  (total_time_seconds, &input, G.GetGraphNaming(), mixingid, G.GetRealNumNodes(), G.GetRealNumEdges(), 0, 0, "mqlib");
    Burer2002Callback mqlib_cb_k(total_time_seconds, &input, kernelized.GetGraphNaming(), mixingid, kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(), already_spent_time_on_kernelization_seconds, -k_change, "mqlib-kernelized");
    auto F_mqlib   = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithMQLib, &G, total_time_seconds, &mqlib_cb));
    auto F_mqlib_k = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithMQLib, &kernelized, total_time_seconds - already_spent_time_on_kernelization_seconds, &mqlib_cb_k), -k_change);

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
        });
    }


    
    std::shared_ptr<std::thread> thread_biqmac, thread_biqmac_k;
#ifdef BIQMAC_EXISTS
    const string biqmac_binpath = BIQMAC_BINARY_PATH;
    const string project_build_dir = PROJECT_BUILD_DIR;
    if (!input.cmdOptionExists("-no-biqmac") && (use_solver_mask & Solvers::BiqMac)) { // EVALUATE BIQMAC
        OutputDebugLog("====> EVALUATE: BiqMac.");

        int v_limit = 1 << 30;
        if (input.cmdOptionExists("-exact-early-stop-v"))
            v_limit = stoi(input.getCmdOption("-exact-early-stop-v"));

        thread_biqmac = std::make_shared<std::thread>([&]{
            if (v_limit < G.GetRealNumNodes()) {
                biqmac_cut_size = -1;
                biqmac_time = -1;
                return;
            }

            G.PrintGraph("out-tmp-graph-for-biqmac", true);
            auto res = exec_custom(biqmac_binpath, project_build_dir + "/out-tmp-graph-for-biqmac", total_time_seconds);

            biqmac_cut_size = ParseBiqmacOutput_MxcCutSize(get<0>(res));
            biqmac_time = get<1>(res);
        });

        thread_biqmac_k = std::make_shared<std::thread>([&]{
            if (v_limit < kernelized.GetRealNumNodes()) {
                biqmac_cut_size_k = -1;
                biqmac_time_k = -1;
                return;
            }

            kernelized.PrintGraph("out-tmp-graph-for-biqmac-kernelized", true);
            auto res = exec_custom(biqmac_binpath, project_build_dir + "/out-tmp-graph-for-biqmac-kernelized", total_time_seconds - already_spent_time_on_kernelization_seconds);

            biqmac_cut_size_k = ParseBiqmacOutput_MxcCutSize(get<0>(res));
            if (biqmac_cut_size_k != -1) biqmac_cut_size_k += (int)(-k_change);
            biqmac_time_k = get<1>(res);
        }); 
    }
#endif
    

    std::shared_ptr<std::thread> thread_localsolver, thread_localsolver_k;
    vector<double> res_localsolver, res_localsolver_k;
#ifdef LOCALSOLVER_EXISTS
    // We cannot do multithreading here, as one license = one thread.
    LocalSolverCallback localsolver_cb  (total_time_seconds, &input, G.GetGraphNaming(), mixingid, G.GetRealNumNodes(), G.GetRealNumEdges(), 0, 0, "localsolver");
    LocalSolverCallback localsolver_cb_k(total_time_seconds, &input, kernelized.GetGraphNaming(), mixingid, kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(), already_spent_time_on_kernelization_seconds, -k_change, "localsolver-kernelized");
    auto F_localsolver   = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithLocalsolver, &G, total_time_seconds, &localsolver_cb));
    auto F_localsolver_k = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithLocalsolver, &kernelized, total_time_seconds - already_spent_time_on_kernelization_seconds, &localsolver_cb_k), -k_change);
        
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
        });

        
    }
#endif

    if (thread_biqmac && thread_biqmac_k) {
        thread_biqmac->join(); thread_biqmac_k->join();

        cout << "BIQMAC(G):  " << biqmac_cut_size << " " << biqmac_time << endl;
        cout << "BIQMAC(Gk): " << biqmac_cut_size_k << " " << biqmac_time_k << endl;
        
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
    }

    if (thread_localsolver && thread_localsolver_k) {
        std::tie(localsolver_cut_size, localsolver_cut_size_k, localsolver_rate, localsolver_rate_sddiff, localsolver_cut_size_best)
                = ComputeAverageAndDeviation(res_localsolver, res_localsolver_k);
            
        cout << "LOCALSOLVER(G):  " << localsolver_cut_size   << " " << localsolver_time << " (timelimit exceeded: " << localsolver_cb.HasExceededTimelimit() << ")" << endl;
        cout << "LOCALSOLVER(Gk): " << localsolver_cut_size_k << " " << localsolver_time_k << " (timelimit exceeded: " << localsolver_cb_k.HasExceededTimelimit() << ")" << endl;
    }

    MAXCUT_best_size = max(SolverEvaluation::local_search_cut_size_best, max(SolverEvaluation::mqlib_cut_size_best, SolverEvaluation::localsolver_cut_size_best));
    MAXCUT_best_size = max(MAXCUT_best_size, biqmac_cut_size);
    MAXCUT_best_size = max(MAXCUT_best_size, biqmac_cut_size_k);
    MAXCUT_best_size = max(MAXCUT_best_size, 0);

    if (input.cmdOptionExists("-exact-early-stop-v")) {
        if (MAXCUT_best_size != mqlib_cut_size   || mqlib_cb.HasExceededTimelimit())   mqlib_time   = -1;
        if (MAXCUT_best_size != mqlib_cut_size_k || mqlib_cb_k.HasExceededTimelimit()) mqlib_time_k = -1;
#ifdef LOCALSOLVER_EXISTS
        if (MAXCUT_best_size != localsolver_cut_size   || localsolver_cb.HasExceededTimelimit())   localsolver_time = -1;
        if (MAXCUT_best_size != localsolver_cut_size_k || localsolver_cb_k.HasExceededTimelimit()) localsolver_time_k = -1;
#endif
        if (MAXCUT_best_size != biqmac_cut_size)   biqmac_time   = -1;
        if (MAXCUT_best_size != biqmac_cut_size_k) biqmac_time_k = -1;
    }
}

} // SolverEvaluation