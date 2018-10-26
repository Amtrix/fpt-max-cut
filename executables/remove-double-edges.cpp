#include <iostream>
#include <fstream>
#include "src/mc-graph.hpp"
#include "src/input-parser.hpp"
using namespace std;

int main(int argc, char **argv){
    ios_base::sync_with_stdio(false);
    InputParser input(argc, argv);

    string data_file;
    if (input.cmdOptionExists("-data")) {
        data_file = input.getCmdOption("-data");
    } else {
        cout << "No file provided." << endl;
        return 0;
    }

    MaxCutGraph G(data_file);
    const auto edges = G.GetAllExistingEdges();


    ofstream out(data_file);
    out << G.GetRealNumNodes() << " " << G.GetRealNumEdges() << endl;
    for (auto e : edges)
        out << e.first + 1 << " " << e.second + 1 << endl;
}