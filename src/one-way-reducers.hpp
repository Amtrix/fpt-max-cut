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

    
    cout << "SZ: " << bicomponents.size() << ", all components: " << endl;
    for (auto component : bicomponents) {
        for (unsigned int i = 0; i < component.size(); ++i)
            cout << component[i] << " ";
        cout << endl;
    }
    cout << "-- END-COMPONENTS --" << endl;
    

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

    /*
    // PRINT-CHECK FOR VALUE X and r AS IN PAPER
    cout << "r: " << r << endl;
    cout << "X: " << " ";
    for (int i = 0; i < component.size(); ++i)
        cout << component[i] << " " ; cout << endl;
    */

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
    
    
    /*MaxCutGraph c_graph(G, component);

    MaxCutGraph c_minus_r_graph(c_graph, component_minus_r);
    c_minus_r_graph.ComputeArticulationAndBiconnected();
    auto bicomponents_sub = G.GetBiconnectedComponents();
    cout << "SUBSZ: " << bicomponents_sub.size() << endl;

    if (bicomponents_sub.size() == 1) { // X - r is 2-connected
        vector<int> xy = c_
    } else { // not 2-connected => use Lemma 4

    }*/
    // #########################################
    return -1;
}