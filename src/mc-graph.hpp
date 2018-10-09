#pragma once

#include "./utils.hpp"

#include <bits/stdc++.h>
using namespace std;

// Definition articulation node:
// Its removal parts the graph in at least two non-empty graphs.

// Undirected, Unweighted.
// pointers not allowed in this class due to usage of default copy !!!!
// All node indices are 0-based.
class MaxCutGraph {
public:
    MaxCutGraph();

    MaxCutGraph(int n, int m);

    // File based input is 1-index based. Transformed to 0-based while reading in.
    MaxCutGraph(const string path);

    // Create graph based on list of edges.
    MaxCutGraph(const vector<pair<int,int>> &elist, int n = 0);

    // Creates induced subgraph.
    MaxCutGraph(const MaxCutGraph& source, const vector<int>& subset);

    //  Resets computed articulation vertices, biconnected components, bridges.
    void ResetComputedTopology();

    // Adds an edge between a and b with a weight.
    void AddEdge(int a, int b, int weight = 1);


    /**
     * Often used graph functionalities. All const.
     **/
    int GetNumNodes() const { return num_nodes; }
    int GetRealNumNodes() const { return GetAllExistingNodes().size(); }
    int GetRealNumEdges() const { return GetAllExistingEdges().size(); }
    vector<int> GetAllExistingNodes() const;
    const vector<int>& GetAdjacency(int node) const { return g_adj_list.at(node); }
    bool AreAdjacent(int n1, int n2) const { return MapEqualCheck(edge_exists_lookup, MakeEdgeKey(n1,n2), true); }
    int Degree(int node) const { return GetAdjacency(node).size(); }
    vector<int> GetConnectedComponentOf(int node) const;
    vector<vector<int>> GetAllConnectedComponents() const;
    bool DoesDisconnect(const vector<int>& selection_rem) const;
    vector<pair<int,int>> GetAllExistingEdges() const;
    bool IsClique(const vector<int>& vertex_set) const;
    double GetEdwardsErdosBound() const;
    ///////////////////////////////////////////////////



    /**
     * Various topology functions.
     **/
    // https://github.com/niklasb/tcr/blob/master/graphentheorie/arti-bruecken.cpp
    void ComputeArticulationAndBiconnected();
    vector<int> GetArticulationNodes();
    bool IsArticulation(int node);
    vector<vector<int>> GetBiconnectedComponents();
    bool IsBridgeBetween(int nodeA, int nodeB);
    void CalculateSingleSourceDistance(int source);
    bool Breaks2Connected(vector<int> selection_rem);
    int GetSingleSourceDistance(int dest) const { return single_source_dist.at(dest); }
    // Return: {r,...,dest}
    vector<int> GetSingleSourcePathFromRoot(int dest) const;
    // Returns -1 if no component of size >= 1 was found.
    tuple<vector<int>, int> GetLeafBlockAndArticulation(bool print_components = false);
    bool IsCliqueForest(); // all of G


    /**
     * Core functions for modifying graph. In a perfect world,
     * all modifications should happen with these functions.
     **/
    void RemoveNode(int node);
    // Does not add the previously removed edges from the RemoveNode function!
    void ReAddNode(int node);
    void RemoveEdgesBetween(int nodex, int nodey);
    void RemoveEdgesInComponent(const vector<int> &component);

    
    /**
     * OneWay reduction rule applications from paper showcasting linear kernel computation. (c_with_v - v) is clique
     **/
    void ApplyOneWayRule3(const vector<int>& c_with_v, const int v);
    void ApplyOneWayRule5(const vector<int>& c_with_v, const int v);
    void ApplyOneWayRule6(const vector<int>& induced_2path);
    void ApplyOneWayRule7(const vector<int>& c, const int v, const int b);


    /**
     * Utilities for onway rules.
     **/
    vector<int> GetInducedPathByLemma2(const vector<int>& component, int r);
    vector<int> FindInducedPathForRule6(const vector<int>& component, const int r);
    void CalculateLemma4DFSTree(int root, int ui);
    int GetDfsTreeDepthFromRoot(int node) { return lemma4_dfs_tree_depth[node]; }

    
    /**
     *  Functions regarding the marked vertex set such that G - paper_S is a clique forest
     **/
    vector<int> GetMarkedVerticesByOneWayRules() const;
    void SetMarkedVertices(const vector<int>& S) { paper_S = S; }
    void ReduceMarksetVertexSet();
    int Algorithm2MarkedComputation();
    int Algorithm3MarkedComputation_Randomized();


    /**
     * Brute force max cut computations (optimal) on S + extension to G - S (clique forest)
     **/
    // Makes assumption(!) that S is subset of G. We can't check this, as this is time-critical.
    tuple<int, vector<int>> MaxCutExtension(const vector<int>& S, const vector<int>& S_color);
    // Bruteforces coloring of S and does MaxCutExtension to induce a maxcut result for remainder of graph.
    int ComputeOptimalColoringBruteforce(const vector<int>& S);



    /**
     *  Two way reduction rules.
     **/
    // Returns a vector of X that satisfy rule 8 from https://arxiv.org/abs/1212.6848  
    vector<vector<int>> GetAllR8Candidates(const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyR8Candidate(const vector<int> &clique);

    // Returns a vector of (x,(pair1, pair2)) where x is the shared vertex of triangles (x,pair1.first,pair1.second),
    // (x,pair2.first,pair2.second).
    vector<pair<int,vector<pair<int,int>>>> GetAllR9Candidates() const;
    void ApplyR9Candidate(const pair<int,vector<pair<int,int>>> &candidates);

    // Returns a vector of (C, X) pairs that all satisfy rule 9 from https://arxiv.org/abs/1212.6848 
    // Warning! X >= C/2, therefore, deletion of some vertices in X is necessary when applying the rule.
    vector<pair<vector<int>, vector<int>>> GetAllR9XCandidates() const;
    void ApplyR9XCandidate(const pair<vector<int>, vector<int>> &candidate);

    // Returns a vector of (u, (x,y)) satisfying rule 10 from https://arxiv.org/abs/1212.6848 
    vector<tuple<bool, int, int, int>> GetAllR10Candidates(const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyR10Candidate(const tuple<bool, int, int, int> &candidate);

    // Returns a vector of 5-tuples a' b c d d'
    vector<tuple<int,int,int,int,int>> GetAllR10ASTCandidates(const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyR10ASTCandidate(const tuple<int,int,int,int,int>& candidate);

    // Returns a vector of odd cliques with less than ceil(n/2) external vertices.
    vector<vector<int>> GetS2Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyS2Candidate(const vector<int>& clique, const unordered_map<int,bool>& preset_is_external = {});

    // Get "almost cliques" (missing one edge) with at least one internal vertex.
    vector<vector<int>> GetS3Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyS3Candidate(const vector<int> &clique, const unordered_map<int,bool>& preset_is_external = {});

    vector<tuple<bool,int,int,int,int>> GetAllS4Candidates(const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyS4Candidate(tuple<bool,int,int,int,int> &candidate);
    
    vector<tuple<int,int,int,int>> GetAllS5Candidates(const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyS5Candidate(const tuple<int,int,int,int>& candidate);

    vector<pair<int,int>> GetAllS6Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyS6Candidate(const pair<int,int> &candidate, const unordered_map<int,bool>& preset_is_external = {});


    /**
     *  Special kernelization rules: weighted <-> unweighted
     **/
    bool CandidateSatisfiesSpecialRule1(const tuple<int,int,int,int> &candidate) const;
    bool CandidateSatisfiesSpecialRule2(const tuple<int,int,int> &candidate) const;
    // 3-paths. Used for handling integer weights > 1.
    vector<tuple<int,int,int,int>> GetAllSpecialRule1Candidates() const;
    // 2-paths. Used for handlging integer weights < 0.
    vector<tuple<int,int,int>> GetAllSpecialRule2Candidates() const;

    // Returns all edges with integer weight > 1.
    vector<pair<int,int>> GetAllRevSpecialRule1Candidates() const;
    // Returns all edges with integer weight < 0.
    vector<pair<int,int>> GetAllRevSpecialRuke2Candidates() const;

    /**
     *  Transformations of all G / create G' with certain properties.
     **/
    double ExecuteLinearKernelization();
    void ExecuteExhaustiveKernelization();
    void ExecuteExhaustiveKernelizationExternalsSupport(const unordered_map<int,bool>& preset_is_external);
    void MakeUnweighted();
    void MakeWeighted();
    

    /**
     *  Heuristic Max-Cut computations
     **/
    // Pregroup[i] in {-1,0,1}. -1 = no predefined group, 0/1 group 0 or 1.
    pair<int, vector<int>> ComputeLocalSearchCut(const vector<int> pregroup = {}) const;
    pair<int, vector<int>> ComputeMaxCutWithMQLib() const;



    /**
     * Misc.
     **/
    vector<vector<int>> GetCliquesWithAtLeastOneInternal() const;
    vector<int> GetAClique(const int min_size, const int max_runs, const bool make_maximum = false) const;
    void PrintGraph(std::ostream& out) const;
    string PrintDegrees(const unordered_map<int,bool>& preset_is_external = {}) const;
    // Get cut size according to 0/1 coloring of nodes. grouping is a 0-1 vector. Vertex x is colored by grouping[x]. 
    int GetCutSize(const vector<int> &grouping) const;
    double GetInflictedCutChangeToKernelized() const { return inflicted_cut_change_to_kernelized; }

private:
    inline long long MakeEdgeKey(int a, int b) const { return a * (long long)(num_nodes+1) + b; }

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

    vector<int> lemma4_dfs_tree_parent;
    vector<int> lemma4_dfs_tree_depth;
    int lemma4_dfs_tree_ui;

    int num_nodes;

    bool bicomponents_computed = false;
    bool articulations_computed = false;
    bool bridges_computed = false;

    // Invariant: no edges should exist from/to removed_nodes.
    unordered_map<int, bool> removed_node;

    map<pair<int,int>, bool> is_bridge_between;

    unordered_map<long long, int> edge_weight;
    unordered_map<long long, bool> edge_exists_lookup;
    vector<vector<int>> g_adj_list;
    vector<vector<int>> biconnected_components;
    vector<int> paper_S;

    vector<bool> is_articulation;

    // Used inside CalculateSingleSourceDistance
    vector<int> single_source_dist;
    vector<int> single_source_prev;

    // Used by MaxCutExtension
    vector<int> computed_maxcut_coloring;

    double inflicted_cut_change_to_kernelized; // absolute! beta(G') = beta(G) + inflicted_cut_change_to_kernelized
};