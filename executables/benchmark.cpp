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
    ios_base::sync_with_stdio(false);
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

    vector<int> tot_used_rules(10, 0);
    for (string data_filepath : all_sets_to_evaluate) {
        cout << endl;
        cout << "================ RUNNING BENCHMARK ON " + data_filepath + " ================ " << endl;
        MaxCutGraph G(data_filepath);
        double EE = G.GetEdwardsErdosBound();

        int k = 0;
        int rule_taken;
        OutputDebugLog("----------- START APPLYING ONE-WAY REDUCTION RULES -----------");
        MaxCutGraph Gp = G; // ! make sure no pointers in G !
        while ((rule_taken = TryOneWayReduce(Gp, k)) != -1) {
            OutputDebugLog("RULE: " + to_string(rule_taken));
            OutputDebugLog("-----------");
            tot_used_rules[rule_taken]++;
        }

        cout << "|V| = " << G.GetNumNodes() << endl;
        cout << "|E| = " << G.GetNumEdges() << endl; 
        cout << "EE = " << EE << endl;
        cout << "k' = " << k << endl;

        G.SetMarkedVertices(Gp.GetMarkedVerticesByOneWayRules());
        auto S = G.GetMarkedVerticesByOneWayRules();
        cout << "|S| = " << S.size() << endl;
        cout << "S: " << " ";
        for (auto node : S) cout << node << " ";
        cout << endl;

        // Try reduce size of S
        G.ReduceMarksetVertexSet();
        S = G.GetMarkedVerticesByOneWayRules();


        cout << "new |S| = " << S.size() << endl;
        cout << "new S: " << " ";
        for (auto node : S) cout << node << " ";
        cout << endl;
    
#ifdef DEBUG
        auto G_minus_S_vertex_set = SetSubstract(G.GetAllExistingNodes(), S);
        MaxCutGraph G_minus_S(G, G_minus_S_vertex_set);
        assert(G_minus_S.IsCliqueForest());
#endif

        if (input.cmdOptionExists("-cc-brute")) { // temp flag since this is very slow
            int mx_sol = G.ComputeOptimalColoringBruteforce(S);
            OutputDebugLog("mx_sol = " + to_string(mx_sol));
            OutputDebugVector("Coloring", G.GetMaxCutColoring());

            auto allv = G.GetAllExistingNodes();
            auto sss = G.MaxCutExtension(allv, G.GetMaxCutColoring());
            cout << (get<0>(sss)) << endl;
        }
    }

    cout << endl;
    cout << " ================ Analysis over all rules commulative ================ " << endl;
    for (int r = 1; r <= 7; ++r)
        cout << "Rule " << r << " was used " << tot_used_rules[r] << " times." << endl;
    cout << " ===================================================================== " << endl << endl;
}