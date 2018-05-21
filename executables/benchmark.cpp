#include "src/mc-graph.hpp"
#include "src/one-way-reducers.hpp"
#include "src/input-parser.hpp"
#include "src/utils.hpp"

#include <iostream>
#include <experimental/filesystem>
using namespace std;
using namespace std::experimental;

const int kDataSetCount = 3;
const string paths[] = {
    "../data/biqmac/ising",
    "../data/biqmac/rudy",
    "../data/custom"
};

vector<string> GetAllDatasets(const string path) {
    vector<string> ret;
    for (auto & p : filesystem::directory_iterator(path))
        if (filesystem::is_regular_file(p))
            ret.push_back(p.path().u8string());
    return ret;
}

int main(int argc, char **argv){
    InputParser input(argc, argv);

    vector<string> all_sets_to_evaluate;
    if (input.cmdOptionExists("-f")) {
        const string data_filepath = input.getCmdOption("-f");
        all_sets_to_evaluate.push_back(data_filepath);
    } else {
        // get all datasets from data/
        for (unsigned int i = 0; i < kDataSetCount; ++i) {
            auto sets = GetAllDatasets(paths[i]);
            for (unsigned int i = 0; i < sets.size(); ++i)
                all_sets_to_evaluate.push_back(sets[i]);
        }
    }

    for (string data_filepath : all_sets_to_evaluate) {
        cout << endl;
        cout << "================ RUNNING BENCHMARK ON " + data_filepath + " ================ " << endl;
        MaxCutGraph G(data_filepath);
        double EE = G.GetEdwardsErdosBound();

        int k = 0;
        int rule_taken;
        OutputDebugLog("----------- START APPLYING ONE-WAY REDUCTION RULES -----------");
        while ((rule_taken = TryOneWayReduce(G, k)) != -1) {
            OutputDebugLog("RULE: " + to_string(rule_taken));
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
}