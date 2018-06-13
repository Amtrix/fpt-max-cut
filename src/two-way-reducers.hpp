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

void ExhaustiveTwoWayReduce(MaxCutGraph &G_0, const vector<int>& S) {
    auto G_minus_S_vertex_set = SetSubstract(G_0.GetAllExistingNodes(), S);
    MaxCutGraph G_minus_S(G_0, G_minus_S_vertex_set);

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


}