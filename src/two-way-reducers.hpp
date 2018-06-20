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

int TryRule9(MaxCutGraph& G_0, MaxCutGraph& G_minus_S, const vector<vector<int>>& bicomponents, vector<bool>& is_triag_block) {
    const unsigned int num_nodes = G_minus_S.GetNumNodes();
    vector<vector<int>> arti_to_triag(num_nodes);
    for (unsigned int i = 0; i < bicomponents.size(); ++i) {
        auto& component = bicomponents[i];

        if (is_triag_block[i] == false) continue;
        
        for (auto& node : component)
            if (G_minus_S.IsArticulation(node))
                arti_to_triag[node].push_back(i);
    }

    for (unsigned int i = 0; i < num_nodes; ++i) {
        if (arti_to_triag[i].size() >= 2) {

            // apply here:
            for (auto x1 : arti_to_triag[0]) {
                for (auto x2 : arti_to_triag[1]) {
                    if (x1 == (int)i || x2 == (int)i) continue;
                    G_0.AddEdge(x1, x2);
                }
            }

            assert(false); // not yet implemented as it hasn't been really triggered yet...
            return 9;
        }
    }

    return -1;
}

int TryRule8(MaxCutGraph& G_0, MaxCutGraph& G_minus_S, const vector<vector<int>>& bicomponents, const vector<int>& S) {
    for (unsigned int i = 0; i < bicomponents.size(); ++i) {
        auto component = bicomponents[i];

        unordered_map<string, vector<int>> partition;
        unordered_map<string, vector<int>> key_descr;
        for (auto node : component) {
            if (G_minus_S.IsArticulation(node)) continue;

            auto adj = G_0.GetAdjacency(node);
            adj.push_back(node); // we need self here, otherwise we just partition in n x 1-node sets.
            sort(adj.begin(), adj.end());

            string key = "";
            for (auto w : adj) key += to_string(w) + "|";
            partition[key].push_back(node);

            // just utility, helps to split the string key into a vector of its parts
            if (key_descr.find(key) == key_descr.end())
                key_descr[key] = adj;
        }

        string largest_key = "";
        for (auto entry : partition)
            if (largest_key == "" || (partition[largest_key].size() < entry.second.size()))
                largest_key = entry.first;
        
        if (largest_key == "") continue;

        auto S_intersect_NX = SetIntersection(S, key_descr[largest_key]); // S operator(intersect) N_{G}(X)

#ifdef DEBUG
        for (auto node : partition[largest_key])
            assert(G_minus_S.IsArticulation(node) == false);
#endif

        double sz = (component.size() + S_intersect_NX.size()) / 2.0;
        
        if (partition[largest_key].size() > sz && sz >= 1 - 1e-9) {
            G_0.RemoveNode(partition[largest_key][0]);
            G_0.RemoveNode(partition[largest_key][1]);
            return 8;
        }
    }

    return -1;
}

// Rule 9 in origin paper: https://arxiv.org/abs/1212.6848
int TryRule10(MaxCutGraph& G_0, const vector<vector<int>>& bicomponents, const vector<int>& S) {
    unordered_map<int,bool> in_S;
    for (auto node : S) in_S[node] = true;

    for (unsigned int i = 0; i < bicomponents.size(); ++i) {
        auto component = bicomponents[i];

        if (component.size() % 2 == 0) continue;

        vector<int> X;
        for (auto node : component) {
            auto& adj = G_0.GetAdjacency(node);
            bool ok = true;

            for (auto w : adj) {
                if (in_S[w]) {
                    ok = false;
                    break;
                }
            }

            if (ok) X.push_back(node);
        }

        if (X.size() == component.size() / 2) {
            G_0.RemoveNode(X[0]);
            return 10;
        }
    }

    return -1;
}

int ExhaustiveTwoWayReduce(MaxCutGraph& G_0, const vector<int>& S) {
    auto G_minus_S_vertex_set = SetSubstract(G_0.GetAllExistingNodes(), S);
    MaxCutGraph G_minus_S(G_0, G_minus_S_vertex_set);

#ifdef DEBUG
    assert(G_minus_S.IsCliqueForest());
#endif

    auto bicomponents = G_minus_S.GetBiconnectedComponents();
    auto is_special = WhichComponentsAreSpecial(G_0, G_minus_S, bicomponents, S);
    auto is_triag_block = WhichComponentIsTriag(G_minus_S, bicomponents, is_special);
    
    int res;

    if ((res = TryRule9(G_0, G_minus_S, bicomponents, is_triag_block)) > -1) return res;

    if ((res = TryRule8(G_0, G_minus_S, bicomponents, S)) > -1) return res;

    if ((res = TryRule10(G_0, bicomponents, S)) > -1) return res;
    
    
    return -1;
}