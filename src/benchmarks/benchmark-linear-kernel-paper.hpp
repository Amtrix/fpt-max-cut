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

class Benchmark_LinearKernelPaper : public BenchmarkAction {
public:
    Benchmark_LinearKernelPaper() {
        tot_used_rules = vector<int>(20, 0);
        tot_used_twoway_rules = vector<int>(20, 0);
    }

    void Evaluate(InputParser& input, const MaxCutGraph& main_graph) {
        BenchmarkAction::Evaluate(input, main_graph);
        string data_filepath_key = BenchmarkAction::GetKey(main_graph.GetGraphNaming());

        int num_iterations = 1;
        if (input.cmdOptionExists("-iterations")) {
            num_iterations = stoi(input.getCmdOption("-num-iterations"));
        }

        vector<vector<double>> accum;
        for (int iteration = 1; iteration <= num_iterations; ++iteration) {
            vector<int> curr_tot_used_rules(20, 0), curr_tot_used_twoway_rules(20, 0);

            MaxCutGraph G = main_graph;
            MaxCutGraph kernelized = G;
            int change_tmp = 0;
            MaxCutGraph G_processing_oneway = G; // ! make sure no pointers in G !
            int rule_taken = -1;
            auto t0 = std::chrono::high_resolution_clock::now();
            while ((rule_taken = TryOneWayReduce(G_processing_oneway, change_tmp)) != -1) {
                OutputDebugLog("RULE: " + to_string(rule_taken));
                OutputDebugLog("-----------");
                tot_used_rules[rule_taken]++;
                curr_tot_used_rules[rule_taken]++;
            }
            auto t1 = std::chrono::high_resolution_clock::now();
            double oneway_time = std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000.;

            auto marked = G_processing_oneway.GetMarkedVerticesByOneWayRules();

            auto marked_vertex_set = G_processing_oneway.GetMarkedVerticesByOneWayRules();
            G.SetMarkedVertices(marked_vertex_set);
            const int s_size_oneway = G.GetMarkedVerticesByOneWayRules().size();

            // Try reduce size of S
            string perform_reduce = "yes";
            if (input.cmdOptionExists("-do-reduce"))
                perform_reduce = input.getCmdOption("-do-reduce");

            int s_size_oneway_with_reverse = -1;
            double oneway_reduc_time = -1;
            int s_size_adhoc = -1;
            if (perform_reduce == "yes") {
                G.ReduceMarksetVertexSet();
                marked_vertex_set = G_processing_oneway.GetMarkedVerticesByOneWayRules();
                s_size_oneway_with_reverse = marked_vertex_set.size();
                auto t2 = std::chrono::high_resolution_clock::now();
                oneway_reduc_time = std::chrono::duration_cast<std::chrono::microseconds> (t2 - t0).count()/1000.;
                s_size_adhoc = G.Algorithm3MarkedComputation_Randomized();
            }

            MaxCutGraph G_processing_twoway = G;
            auto tk0 = std::chrono::high_resolution_clock::now();
            while ((rule_taken = ExhaustiveTwoWayReduce(G_processing_twoway, marked_vertex_set)) != -1) {
                OutputDebugLog("2-way-RULE: " + to_string(rule_taken));
                OutputDebugLog("-----------");
                tot_used_twoway_rules[rule_taken]++;
                curr_tot_used_twoway_rules[rule_taken]++;
            }
            auto tk1 = std::chrono::high_resolution_clock::now();
            double twoway_time = std::chrono::duration_cast<std::chrono::microseconds> (tk1 - tk0).count()/1000.;

            cout << "One-way: ";
            for (int i = 0; i < 20; ++i) cout << curr_tot_used_rules[i] << " ";
            cout << endl;
            cout << "Two-way: ";
            for (int i = 0; i < 20; ++i) cout << curr_tot_used_twoway_rules[i] << " ";
            cout << endl;


            OutputLinearKernelAnalysis(input, G.GetGraphNaming(), BenchmarkAction::GetMixingId(G), iteration,
                G.GetRealNumNodes(), G.GetRealNumEdges(), G_processing_twoway.GetRealNumNodes(), G_processing_twoway.GetRealNumEdges(),
                s_size_oneway, s_size_oneway_with_reverse, s_size_adhoc, oneway_time, twoway_time, oneway_reduc_time);
        }

        

        test_id++;
    }

    void PostProcess(InputParser& input) override {
        std::stringstream buffer;
        buffer << "Total one way rules coverage: " << endl;
        for (int i = 0; i < (int)tot_used_rules.size(); ++i)
            buffer << tot_used_rules[i] << " ";
        buffer << endl;

        buffer << "Two way: " << endl;
        for (int i = 0; i < (int)tot_used_twoway_rules.size(); ++i)
            buffer << tot_used_twoway_rules[i] << " ";
        buffer << endl;

        cout << buffer.str();
        OutputMarkedSetAnalysisMeta(input, string(buffer.str()));
    }

private:
    vector<int> tot_used_rules, tot_used_twoway_rules;
    int test_id = 1;
};