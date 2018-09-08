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

    // Create graph based on list of edges.
    MaxCutGraph(const vector<pair<int,int>> &elist);

    // Create induced subgraph
    MaxCutGraph(const MaxCutGraph& source, const vector<int>& subset);

    int GetNumNodes() const { return num_nodes; }

    int GetRealNumNodes() { return GetAllExistingNodes().size(); }
    int GetRealNumEdges() { return GetAllExistingEdges().size(); }

    void AddEdge(int a, int b) {
        if(edge_exists_lookup[make_pair(a,b)] || a == b) {
            //cout << "Warning: Edge added already or loop: " << a << " " << b << ". Ignored." << endl;
            return;
        }

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
    int Degree(int node) { return GetAdjacency(node).size(); }

    // https://github.com/niklasb/tcr/blob/master/graphentheorie/arti-bruecken.cpp
    void ComputeArticulationAndBiconnected();

    void CalculateSingleSourceDistance(int source);

    vector<int> GetConnectedComponentOf(int node);
    vector<vector<int>> GetAllConnectedComponents();

    bool DoesDisconnect(vector<int> selection_rem);

    bool Breaks2Connected(vector<int> selection_rem);

    int GetSingleSourceDistance(int dest) { return single_source_dist[dest]; }

    // Return: {r,...,dest}
    vector<int> GetSingleSourcePathFromRoot(int dest);

    void RemoveNode(int node);
    // Does not add the previously removed edges with the RemoveNode function!
    void ReAddNode(int node);
    void RemoveEdgesBetween(int nodex, int nodey);
    void RemoveEdgesInComponent(const vector<int> &component);

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

    // Due to the possibility of components disappearing, we need to do this. Otherwise, once a whole
    // component is deleted, we gain 1/4.0 in EE. In itself, this is good, as a higher bound is achieved.
    // Although... The kernelization rules' invariants break in those cases.
    void SaveNumOfComponentsForEdwardsErdosBound();

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

    // Returns a vector of X that satisfy rule 8 from https://arxiv.org/abs/1212.6848  
    vector<vector<int>> GetAllR8Candidates(const unordered_map<int,bool>& preset_is_external = {});
    void ApplyR8Candidate(const vector<int> &clique, double &cut_change);

    // Returns a vector of (x,(pair1, pair2)) where x is the shared vertex of triangles (x,pair1.first,pair1.second),
    // (x,pair2.first,pair2.second).
    vector<pair<int,vector<pair<int,int>>>> GetAllR9Candidates();
    void ApplyR9Candidate(const pair<int,vector<pair<int,int>>> &candidates, double &cut_change);

    // Returns a vector of (C, X) pairs that all satisfy rule 9 from https://arxiv.org/abs/1212.6848 
    // Warning! X >= C/2, therefore, deletion of some vertices in X is necessary when applying the rule.
    vector<pair<vector<int>, vector<int>>> GetAllR9XCandidates();
    void ApplyR9XCandidate(const pair<vector<int>, vector<int>> &candidate, double &cut_change);

    // Returns a vector of (u, (x,y)) satisfying rule 10 from https://arxiv.org/abs/1212.6848 
    vector<tuple<bool, int, int, int>> GetAllR10Candidates(const unordered_map<int,bool>& preset_is_external = {});
    void ApplyR10Candidate(const tuple<bool, int, int, int> &candidate, double &k);

    // Returns a vector of 5-tuples a' b c d d'
    vector<tuple<int,int,int,int,int>> GetAllR10ASTCandidates(const unordered_map<int,bool>& preset_is_external = {});
    void ApplyR10ASTCandidate(const tuple<int,int,int,int,int>& candidate, double &cut_change);

    // Returns a vector of odd cliques with less than ceil(n/2) external vertices.
    vector<vector<int>> GetS2Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {});
    void ApplyS2Candidate(const vector<int>& clique, double &cut_change, const unordered_map<int,bool>& preset_is_external = {});

    // Get "almost cliques" (missing one edge) with at least one internal vertex.
    vector<vector<int>> GetS3Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {});
    void ApplyS3Candidate(const vector<int> &clique, double &cut_change, const unordered_map<int,bool>& preset_is_external = {});

    vector<tuple<bool,int,int,int,int>> GetAllS4Candidates(const unordered_map<int,bool>& preset_is_external = {});
    void ApplyS4Candidate(tuple<bool,int,int,int,int> &candidate, double &cut_change);
    
    vector<tuple<int,int,int,int>> GetAllS5Candidates(const unordered_map<int,bool>& preset_is_external = {});
    void ApplyS5Candidate(const tuple<int,int,int,int>& candidate, double &cut_change);


    vector<pair<int,int>> GetAllS6Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {});
    void ApplyS6Candidate(const pair<int,int> &candidate, double &cut_change, const unordered_map<int,bool>& preset_is_external = {});

    void ExecuteExhaustiveKernelization();
    void ExecuteExhaustiveKernelizationExternalsSupport(const unordered_map<int,bool>& preset_is_external);

    vector<int> GetAClique(const int min_size, const int max_runs, const bool make_maximum = false);

    // WRONG AS OF NOW, AS IT WORKS WITH VERTEX COVER, BUT SHOULD USE EDGE COVER
    vector<vector<int>> DecomposeIntoCliques();

    // As of now, doesn't print the actual nodes. Some single nodes available.
    void PrintGraph(std::ostream& out);
    string PrintDegrees(const unordered_map<int,bool>& preset_is_external = {});

    // Get cut size according to 0/1 coloring of nodes.
    int GetCutSize(const vector<int> &grouping);

    // Pregroup[i] in {-1,0,1}. -1 = no predefined group, 0/1 group 0 or 1.
    pair<int, vector<int>> ComputeLocalSearchCut(const vector<int> pregroup = {});

    pair<int, vector<int>> ComputeMaxCutHeuristically();

    vector<vector<int>> GetCliquesWithAtLeastOneInternal();

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

    int num_nodes;

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