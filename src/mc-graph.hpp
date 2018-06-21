#pragma once

#include "./utils.hpp"

#include <bits/stdc++.h>
using namespace std;

// Definition articulation node:
// Its removal parts the graph in at least two non-empty graphs.

// Undirected, Unweighted.
// pointers not allowed due to usage of default copy in benchmark.cpp !!!!
// All node indices are 0-based.
class MaxCutGraph {
public:
    MaxCutGraph();

    MaxCutGraph(int n, int m);

    // File based input is 1-index based. Transformed to 0-based while reading in.
    MaxCutGraph(const string path);

    // Create induced subgraph
    MaxCutGraph(const MaxCutGraph& source, const vector<int>& subset);

    int GetNumNodes() const { return num_nodes; }
//    int GetNumEdges() const { return num_edges; }

    int GetRealNumNodes() { return GetAllExistingNodes().size(); }
    int GetRealNumEdges() { return GetAllExistingEdges().size(); }

    void AddEdge(int a, int b) {
        assert(edge_exists_lookup[make_pair(a,b)] == false);

        g_adj_list[a].push_back(b);
        g_adj_list[b].push_back(a);
        edge_exists_lookup[make_pair(a,b)] = true;
        edge_exists_lookup[make_pair(b,a)] = true;

        bicomponents_computed = false;
        articulations_computed = false;
        bridges_computed = false;
    }

    const vector<int>& GetAdjacency(int node) const { return g_adj_list[node]; }
    bool AreAdjacent(int n1, int n2) { return edge_exists_lookup[make_pair(n1,n2)]; }

    // https://github.com/niklasb/tcr/blob/master/graphentheorie/arti-bruecken.cpp
    void ComputeArticulationAndBiconnected();

    void CalculateSingleSourceDistance(int source);

    vector<vector<int>> GetAllConnectedComponents();

    bool DoesDisconnect(vector<int> selection_rem);

    bool Breaks2Connected(vector<int> selection_rem);

    int GetSingleSourceDistance(int dest) { return single_source_dist[dest]; }

    // Return: {r,...,dest}
    vector<int> GetSingleSourcePathFromRoot(int dest);

    void RemoveNode(int node);

    vector<int> GetAllExistingNodes();

    vector<pair<int,int>> GetAllExistingEdges();

    bool IsClique(const vector<int>& vertex_set);

    // (c_with_v - v) is clique
    void ApplyRule3(const vector<int>& c_with_v, const int v);

    void ApplyRule5(const vector<int>& c_with_v, const int v);

    void ApplyRule6(const vector<int>& induced_2path);

    void ApplyRule7(const vector<int>& c, const int v, const int b);

    vector<int> GetInducedPathByLemma2(const vector<int>& component, int r);

    vector<int> FindInducedPathForRule6(const vector<int>& component, const int r);

    void CalculateLemma4DFSTree(int root, int ui);

    int GetDfsTreeDepthFromRoot(int node) { return dfs_tree_depth[node]; }

    vector<int> GetArticulationNodes();

    bool IsArticulation(int node) {
        if (!articulations_computed) ComputeArticulationAndBiconnected();

        return is_articulation[node];
    }

    bool IsBridgeBetween(int nodeA, int nodeB);

    bool IsCliqueForest();

    vector<vector<int>> GetBiconnectedComponents();

    vector<int> GetMarkedVerticesByOneWayRules() const;

    void SetMarkedVertices(const vector<int>& S) { paper_S = S; }

    double GetEdwardsErdosBound();

    // Returns -1 if no component of size >= 1 was found.
    tuple<vector<int>, int> GetLeafBlockAndArticulation(bool print_components = false);

    // Makes assumption(!) that S is subset of G. We can't check this, as this is time-critical.
    tuple<int, vector<int>> MaxCutExtension(const vector<int>& S, const vector<int>& S_color);

    void ReduceMarksetVertexSet();

    int Algorithm2MarkedComputation();

    int Algorithm3MarkedComputation_Randomized();

    int ComputeOptimalColoringBruteforce(const vector<int>& S);

    // TODO: A more efficient way to color the graph.
    int ComputeOptimalColoring(const vector<int>& S, const vector<int>& S_color = {});

    vector<int> GetMaxCutColoring() { return computed_maxcut_coloring; }

    // As of now, doesn't print the actual nodes. Some single nodes available.
    void PrintGraph(std::ostream& out);

private:
    enum class tarjan_dfs_data_type {
        FIRST_VISIT,
        REVISIT
    };

    struct tarjan_dfs_data {
        int depth;
        int node;
        tarjan_dfs_data_type type;
        unsigned int last_dx;
    };

    // Recursive! MaxCutGraphs generally small, so stack size shouldn't be a problem.
    void CalculateLemma4DFSTree_(int node);

    vector<int> dfs_tree_parent;
    vector<int> dfs_tree_depth;
    int dfs_tree_ui;

    int num_nodes, num_edges;

    bool bicomponents_computed = false;
    bool articulations_computed = false;
    bool bridges_computed = false;

    // Invariant: no edges should exist from/to removed_nodes.
    unordered_map<int, bool> removed_node;

    map<pair<int,int>, bool> is_bridge_between;

    map<pair<int,int>, bool> edge_exists_lookup; // IMPROVE TO O(1)!!!!
    vector<vector<int>> g_adj_list;
    vector<vector<int>> biconnected_components;
    vector<int> paper_S;

    vector<bool> is_articulation;

    // Used inside CalculateSingleSourceDistance
    vector<int> single_source_dist;
    vector<int> single_source_prev;

    // Used by MaxCutExtension
    vector<int> computed_maxcut_coloring;
};