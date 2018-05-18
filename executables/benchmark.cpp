#include "src/mc-graph.hpp"
#include "src/one-way-reducers.hpp"
#include "src/input-parser.hpp"
#include "src/utils.hpp"

#include <iostream>
using namespace std;

int main(int argc, char **argv){
    InputParser input(argc, argv);

    const string data_filepath = input.getCmdOption("-f");
    cout << data_filepath << endl;

    MaxCutGraph G(data_filepath);
    double EE = G.GetEdwardsErdosBound();

    int k = 0;
    int rule_taken;
    OutputDebugLog("----------- START APPLYING ONE-WAY REDUCTION RULES -----------");
    while ((rule_taken = TryOneWayReduce(G, k)) != -1) {
        cout << "RULE: " << rule_taken << endl;
        OutputDebugLog("-----------");
    }

    cout << "|V| = " << G.GetNumNodes() << endl;
    cout << "|E| = " << G.GetNumEdges() << endl; 
    cout << "EE = " << EE << endl;
    cout << "k' = " << k << endl;

    auto S = G.GetMarkedVerticesByOneWayRules();
    cout << "|S| = " << S.size() << endl;
    cout << "S: " << " ";
    for (auto node : S) cout << node << " ";
    cout << endl;
}