#include <iostream>
#include <fstream>
#include <vector>
#include "localsolver.h"
#include "./src/mc-graph.hpp"

using namespace localsolver;
using namespace std;

class Maxcut {
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
    vector<lsdouble> w;

    // True if vertex x[i] is on the right side of the cut, false if it is on the left side of the cut 
    vector<LSExpression> x;

    // Objective 
    LSExpression cutWeight;

    // Reads instance data. 
    void readInstance(const string& fileName){
        cout << "F: " << fileName << endl; 
        ifstream infile;
        ///infile.exceptions(ifstream::failbit | ifstream::badbit);
        infile.open(fileName.c_str());
        
        infile >> n;
        infile >> m;

        origin.resize(m);
        dest.resize(m);
        w.resize(m);

        for(int e = 0; e < m; e++){
            long long p1,p2,p3;
            infile >> p1 >> p2 >> p3;

            origin[e] = p1;
            dest[e] = p2;
            w[e] = p3 / 10000.0;
            
            cout << origin[e] << " " << dest[e] << " " << w[e] << endl;
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
        
        localsolver.getParam().setVerbosity(1);
      //  localsolver.getParam().setTimeBetweenTicks(2);
      std::cout.setf ( std::ios::fixed );
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
        for (unsigned int i = 0; i < n; ++i)
            outfile << i+1 << " " << x[i].getValue() << endl;
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: maxcut inputFile [outputFile] [timeLimit] " << endl;
        return 1;
    }

    const char* instanceFile = argv[1];
    const char* solFile = argc > 2 ? argv[2] : NULL;
    const char* strTimeLimit = argc > 3 ? argv[3] : "10";

    try {
        MaxCutGraph G(instanceFile);
       // cout << G.ComputeMaxCutWithMQLib(60, nullptr).first << endl;
      //  cout << G.ComputeMaxCutWithLocalsolver(60, nullptr).first << endl;

        Maxcut model;
        model.readInstance(instanceFile);
        model.solve(atoi(strTimeLimit));
        //if(solFile != NULL) model.writeSolution(solFile);
        cout << "GET VALUE" << endl;
        cout << model.cutWeight.getDoubleValue() << endl;
        return 0;
    } catch (const exception& e){
        cerr << "Error occurred: " << e.what() << endl;
        return 1;
    }
}