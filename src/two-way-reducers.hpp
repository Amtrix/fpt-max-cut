#pragma once

#include <vector>
#include <algorithm>
#include <cassert>

#include "mc-graph.hpp"
#include "utils.hpp"

vector<bool> WhichComponentsAreSpecial(MaxCutGraph& G_0, MaxCutGraph& G_minus_S, const vector<int>& S) {
    const auto& bicomponents = G_minus_S.GetBiconnectedComponents();

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

vector<bool> WhichComponentIsTriag(MaxCutGraph& G_minus_S, vector<bool>& is_special) {
    const auto& bicomponents = G_minus_S.GetBiconnectedComponents();

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

int TryRule9(MaxCutGraph& G_0, MaxCutGraph& G_minus_S, vector<bool>& is_triag_block) {
    const auto& bicomponents = G_minus_S.GetBiconnectedComponents();

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
        if (arti_to_triag[i].size() < 2) continue;

        // apply here:
        int cdx1 = arti_to_triag[i][0], cdx2 = arti_to_triag[i][1];
        for (auto x1 : bicomponents[cdx1]) {
            for (auto x2 : bicomponents[cdx2]) {
                if (x1 == (int)i || x2 == (int)i) continue;
                G_0.AddEdge(x1, x2);
            }
        }

        return 9;
    }

    return -1;
}

int TryRule8(MaxCutGraph& G_0, MaxCutGraph& G_minus_S, const vector<int>& S) {
    const auto& bicomponents = G_minus_S.GetBiconnectedComponents();

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
        for (auto entry : partition) {
            if (largest_key == "" || (partition[largest_key].size() < entry.second.size()))
                largest_key = entry.first;
        }
        
        if (largest_key == "") continue;

        auto S_intersect_NX = SetIntersection(S, key_descr[largest_key]); // S operator(intersect) N_{G}(X)

#ifdef DEBUG
        for (auto node : partition[largest_key]) {
           (void) node;
           assert(G_minus_S.IsArticulation(node) == false);
        }
#endif

        double sz = (component.size() + S_intersect_NX.size()) / 2.0;
        
      //  cout << partition[largest_key].size() << " " << sz << "( = (" << component.size() << " " << S_intersect_NX.size() << ") / 2.0" <<  endl;
        if (partition[largest_key].size() > sz && sz >= 1 - 1e-9) {
            G_0.RemoveNode(partition[largest_key][0]);
            G_0.RemoveNode(partition[largest_key][1]);
            return 8;
        }
    }

    return -1;
}

// Rule 9 in origin paper: https://arxiv.org/abs/1212.6848
int TryRule10(MaxCutGraph& G_0, MaxCutGraph& G_minus_S, const vector<int>& S) {
    const auto& bicomponents = G_minus_S.GetBiconnectedComponents();

    unordered_map<int,bool> in_S;
    for (auto node : S) in_S[node] = true;

    for (unsigned int i = 0; i < bicomponents.size(); ++i) {
        auto component = bicomponents[i];

        if (component.size() % 2) continue; // odd? => nope

        // Get subset in C_{int} with no adjaceny into S.
        vector<int> X;
        for (auto node : component) {
            if (G_minus_S.IsArticulation(node)) // is in C_{ext}
                continue;

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

        if (X.size() >= component.size() / 2) {
            G_0.RemoveNode(X[0]);
            // TODO: k--
            return 10;
        }
    }

    return -1;
}

// Rule 10 in origin paper: https://arxiv.org/abs/1212.6848
int TryRule11(MaxCutGraph& G_0, MaxCutGraph& G_minus_S) {
    const auto& bicomponents = G_minus_S.GetBiconnectedComponents();

    for (auto& component : bicomponents) {
        if (component.size() != 3) continue;

        int u = -1;
        for (int i : {0, 1, 2})
            if (G_0.GetAdjacency(component[i]).size() == 2) { u = i; break; } // those two edges must be to the other two vertices, otherwise not biconnected.
        
        if (u == -1) continue;

        int x1 = component[u == 0]; // if u is 0, we get 1
        int x2 = component[2 - (u == 2)]; // gonna be always 2nd except if u is 2nd.
        int x0 = component[u];

        assert(x0 != x1 && x1 != x2 && x0 != x2); // just to make sure the magic from above is right.

        auto allvertices = G_0.GetAllExistingNodes();
        auto allvertices_minus_x0 = SetSubstract(allvertices, {x0});
        MaxCutGraph G0_minus_x0(G_0, allvertices_minus_x0);

        if (G0_minus_x0.IsBridgeBetween(x1, x2)) {
            G_0.RemoveNode(x0);
            G_0.RemoveNode(x1);
            G_0.RemoveNode(x2);
            // TODO: create a new node X and add edges between X and all adjacent to {x1, x2} in G0_minus_x0 
            throw std::logic_error("Effects not fully implemented when rule 11 triggers.");
            
        } else {
            G_0.RemoveNode(x0);
            //G_0.RemoveEdge(x1, x2);
            // k--;
            // TODO: above two lines.
            throw std::logic_error("Effects not fully implemented when rule 11 triggers.");
        }
        
        
        return 11;
    }

    return -1;
}

// Rule 11 in origin paper: https://arxiv.org/abs/1212.6848
// This rule is really bad. It expects a whole connected component of G - S to be adjacent to only a single vertex in S.
// Implementation bellow needs almost full rework due to the realization that it's not a connected sub-component.
/*int TryRule12(MaxCutGraph& G_0, MaxCutGraph& G_minus_S, const vector<int>& S) {
    const auto& bicomponents = G_minus_S.GetBiconnectedComponents();

    unordered_map<int,bool> in_S;
    for (auto node : S) in_S[node] = true;

    vector<int> existing_nodes = G_minus_S.GetAllExistingNodes();
    for (auto node : existing_nodes) {
        auto adj = G_0.GetAdjacency(node);
        int in_S_cnt = 0;
        for (auto w : adj)
            in_S_cnt += in_S[w];
        
        if (in_S_cnt == 1) {
            cout << "12 MIGHT BE POSSIBLE" << endl;
            return -1;
        }
    }
    return -1;
}*/

// Passing k missing. Be aware that decreasing k by 1 is actually 1/4 when considering above edwards erdos bound.
int ExhaustiveTwoWayReduce(MaxCutGraph& G_0, const vector<int>& S, vector<pair<double,int>>& times_within_call) {
    auto t0 = std::chrono::high_resolution_clock::now();
    auto t1 = t0;

    auto G_minus_S_vertex_set = SetSubstract(G_0.GetAllExistingNodes(), S);
    MaxCutGraph G_minus_S(G_0, G_minus_S_vertex_set);

#ifdef DEBUG
    assert(G_minus_S.IsCliqueForest());
#endif

    auto bicomponents = G_minus_S.GetBiconnectedComponents();
    auto is_special = WhichComponentsAreSpecial(G_0, G_minus_S, S);
    auto is_triag_block = WhichComponentIsTriag(G_minus_S, is_special);

    t1 = std::chrono::high_resolution_clock::now();
    times_within_call.push_back(make_pair(std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000., -1));
    t0 = t1;
    
    int res;
    res = TryRule9(G_0, G_minus_S, is_triag_block);

    t1 = std::chrono::high_resolution_clock::now();
    times_within_call.push_back(make_pair(std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000., 9));
    t0 = t1;
    if (res > -1) return res;

    res = TryRule8(G_0, G_minus_S, S);

    t1 = std::chrono::high_resolution_clock::now();
    times_within_call.push_back(make_pair(std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000., 8));
    t0 = t1;
    if (res > -1) return res;

    //if ((res = TryRule10(G_0, G_minus_S, S)) > -1) return res;
    //if ((res = TryRule11(G_0, G_minus_S)) > -1) return res;
    //if ((res = TryRule12(G_0, G_minus_S, S)) > -1) return res;
    
    return -1;
}