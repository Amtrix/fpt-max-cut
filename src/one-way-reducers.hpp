#pragma once

#include "mc-graph.hpp"
#include "utils.hpp"

int TryRule5(MaxCutGraph& G, const vector<int>& leaf_block, const int r, int& k) {
    if (G.IsClique(leaf_block)) { 
        G.ApplyRule5(leaf_block, r);
        if (leaf_block.size() % 2 == 0) k--; // We do even instead odd, because here C = component U r
        return 5;
    }
    return -1;
}

int TryRule3(MaxCutGraph& G, const vector<int>& leaf_block, const int r, int& k) {
    auto component_minus_r = SetSubstract(leaf_block, vector<int>{r});
    if (G.IsClique(component_minus_r)) {
        G.ApplyRule3(leaf_block, r); // TODO(): Make sure leaf_block is right here.
        k -= 2;
        return 3;
    }
    return -1;
}

int TryRule7(MaxCutGraph& G, const vector<int>& leaf_block, const int r, int& k) {
    auto adj_to_r = G.GetAdjacency(r);
    vector<int> inside_component;

    for (const int node : leaf_block)
        if (G.AreAdjacent(node, r))
            inside_component.push_back(node);
    
    if (inside_component.size() == 2) {
        vector<int> s1 = SetSubstract(leaf_block, {r, inside_component[0]});
        vector<int> s2 = SetSubstract(leaf_block, {r, inside_component[1]});
        vector<int> intersect = SetSubstract(leaf_block, {r, inside_component[0], inside_component[1]});

        if (G.IsClique(s1) && G.IsClique(s2)) {
            G.ApplyRule7(intersect, inside_component[0], inside_component[1]);
            k--;
            return 7;
        }
    }

    return -1;
}

int TryRule6(MaxCutGraph& G, const vector<int>& leaf_block, const int r, int& k) {
    vector<int> induced_path = G.FindInducedPathForRule6(leaf_block, r);

    if (induced_path.size() > 0) {
        OutputDebugVector("Induced path for rule 6", induced_path);

        assert(G.AreAdjacent(induced_path[0], induced_path[1]));
        assert(G.AreAdjacent(induced_path[1], induced_path[2]));
        assert(!G.AreAdjacent(induced_path[0], induced_path[2]));

        G.ApplyRule6(induced_path);
        k--;
        return 6;
    }

    return -1;
}

/*
 * Difference from the paper:
 * - vertices are not being deleted, but just made disconnected from the rest of the graph.
 * 
**/
int TryOneWayReduce(MaxCutGraph& G, int &k) {
    // First, find leaf block.
    vector<int> component;
    int r;

    std::tie(component, r) = G.GetLeafBlockAndArticulation(true);

    if (r == -1) return -1;

    // ############## TRY RULE 5 ##############
    if (TryRule5(G, component, r, k) != -1) return 5;
    // #########################################

    // ############## TRY RULE 3 ##############
    if (TryRule3(G, component, r, k) != -1) return 3;
    // #########################################

    // ############## TRY RULE 7 ##############
    if (TryRule7(G, component, r, k) != -1) return 7;
    // #########################################

    // ############## TRY RULE 6 ##############
    if (TryRule6(G, component, r, k) != -1) return 6;
    // #########################################

    return -1;
}