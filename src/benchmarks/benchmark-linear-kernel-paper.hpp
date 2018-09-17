#pragma once

#include "./benchmark-interface.hpp"
#include "../mc-graph.hpp"
#include "../one-way-reducers.hpp"
#include "../two-way-reducers.hpp"
#include "../input-parser.hpp"
#include "../utils.hpp"
#include "../output-filter.hpp"

#include <iostream>
using namespace std;

class Benchmark_LinearKernelPaper : public BenchmarkAction {
public:
    Benchmark_LinearKernelPaper() {
        tot_case_coverage_cnt = vector<int>(20, 0);
    }

    void Evaluate(InputParser& input, const string data_filepath /*, vector<int>& tot_used_rules*/) {
        int num_iterations = 1;
        if (input.cmdOptionExists("-iterations")) {
            num_iterations = stoi(input.getCmdOption("-num-iterations"));
        }

        string key = "";
        int dx = ((int)data_filepath.size()) - 1;
        while (data_filepath[dx] != '/' && data_filepath[dx] != '\\')
            key += data_filepath[dx--];
        key = key.substr(key.find('.'));
        
        if (mixingid[key] == 0) mixingid[key] = mixingid_giver++;


        vector<vector<double>> accum;
        for (int iteration = 1; iteration <= num_iterations; ++iteration) {
            MaxCutGraph G(data_filepath);
            MaxCutGraph kernelized = G;
            int change_tmp = 0;
            MaxCutGraph G_processing_oneway = G; // ! make sure no pointers in G !
            int rule_taken = -1;
            while ((rule_taken = TryOneWayReduce(G_processing_oneway, change_tmp)) != -1) {
                OutputDebugLog("RULE: " + to_string(rule_taken));
                OutputDebugLog("-----------");
            }

            auto marked = G_processing_oneway.GetMarkedVerticesByOneWayRules();

            G.SetMarkedVertices(G_processing_oneway.GetMarkedVerticesByOneWayRules());
            const int s_size_oneway = G.GetMarkedVerticesByOneWayRules().size();


            // Try reduce size of S
            G.ReduceMarksetVertexSet();
            const int s_size_oneway_with_reverse = G.GetMarkedVerticesByOneWayRules().size();


            const int s_size_adhoc = G.Algorithm3MarkedComputation_Randomized();
            OutputMarkedSetAnalysis(input, data_filepath, mixingid[key], iteration, G.GetRealNumNodes(), G.GetRealNumEdges(), s_size_oneway, s_size_oneway_with_reverse, s_size_adhoc);
        }

        test_id++;
    }

    void PostProcess(InputParser& /* input */) override {
        cout << "Total case coverage: " << endl;
        for (int i = 0; i < (int)tot_case_coverage_cnt.size(); ++i)
            cout << tot_case_coverage_cnt[i] << " ";
        cout << endl;
    }

private:
    int mixingid_giver = 1;
    map<string,int> mixingid;
    vector<int> tot_case_coverage_cnt;
    int test_id = 1;
};