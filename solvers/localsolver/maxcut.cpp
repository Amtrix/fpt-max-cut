//********* maxcut.cpp *********

#include <iostream>
#include <fstream>
#include <vector>
#include "localsolver.h"
#include "../../src/utils.hpp"
#include "../../src/mc-graph.hpp"
#include "../../src/one-way-reducers.hpp"

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
    vector<lsint> w;

    // True if vertex x[i] is on the right side of the cut, false if it is on the left side of the cut 
    vector<LSExpression> x;

    // Objective 
    LSExpression cutWeight;

    // Reads instance data. 
    void readInstance(const string& fileName){
        ifstream infile;
        infile.exceptions(ifstream::failbit | ifstream::badbit);
        infile.open(fileName.c_str());

        vector<string> sparams = ReadLine(infile);
        int lsz = sparams.size();
        n = stoi(sparams[0 + (sparams[0]=="p")]);
        m = stoi(sparams[1 + (sparams[0]=="p")]);

        origin.resize(m);
        dest.resize(m);
        w.resize(m);

        for (int i = 0; i < m; ++i) {
            sparams = ReadLine(infile);
            lsz = sparams.size();

            if (sparams.size() < 2) throw std::logic_error("Line malformed: " + to_string(i));

           // AddEdge(params[0] - 1, params[1] - 1);
            //cout << i << " , " << n << " " << m << " : " << params.size() << " : " <<  params[0] << " " << params[1] << endl;
            origin[i] = stoi(sparams[0 + (sparams[0]=="e")]);
            dest[i] = stoi(sparams[1 + (sparams[0]=="e")]);
            w[i] = 1;
        }
        
        /*for(int e = 0; e < m; e++){
            infile >> origin[e];
            infile >> dest[e];
            infile >> w[e];
            w[e] = 1;
        }*/
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
        for (unsigned int i = 0; i < n; ++i)
            outfile << i+1 << " " << x[i].getValue() << endl;
    }

    int getCutSize() {
        return cutWeight.getValue();
    }
};

const int kDataSetCount = 6;
const string paths[] = {
    "../../data/biqmac/ising",
    "../../data/biqmac/rudy",
    "../../data/custom",
    "../../data/KaGen/ba",
    "../../data/KaGen/gnp_undirected",
    "../../data/KaGen/rhg"
};


int main(int argc, char** argv) {
    if (argc < 1) {
        cerr << "Usage: maxcut inputFile [outputFile] [timeLimit] " << endl;
        return 1;
    }

    const char* instanceFile = argc > 1 ? argv[1] : NULL;
    const char* solFile = argc > 2 ? argv[2] : NULL;
    const char* strTimeLimit = argc > 3 ? argv[3] : "3";

    try {
        /*
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
        */

        // UNCOMMENT THIS WHEN WANTING TO COMPUTE CUT FOR ALL DATASETS 
        
        ofstream out("../../data/output/localsolver-lower-bound-cut-size/output_2.0-3sec");
        for (unsigned int i = 0; i < kDataSetCount; ++i) {
            auto sets = GetAllDatasets(paths[i]);
            for (unsigned int i = 0; i < sets.size(); ++i) {
                cout << sets[i] << endl;
                string datapath = sets[i];
            
                Maxcut model;
                model.readInstance(datapath);
                model.solve(atoi("5"));
                int cutsize = model.getCutSize();

                MaxCutGraph G(datapath);

                int k = 0;
                int rule_taken;
                OutputDebugLog("----------- START: APPLYING ONE-WAY REDUCTION RULES TO COMPUTE S -----------");
                MaxCutGraph G_processing_oneway = G; // ! make sure no pointers in G !
                while ((rule_taken = TryOneWayReduce(G_processing_oneway, k)) != -1) {
                    OutputDebugLog("RULE: " + to_string(rule_taken));
                    OutputDebugLog("-----------");
                }

                double k_dest = cutsize - G.GetEdwardsErdosBound();

                out.width(50);
                out << datapath.substr(6) << " ";
                out.width(15);
                out << G.GetNumNodes() << " ";
                out.width(15);
                out << G.GetRealNumEdges() << " ";
                out.width(15);
                out << G.GetEdwardsErdosBound() << " ";
                out.width(15);
                out << cutsize << " ";
                out.width(15);
                out << k_dest << " ";
                out.width(17);
                out << (k/4.0) << " ";
                out.width(17);
                out << k_dest + (k/4.0) << endl;
                out.flush();
            }
        }
        

        return 0;
    } catch (const exception& e){
        cerr << "Error occurred: " << e.what() << endl;
        return 1;
    }
}