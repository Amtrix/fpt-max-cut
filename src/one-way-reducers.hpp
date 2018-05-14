#pragma once

#include "mc-graph.hpp"

int TryOneWayReduce(MaxCutGraph& G, int &k) {
    // First, find leaf block.
    int selected_block_dx = -1;
    auto bicomponents = G.GetBiconnectedComponents();
    for (unsigned int i = 0; i < bicomponents.size(); ++i) {
        const auto& component = bicomponents[i];
        if (component.size() == 1) continue; // ?????

        int articulation_count = 0;
        for (const int node : component) articulation_count += G.IsArticulation(node);
        if (articulation_count <= 1 && (selected_block_dx == -1 || component.size() < bicomponents[selected_block_dx].size()))
            selected_block_dx = i;
    }

    const auto& component = bicomponents[selected_block_dx];

    // Find r as outlined in paper. r is cut vertex if exists, or arbitrary vertex if not.
    int r = 0;
    for (unsigned int i = 1; i < component.size(); ++i)
        if (G.IsArticulation(component[i]))
            r = i;
    r = component[r];

    if (G.IsClique(component)) { // Apply RULE 5
        G.ApplyRule5(component, r);
        if (component.size() % 2) k--; // We do even instead odd, because here C = component U r
        return 5;
    }

    return -1;
}