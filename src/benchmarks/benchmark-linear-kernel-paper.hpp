#pragma once

#include "./benchmark-interface.hpp"
#include "../mc-graph.hpp"
#include "../one-way-reducers.hpp"
#include "../two-way-reducers.hpp"
#include "../input-parser.hpp"
#include "../utils.hpp"
#include "../output-filter.hpp"

#include <iostream>
#include <chrono>
#include <sstream>
using namespace std;


const map<int, string> kLinearKernelRuleNames = {
    {3, "OneWay3"},
    {5, "OneWay5"},
    {6, "OneWay6"},
    {7, "OneWay7"},
    {8, "TwoWay8"},
    {9, "TwoWay9"}
};

const vector<int> kAllLinearKernelRuleIds = {
    5, 3, 7, 6,
    8, 9
};

class Benchmark_LinearKernelPaper : public BenchmarkAction {
public:
    Benchmark_LinearKernelPaper() {
        tot_used_rules = vector<int>(20, 0);
    }

    void Evaluate(InputParser& input, const MaxCutGraph& main_graph) {
        GenerateMissingMixingId(main_graph);

        int num_iterations = 1;
        if (input.cmdOptionExists("-iterations")) {
            num_iterations = stoi(input.getCmdOption("-iterations"));
        }

        vector<vector<double>> accum;
        for (int iteration = 1; iteration <= num_iterations; ++iteration) {
            vector<int> curr_tot_used_rules(20, 0);

            MaxCutGraph G = main_graph;
            MaxCutGraph kernelized = G;
            int change_tmp = 0;
            MaxCutGraph G_processing_oneway = G; // ! make sure no pointers in G !
            int rule_taken = -1;


            ///////////// One-Way rules
            auto t0 = std::chrono::high_resolution_clock::now();
            vector<pair<double,int>> times_within_call;
            while ((rule_taken = TryOneWayReduce(G_processing_oneway, change_tmp, times_within_call)) != -1) {
                OutputDebugLog("RULE: " + to_string(rule_taken));
                OutputDebugLog("-----------");
                tot_used_rules[rule_taken]++;
                curr_tot_used_rules[rule_taken]++;

                for (auto time_entry : times_within_call)
                    LogTimeEx(time_entry.first, time_entry.second);
                times_within_call.clear();
            }

            auto t1 = std::chrono::high_resolution_clock::now();
            double oneway_time = std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000.;
            //t0 = t1; we want total time if reduce done too!

            auto marked_vertex_set = G_processing_oneway.GetMarkedVerticesByOneWayRules();
            G.SetMarkedVertices(marked_vertex_set);
            const int s_size_oneway = marked_vertex_set.size();
            ///////////////////////////


            ///////////// Try reduce size of S
            string perform_reduce = "no";
            if (input.cmdOptionExists("-do-reduce"))
                perform_reduce = input.getCmdOption("-do-reduce");

            int s_size_oneway_with_reverse = -1;
            double oneway_reduc_time = -1;
            int s_size_adhoc = -1;
            double adhoc_time = -1;
            if (perform_reduce == "yes") {
                marked_vertex_set = G.Algorithm3MarkedComputation(marked_vertex_set);
                G.UpdateMarkedVertices(marked_vertex_set);
                s_size_oneway_with_reverse = marked_vertex_set.size();
                auto t2 = std::chrono::high_resolution_clock::now();
                oneway_reduc_time = std::chrono::duration_cast<std::chrono::microseconds> (t2 - t0).count()/1000.;
                t0 = t2;


                marked_vertex_set = G.Algorithm3MarkedComputation();
                G.UpdateMarkedVertices(marked_vertex_set);
                s_size_adhoc = marked_vertex_set.size();
                auto t3 = std::chrono::high_resolution_clock::now();
                adhoc_time = std::chrono::duration_cast<std::chrono::microseconds> (t3 - t0).count()/1000.;
                t0 = t3;
            }
            ///////////////////////////



            ///////////// Two-Way rules
            assert(times_within_call.empty());
            MaxCutGraph G_processing_twoway = G;
            auto tk0 = std::chrono::high_resolution_clock::now();
            while ((rule_taken = ExhaustiveTwoWayReduce(G_processing_twoway, marked_vertex_set, times_within_call)) != -1) {
                OutputDebugLog("2-way-RULE: " + to_string(rule_taken));
                OutputDebugLog("-----------");
                tot_used_rules[rule_taken]++;
                curr_tot_used_rules[rule_taken]++;

                for (auto time_entry : times_within_call)
                    LogTimeEx(time_entry.first, time_entry.second);
                times_within_call.clear();
            }
            auto tk1 = std::chrono::high_resolution_clock::now();
            double twoway_time = std::chrono::duration_cast<std::chrono::microseconds> (tk1 - tk0).count()/1000.;
            ///////////////////////////

            int mcpre = -1, mcpost = -1;
            double mcpre_time = -1, mcpost_time = -1;
            
            if (input.cmdOptionExists("-do-mc-extension-algo")) {
                std::thread t1([&]{
                    tie(mcpre, mcpre_time)   = G.GetMaxCutWithMarkedVertexSet(29, 60*180); // 3 hours
                });
                
                std::thread t2([&]{
                    tie(mcpost, mcpost_time) = G_processing_twoway.GetMaxCutWithMarkedVertexSet(29, 60*180); // 3 hours
                });
                
                t1.join();
                t2.join();
            }


            cout << "CUTS: " << mcpre << "(" << mcpre_time << ")  " << mcpost << "(" << mcpost_time << ")  " << endl;
            cout << "All rules usage count: ";
            for (int i = 0; i < 20; ++i) cout << curr_tot_used_rules[i] << " ";
            cout << endl;


            OutputLinearKernelAnalysis(input, G.GetGraphNaming(), BenchmarkAction::GetMixingId(G), iteration,
                G.GetRealNumNodes(), G.GetRealNumEdges(), G_processing_twoway.GetRealNumNodes(), G_processing_twoway.GetRealNumEdges(),
                s_size_oneway, s_size_oneway_with_reverse, s_size_adhoc, oneway_time / 1000.0, twoway_time / 1000.0, oneway_reduc_time / 1000.0, adhoc_time / 1000.0,
                mcpre, mcpre_time / 1000.0, mcpost, mcpost_time / 1000.0);
        }

        

        test_id++;
    }

    void PostProcess(InputParser& input) override {
        int num_iterations = 1;
        if (input.cmdOptionExists("-iterations")) {
            num_iterations = stoi(input.getCmdOption("-iterations"));
        }

        cout << "Total case coverage: (time in milliseconds, all values divided by number of iterations[" << num_iterations << "])" << endl; // ordered according kAllLinearKernelRuleIds
        cout << setw(20) << "RULE" << setw(20) << "|CNT|" << setw(20) << "|TIME|" << setw(20) << "|TIME|/|CNT|" << endl;

        double tot_time = 0;
        for (auto rule : kAllLinearKernelRuleIds) {
            int used_cnt = tot_used_rules[rule];
            double used_time = times_all[rule];
            tot_time += used_time;
            cout << setw(20) << kLinearKernelRuleNames.at(rule) << setw(20) << (used_cnt / (double) num_iterations)
                 << setw(20) << (used_time / (double) num_iterations) << setw(20) << (used_time/used_cnt) << endl; // this last value does not need to be divided by numm_iterations!!!
        }
        cout << "Time spent on other stuff: " << times_all[-1] << endl;
        cout << "TOTAL time (all iterations included): " << tot_time + times_all[-1] << endl;
        cout << endl;
        cout << endl;

        std::stringstream buffer;
        buffer << "Total rules coverage: " << endl;
        for (int i = 0; i < (int)tot_used_rules.size(); ++i)
            buffer << tot_used_rules[i] << " ";
        buffer << endl;

        cout << buffer.str();
        OutputMarkedSetAnalysisMeta(input, string(buffer.str()));
    }

private:
    vector<int> tot_used_rules;
    int test_id = 1;
};