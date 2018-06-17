#pragma once

#include "mc-graph.hpp"
#include "utils.hpp"

vector<bool> WhichComponentsAreSpecial(MaxCutGraph& G_0, MaxCutGraph& G_minus_S, vector<vector<int>>& bicomponents, const vector<int>& S) {
    vector<bool> is_special(bicomponents.size(), false);

    for (unsigned int i = 0; i < bicomponents.size(); ++i) {
        auto& component = bicomponents[i];
        for (auto& node : component) {
            if (G_minus_S.IsArticulation(node)) continue; // member of C_ext

            const auto adj = G_0.GetAdjacency(node);
            for (auto& adj_node : adj) {
                if (std::binary_search(S.begin(), S.end(), adj_node)) {
                    is_special[i] = true;
                    break;
                }
            }

            if (is_special[i])
                break;
        }
    }

    return is_special;
}

vector<bool> WhichComponentIsTriag(MaxCutGraph& G_minus_S, vector<vector<int>>& bicomponents, vector<bool>& is_special) {
    vector<bool> is_triag_block(bicomponents.size(), false);

    for (unsigned int i = 0; i < bicomponents.size(); ++i) {
        if (is_special[i]) continue;

        auto component = bicomponents[i];
        if (component.size() != 3) continue;

        int artcnt = 0;
        for (auto& node : component)
            if (G_minus_S.IsArticulation(node))
                artcnt++;
        
        if (artcnt >= 3) continue;

        is_triag_block[i] = true; // i.e. non-special block with 3 vertices and |C_ext| <= 2
    }

    return is_triag_block;
}

int ExhaustiveTwoWayReduce(MaxCutGraph &G_0, const vector<int>& S) {
    auto G_minus_S_vertex_set = SetSubstract(G_0.GetAllExistingNodes(), S);
    MaxCutGraph G_minus_S(G_0, G_minus_S_vertex_set);
    assert(G_minus_S.IsCliqueForest());

    auto bicomponents = G_minus_S.GetBiconnectedComponents();
    auto is_special = WhichComponentsAreSpecial(G_0, G_minus_S, bicomponents, S);

    auto is_triag_block = WhichComponentIsTriag(G_minus_S, bicomponents, is_special);
    
    vector<vector<int>> arti_to_triag;
    for (unsigned int i = 0; i < bicomponents.size(); ++i) {
        if (is_triag_block[i] == false) continue;

        auto component = bicomponents[i];
        for (auto& node : component)
            if (G_minus_S.IsArticulation(node) == false)
                arti_to_triag[node].push_back(i);
    }

    for (unsigned int i = 0; i < bicomponents.size(); ++i) {
        cout << "COMPONENT " << i << " : " << is_special[i] << " " << is_triag_block[i] << endl;
        auto component = bicomponents[i];

        unordered_map<string, vector<int>> partition;
        for (auto node : component) {
            if (G_minus_S.IsArticulation(node)) continue;

            auto adj = G_0.GetAdjacency(node);
            adj.push_back(node); // we need self here, otherwise we just partition in n x 1-node sets.
            sort(adj.begin(), adj.end());

            string key = "";
            for (auto w : adj) key += to_string(w) + "|";
            partition[key].push_back(node);
        //    cout << key << " => " << node << endl;
        }

        string largest_key = "";
        for (auto entry : partition)
            if (largest_key == "" || (partition[largest_key].size() < entry.second.size()))
                largest_key = entry.first;
        
        if (largest_key == "") continue;

       // cout << largest_key << " " << partition[largest_key].size() << endl;

        auto S_intersect_NX = SetIntersection(partition[largest_key], S);

        //cout << "intersection: " << endl;
        //for (auto node : S_intersect_NX)
        //    cout << node << " ";
        //cout << endl;

        double sz = (component.size() + S_intersect_NX.size()) / 2.0;

        cout << partition[largest_key].size() << " " << sz << endl;
        if (partition[largest_key].size() > sz && sz >= 1 - 1e-9) {
            cout << "REMOVE: " << partition[largest_key][0] << " " << partition[largest_key][1] << endl;
            G_0.RemoveNode(partition[largest_key][0]);
            G_0.RemoveNode(partition[largest_key][1]);
            return 9;
        }
    }
    
    return -1;
}