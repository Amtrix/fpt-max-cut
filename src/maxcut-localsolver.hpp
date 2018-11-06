//********* maxcut.cpp *********

#include <iostream>
#include <fstream>
#include <vector>
#include "localsolver.h"
#include "../../src/utils.hpp"
#include "../../src/mc-graph.hpp"

using namespace localsolver;
using namespace std;

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

    void solve(int limit){
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

        // Parameterizes the solver. 
        LSPhase phase = localsolver.createPhase();
        phase.setTimeLimit(limit);
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