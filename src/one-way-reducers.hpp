#pragma once

#include "mc-graph.hpp"
#include "utils.hpp"

/*
 * Difference from the paper:
 * - vertices are not being deleted, but just made disconnected from the rest of the graph.
 * 
**/
int TryOneWayReduce(MaxCutGraph& G, int &k) {
    // First, find leaf block.
    int selected_block_dx = -1;
    auto bicomponents = G.GetBiconnectedComponents();

    
    OutputDebugLog("Number of biconnected components in graph: " + to_string(bicomponents.size()) + ", all components:");
    for (unsigned int dx = 0; dx < bicomponents.size(); ++dx) {
        const auto& component = bicomponents[dx];
        OutputDebugVector("Component " + to_string(dx), component);
    }
    OutputDebugLog("-- END-COMPONENTS --");
    

    for (unsigned int i = 0; i < bicomponents.size(); ++i) {
        const auto& component = bicomponents[i];
        if (component.size() == 1) continue; // We ignore 1-vertex components

        int articulation_count = 0;
        for (const int node : component) articulation_count += G.IsArticulation(node);
        if (articulation_count <= 1 && (selected_block_dx == -1 || component.size() < bicomponents[selected_block_dx].size()))
            selected_block_dx = i;
    }

    // No leaf block with more than one vertex found => graph contains only isolated vertices.
    if (selected_block_dx == -1) return -1;
    const auto& component = bicomponents[selected_block_dx];

    // Find r as outlined in paper. r is cut vertex if exists, or arbitrary vertex if not.
    int r = 0;
    for (unsigned int i = 1; i < component.size(); ++i)
        if (G.IsArticulation(component[i]))
            r = i;
    r = component[r];
    OutputDebugLog("r = " + to_string(r) + ", X = Component " + to_string(selected_block_dx));

    // ############## TRY RULE 5 ##############
    if (G.IsClique(component)) { 
        G.ApplyRule5(component, r);
        if (component.size() % 2 == 0) k--; // We do even instead odd, because here C = component U r
        return 5;
    }
    // #########################################

    // ############## TRY RULE 3 ##############
    auto component_minus_r = SetSubstract(component, vector<int>{r});
    if (G.IsClique(component_minus_r)) {
        G.ApplyRule3(component, r);
        k -= 2;
        return 3;
    }
    // #########################################

    // ############## TRY RULE 7 ##############
    auto adj_to_r = G.GetAdjacency(r);
    vector<int> inside_component;
    for (const int node : component)
        if (G.AreAdjacent(node, r))
            inside_component.push_back(node);
    
    if (inside_component.size() == 2) {
        vector<int> s1 = SetSubstract(component, {r, inside_component[0]});
        vector<int> s2 = SetSubstract(component, {r, inside_component[1]});
        vector<int> intersect = SetSubstract(component, {r, inside_component[0], inside_component[1]});

        if (G.IsClique(s1) && G.IsClique(s2)) {
            G.ApplyRule7(intersect, inside_component[0], inside_component[1]);
            k--;
            return 7;
        }
    }
    // #########################################


    // ############## TRY RULE 6 ##############
    vector<int> induced_path = G.FindInducedPathForRule6(component, r);
    if (induced_path.size() > 0) {
        OutputDebugVector("Induced path for rule 6", induced_path);

        assert(G.AreAdjacent(induced_path[0], induced_path[1]));
        assert(G.AreAdjacent(induced_path[1], induced_path[2]));
        assert(!G.AreAdjacent(induced_path[0], induced_path[2]));

        G.ApplyRule6(induced_path);
        k--;
        return 6;
    }
    
    
    // #########################################
    return -1;
}