/**
 * It might be possible to improve here the set size of S!
 * **/

#pragma once

#include <cassert>
#include <algorithm>

#include "mc-graph.hpp"
#include "utils.hpp"

int TryRule5(MaxCutGraph& G, const vector<int>& leaf_block, const int r, int& k) {
    if (G.IsClique(leaf_block)) { 
        G.ApplyOneWayRule5(leaf_block, r);
        if (leaf_block.size() % 2 == 0) k--; // We do even instead odd, because here C = component U r
        return 5;
    }
    return -1;
}

int TryRule3(MaxCutGraph& G, const vector<int>& leaf_block, const int r, int& k) {
    auto component_minus_r = SetSubstract(leaf_block, vector<int>{r});
    if (G.IsClique(component_minus_r)) {
        G.ApplyOneWayRule3(leaf_block, r); // TODO(): Make sure leaf_block is right here.
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
        if (G.AreAdjacent(inside_component[0], inside_component[1]))
            return -1;

        vector<int> s1 = SetSubstract(leaf_block, {r, inside_component[0]});
        vector<int> s2 = SetSubstract(leaf_block, {r, inside_component[1]});
        vector<int> intersect = SetSubstract(leaf_block, {r, inside_component[0], inside_component[1]});

        if (G.IsClique(s1) && G.IsClique(s2)) {
            G.ApplyOneWayRule7(intersect, inside_component[0], inside_component[1]);
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

        G.ApplyOneWayRule6(induced_path);
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
int TryOneWayReduce(MaxCutGraph& G, int &k, vector<pair<double,int>>& times_within_call) {
    // First, find leaf block.
    vector<int> component;
    int r;

    bool res;
    auto t0 = std::chrono::high_resolution_clock::now();
    auto t1 = t0;
    std::tie(component, r) = G.GetLeafBlockAndArticulation(false);

    t1 = std::chrono::high_resolution_clock::now();
    times_within_call.push_back(make_pair(std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000., -1));
    t0 = t1;

    if (r == -1) return -1;

    // ############## TRY RULE 5 ##############
    res = TryRule5(G, component, r, k) != -1;
    t1 = std::chrono::high_resolution_clock::now();
    times_within_call.push_back(make_pair(std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000., 5));
    t0 = t1;

    if (res) return 5;

    // ############## TRY RULE 3 ##############
    res = TryRule3(G, component, r, k) != -1;
    t1 = std::chrono::high_resolution_clock::now();
    times_within_call.push_back(make_pair(std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000., 3));
    t0 = t1;

    if (res) return 3;

    // ############## TRY RULE 7 ##############
    res = TryRule7(G, component, r, k) != -1;
    t1 = std::chrono::high_resolution_clock::now();
    times_within_call.push_back(make_pair(std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000., 7));
    t0 = t1;

    if (res) return 7;

    // ############## TRY RULE 6 ##############
    res = TryRule6(G, component, r, k) != -1;
    t1 = std::chrono::high_resolution_clock::now();
    times_within_call.push_back(make_pair(std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000., 6));
    t0 = t1;

    if (res) return 6;

    return -1;
}