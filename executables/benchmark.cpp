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

        auto S = Gp.GetMarkedVerticesByOneWayRules();
        cout << "|S| = " << S.size() << endl;
        cout << "S: " << " ";
        for (auto node : S) cout << node << " ";
        cout << endl;

        /*
        S.erase(std::remove(S.begin(), S.end(), 9), S.end());
        S.erase(std::remove(S.begin(), S.end(), 8), S.end());
        auto vvv = SetSubstract(G.GetAllExistingNodes(), S);
        MaxCutGraph newG(G, vvv);
        assert(newG.IsCliqueForest());
        
        newG.GetLeafBlockAndArticulation(true);*/

        // Try reduce size of S
        while (1) {
            bool was_possible = false;
            for (int node : S) {
                auto G_minus_S_vertex_set = SetSubstract(G.GetAllExistingNodes(), S);
                auto when_node_added = SetUnion(G_minus_S_vertex_set, vector<int>{node});
                MaxCutGraph G_minus_newS(G, when_node_added);
                
                if (G_minus_newS.IsCliqueForest()) {
                    S.erase(std::remove(S.begin(), S.end(), node), S.end());
                    was_possible = true;
                    break;
                }
            }

            if (!was_possible) break;
        }


        cout << "new |S| = " << S.size() << endl;
        cout << "new S: " << " ";
        for (auto node : S) cout << node << " ";
        cout << endl;
    
#ifdef DEBUG
        auto G_minus_S_vertex_set = SetSubstract(G.GetAllExistingNodes(), S);
        MaxCutGraph G_minus_S(G, G_minus_S_vertex_set);
        assert(G_minus_S.IsCliqueForest());
#endif

        if (input.cmdOptionExists("-cc")) { // temp flag since this is very slow
            int mx_sol = 0;
            for (int mask = 0; mask < (1 << S.size()); ++mask) {
                vector<int> s_color;
                for (unsigned int i = 0; i < S.size(); ++i)
                    if (mask & (1<<i)) s_color.push_back(1);
                    else s_color.push_back(0);
                
                int sol = G.ComputeCut(S, s_color);
                mx_sol = max(mx_sol, sol);
            }

            OutputDebugLog("mx_sol = " + to_string(mx_sol));
        }
    }

    cout << endl;
    cout << " ================ Analysis over all rules commulative ================ " << endl;
    for (int r = 1; r <= 7; ++r)
        cout << "Rule " << r << " was used " << tot_used_rules[r] << " times." << endl;
    cout << " ===================================================================== " << endl << endl;
}