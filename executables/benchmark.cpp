#include "src/mc-graph.hpp"
#include "src/one-way-reducers.hpp"
#include "src/input-parser.hpp"

#include <iostream>
using namespace std;

int main(int argc, char **argv){
    InputParser input(argc, argv);

    const string data_filepath = input.getCmdOption("-f");
    cout << data_filepath << endl;

    MaxCutGraph G(data_filepath);

    int k = 5;
    int rule_taken;
    while ((rule_taken = TryOneWayReduce(G, k)) != -1) {
        cout << "RULE: " << rule_taken << endl;
    }
}