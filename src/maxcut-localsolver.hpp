//********* maxcut.cpp *********
#pragma once
#ifdef LOCALSOLVER_EXISTS


#include <iostream>
#include <fstream>
#include <vector>
#include "localsolver.h"
#include "../../src/utils.hpp"
#include "../../src/mc-graph.hpp"

using namespace localsolver;
using namespace std;


class LocalSolverCallback : public LSCallback {
public:
    LocalSolverCallback(double total_allowed_time_, InputParser *input_parser_, const string graph_name_, int mixingid_, int num_nodes_, int num_edges_, double added_preprocess_time_, int cutadd_, string sfxout_) :
                total_allowed_time(total_allowed_time_),
                input_parser(input_parser_),
                graph_name(graph_name_),
                mixingid(mixingid_),
                num_nodes(num_nodes_),
                num_edges(num_edges_),
                added_preprocess_time(added_preprocess_time_),
                cutadd(cutadd_),
                sfxout(sfxout_)
        {
            if (input_parser->cmdOptionExists("-exact-early-stop-v")) {
                int v_limit = stoi(input_parser->getCmdOption("-exact-early-stop-v"));
                if (v_limit < num_nodes)
                    added_preprocess_time = max(added_preprocess_time, total_allowed_time_ - 60*60);
            }
        }

    void callback(LocalSolver& ls, LSCallbackType /*type*/) {
        LSStatistics stats = ls.getStatistics();
        LSExpression obj = ls.getModel().getObjective(0);

        double runtime = stats.getRunningTime() + added_preprocess_time;
        OutputLiveMaxcut(*input_parser, graph_name, mixingid, num_nodes, num_edges, runtime, obj.getValue() + cutadd, sfxout);

        //cout << " TICK: " << runtime << " " << obj.getValue() + cutadd << endl;

        if (runtime * 1.01 > total_allowed_time) {
            timelimit_exceeded = true;
            ls.stop();
        }
    }

    bool HasExceededTimelimit() {
        return timelimit_exceeded;
    }

private:
    double total_allowed_time;
    InputParser *input_parser;
    string graph_name;
    int mixingid;
    int num_nodes;
    int num_edges;
    double added_preprocess_time;
    int cutadd;
    string sfxout;
    bool timelimit_exceeded = false;
};


class MaxcutLocalsolver {
public:
    // LocalSolver 
    LocalSolver localsolver;

    // Number of vertices 
    int n;

    // Number of edges
    int m;

    // Origin of each edge 
    vector<int> origin;

    // Destination of each edge 
    vector<int> dest;

    // Weight of each edge 
    vector<lsint> w;

    // True if vertex x[i] is on the right side of the cut, false if it is on the left side of the cut 
    vector<LSExpression> x;

    // Objective 
    LSExpression cutWeight;

    // Reads instance data. Takes 1-indexed vertices!
    void readInstance(const int num_nodes, const vector<tuple<int,int,int>>& edges){
        n = num_nodes;
        m = edges.size();

        origin.resize(m);
        dest.resize(m);
        w.resize(m);

        for (int i = 0; i < m; ++i) {
            origin[i] = get<0>(edges[i]);
            dest[i] = get<1>(edges[i]);
            w[i] = get<2>(edges[i]);
        }
    }

    void solve(int limit, LocalSolverCallback* callback = nullptr){
        // Declares the optimization model. 
        LSModel model = localsolver.getModel();

        // Decision variables x[i]
        x.resize(n);
        for(int i = 0; i < n; i++) {
            x[i] = model.boolVar();
        }
        
        // incut[e] is true if its endpoints are in different class of the partition
        vector<LSExpression> incut(m);
        for(int e = 0; e < m; e++){
            incut[e] = model.neq(x[origin[e] - 1], x[dest[e] - 1]);
        }
        
        // Size of the cut
        cutWeight = model.sum();
        for(int e = 0; e < m; e++){
            cutWeight += w[e]*incut[e];
        }

        model.maximize(cutWeight);
        model.close();

        if (callback != nullptr) localsolver.addCallback(CT_TimeTicked, callback);

        // Parameterizes the solver. 
        LSPhase phase = localsolver.createPhase();
        phase.setTimeLimit(limit);
        localsolver.getParam().setNbThreads(1);
      //  localsolver.getParam().setTimeBetweenTicks(2);
        localsolver.solve();
    }

    // Writes the solution in a file following the following format: 
    //  - objective value
    //  - each line contains a vertex number and its subset (1 for S, 0 for V-S)
    void writeSolution(const string& fileName){
        ofstream outfile;
        outfile.exceptions(ofstream::failbit | ofstream::badbit);
        outfile.open(fileName.c_str());

        outfile << cutWeight.getValue() << endl;
        // Note: in the instances the indices start at 1
        for (int i = 0; i < n; ++i)
            outfile << i+1 << " " << x[i].getValue() << endl;
    }

    int getCutSize() {
        return cutWeight.getValue();
    }

    vector<int> getCut() {
        // NOT IMPLEMENTED YET.
        vector<int> ret;
        return ret;
    }
};


/*
int main(int argc, char** argv) {
    if (argc < 1) {
        cerr << "Usage: maxcut inputFile [outputFile] [timeLimit] " << endl;
        return 1;
    }

    const char* instanceFile = argc > 1 ? argv[1] : NULL;
    const char* solFile = argc > 2 ? argv[2] : NULL;
    const char* strTimeLimit = argc > 3 ? argv[3] : "3";

    try {
        if (instanceFile != NULL) {
            MaxCutGraph G(instanceFile);
            cout << "Edwards Erdos bound: " << G.GetEdwardsErdosBound() << endl;

            Maxcut model;
            model.readInstance(instanceFile);
            model.solve(atoi(strTimeLimit));
            if(solFile != NULL) model.writeSolution(solFile);
        } else {
            cerr << "No input file" << endl;
        }

        return 0;
    } catch (const exception& e){
        cerr << "Error occurred: " << e.what() << endl;
        return 1;
    }
}*/

#endif