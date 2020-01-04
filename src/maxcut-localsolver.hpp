
#pragma once
#ifdef LOCALSOLVER_EXISTS


#include <iostream>
#include <fstream>
#include <vector>
#include "localsolver.h"
#include "./utils.hpp"

using namespace localsolver;
using namespace std;

bool ShouldExitEarly(InputParser *input, const int num_nodes, const int num_edges);


class LocalSolverCallback : public LSCallback {
public:
    LocalSolverCallback(double total_allowed_time_, InputParser *input_parser_, const string graph_name_, int mixingid_, int num_nodes_, int num_edges_, double added_preprocess_time_, long long cutadd_, string sfxout_, bool edge_weights_are_double_) :
                total_allowed_time(total_allowed_time_),
                input_parser(input_parser_),
                graph_name(graph_name_),
                mixingid(mixingid_),
                num_nodes(num_nodes_),
                num_edges(num_edges_),
                added_preprocess_time(added_preprocess_time_),
                cutadd(cutadd_),
                sfxout(sfxout_),
                wedge_weights_are_double(edge_weights_are_double_)
        {
            if (ShouldExitEarly(input_parser, num_nodes, num_edges))
                added_preprocess_time = max(added_preprocess_time, total_allowed_time_ - 60*60);
        }

    void callback(LocalSolver& ls, LSCallbackType /*type*/) {
        LSStatistics stats = ls.getStatistics();
        LSExpression obj = ls.getModel().getObjective(0);

        double runtime = stats.getRunningTime() + added_preprocess_time;

        double cutsz;
        if (wedge_weights_are_double) cutsz = obj.getDoubleValue();
        else cutsz = obj.getValue();

        OutputLiveMaxcut(*input_parser, graph_name, mixingid, num_nodes, num_edges, runtime, cutsz + cutadd, sfxout);

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
    long long cutadd;
    string sfxout;
    bool timelimit_exceeded = false;
    bool wedge_weights_are_double;
};


// https://www.localsolver.com/docs/last/exampletour/maxcut.html
class MaxcutLocalsolver {
public:
    // Number of vertices 
    int n;

    // Number of edges
    int m;

    // Endpoints of each edge 
    vector<int> origin;
    vector<int> dest;

    // Weight of each edge 
    vector<lsdouble> w_double;
    vector<lsint> w_int;

    // Objective 
    LSExpression cutWeight;

    bool use_double;

    // Reads instance data. Takes 1-indexed vertices!
    void readInstanceDouble(const int num_nodes, const vector<tuple<int,int,double>>& edges){
        n = num_nodes;
        m = edges.size();

        origin.resize(m);
        dest.resize(m);
        w_double.resize(m);

        for (int i = 0; i < m; ++i) {
            origin[i] = get<0>(edges[i]);
            dest[i] = get<1>(edges[i]);
            w_double[i] = get<2>(edges[i]);
        }

        use_double = true;
    }
    void readInstanceInt(const int num_nodes, const vector<tuple<int,int,long long>>& edges){
        n = num_nodes;
        m = edges.size();

        origin.resize(m);
        dest.resize(m);
        w_int.resize(m);

        for (int i = 0; i < m; ++i) {
            origin[i] = get<0>(edges[i]);
            dest[i] = get<1>(edges[i]);
            w_int[i] = get<2>(edges[i]);
        }

        use_double = false;
    }

    void solve(int limit, LocalSolverCallback* callback = nullptr){
        LocalSolver localsolver;

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
            incut[e] = model.neq(x.at(origin[e] - 1), x.at(dest[e] - 1));
        }
        
        // Size of the cut
        cutWeight = model.sum();
        for(int e = 0; e < m; e++){
            if (use_double) cutWeight += w_double[e]*incut[e];
            else cutWeight += w_int[e]*incut[e];
        }

        model.maximize(cutWeight);
        model.close();

        if (callback != nullptr) localsolver.addCallback(CT_TimeTicked, callback);

        // Parameterizes the solver. 
        LSPhase phase = localsolver.createPhase();
        phase.setTimeLimit(limit);
        localsolver.getParam().setNbThreads(1);
        localsolver.getParam().setVerbosity(1);
      //  localsolver.getParam().setTimeBetweenTicks(2);
        localsolver.solve();
    }

    double getCutSize() {
        return use_double ? cutWeight.getDoubleValue() : cutWeight.getValue();
    }

    vector<int> getCut() {
        // NOT IMPLEMENTED YET.
        vector<int> ret;
        return ret;
    }
};

#endif