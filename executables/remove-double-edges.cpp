#include <bits/stdc++.h>
#include "src/mc-graph.hpp"
#include "src/input-parser.hpp"
using namespace std;

int main(int argc, char **argv){
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

    cout << G.GetRealNumNodes() << " " << G.GetRealNumEdges() << endl;
    for (auto e : edges)
        cout << e.first + 1 << " " << e.second + 1 << endl;
}