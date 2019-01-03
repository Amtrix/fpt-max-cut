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

void Evaluate(const int mixingid, InputParser &input, int already_spent_time_on_kernelization_seconds, const MaxCutGraph& G, const MaxCutGraph& kernelized, int use_solver_mask = Solvers::All) {
    double k_change = kernelized.GetInflictedCutChangeToKernelized();

    local_search_cut_size = -1, local_search_cut_size_k = -1, local_search_rate = 0, local_search_rate_sddiff = 0;
    mqlib_cut_size = -1, mqlib_cut_size_k = -1, mqlib_rate = 0, mqlib_rate_sddiff = 0;
    localsolver_cut_size = -1, localsolver_cut_size_k = -1, localsolver_rate = 0, localsolver_rate_sddiff = 0;
    local_search_cut_size_best = -1, mqlib_cut_size_best = -1, localsolver_cut_size_best = -1;


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

    std::unique_ptr<std::thread> thread_mqlib, thread_mqlib_k;
    vector<double> res_mqlib, res_mqlib_k;
    if (use_solver_mask & Solvers::MqLib) {
        Burer2002Callback mqlib_cb  (total_time_seconds, &input, G.GetGraphNaming(), mixingid, G.GetRealNumNodes(), G.GetRealNumEdges(), 0, 0, "mqlib");
        Burer2002Callback mqlib_cb_k(total_time_seconds, &input, kernelized.GetGraphNaming(), mixingid, kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(), already_spent_time_on_kernelization_seconds, -k_change, "mqlib-kernelized");

        auto F_mqlib   = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithMQLib, &G, total_time_seconds, &mqlib_cb));
        auto F_mqlib_k = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithMQLib, &kernelized, total_time_seconds - already_spent_time_on_kernelization_seconds, &mqlib_cb_k), -k_change);

        
        thread_mqlib = std::unique_ptr<std::thread>(new std::thread([&]{
            for (int i = 0; i < mqlib_iterations; ++i) {
                res_mqlib.push_back(F_mqlib());
            }
        }));
        thread_mqlib_k = std::unique_ptr<std::thread>(new std::thread([&]{
            for (int i = 0; i < mqlib_iterations; ++i) {
                res_mqlib_k.push_back(F_mqlib_k());
            }
        }));
    }

#ifdef BIQMACSOLVER_EXISTS
    if (use_solver_mask & Solvers::BiqMac) {
        // call localsolver here.
    }
#endif
    

#ifdef LOCALSOLVER_EXISTS
    if (use_solver_mask & Solvers::LocalSolver) {
        LocalSolverCallback localsolver_cb  (total_time_seconds, &input, G.GetGraphNaming(), mixingid, G.GetRealNumNodes(), G.GetRealNumEdges(), 0, 0, "localsolver");
        LocalSolverCallback localsolver_cb_k(total_time_seconds, &input, kernelized.GetGraphNaming(), mixingid, kernelized.GetRealNumNodes(), kernelized.GetRealNumEdges(), already_spent_time_on_kernelization_seconds, -k_change, "localsolver-kernelized");

        auto F_localsolver   = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithLocalsolver, &G, total_time_seconds, &localsolver_cb));
        auto F_localsolver_k = TakeFirstFromPairFunction(std::bind(&MaxCutGraph::ComputeMaxCutWithLocalsolver, &kernelized, total_time_seconds - already_spent_time_on_kernelization_seconds, &localsolver_cb_k), -k_change);

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
    }
#endif

    if (thread_mqlib && thread_mqlib_k) {
        thread_mqlib->join(); thread_mqlib_k->join();

        std::tie(mqlib_cut_size, mqlib_cut_size_k, mqlib_rate, mqlib_rate_sddiff, mqlib_cut_size_best)
            = ComputeAverageAndDeviation(res_mqlib, res_mqlib_k);
    }
}

} // SolverEvaluation