#pragma once

#include "utils.hpp"
#include <vector>
#include <unordered_map>
#include <map>
#include <queue>
#include <functional>
using namespace std;

enum class RuleIds : int {
    SpecialRule1,
    SpecialRule2,
    RevSpecialRule1,
    RevSpecialRule2,
    Rule8, Rule9, Rule9X, Rule10, Rule10AST, RuleS2, RuleS3, RuleS4, RuleS5, RuleS6
};

extern const map<RuleIds, string> kRuleDescriptions;

extern const map<RuleIds, string> kRuleNames;

extern const vector<RuleIds> kAllRuleIds;

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
    MaxCutGraph(const vector<tuple<int,int,int>> &elist, int n = 0);

    // Creates induced subgraph.
    MaxCutGraph(const MaxCutGraph& source, const vector<int>& subset);

    //  Resets computed articulation vertices, biconnected components, bridges.
    void ResetComputedTopology();


    /**
     * Core functions for modifying graph. In a perfect world,
     * all modifications should happen with these functions.
     **/
    // Adds an edge between a and b with a weight.
    void SetNumNodes(int _num_nodes);
    void AddEdge(int a, int b, int weight = 1, bool inc_weight_on_double = true);
    void RemoveNode(int node);
    // Does not add the previously removed edges from the RemoveNode function!
    void ReAddNode(int node);
    int CreateANode();
    void RemoveEdgesBetween(int nodex, int nodey);
    void RemoveEdgesInComponent(const vector<int> &component);



    /**
     * Often used graph functionalities. All const.
     **/
    int GetNumNodes() const { return num_nodes; }
    int GetRealNumNodes() const { return GetAllExistingNodes().size(); }
    int GetRealNumEdges() const { return GetAllExistingEdges().size(); }
    vector<int> GetAllExistingNodes() const;
    const vector<int>& GetAdjacency(int node) const { return g_adj_list.at(node); }
    int GetEdgeWeight(const pair<int,int> &e) const { return edge_weight.at(MakeEdgeKey(e)); }
    bool AreAdjacent(int n1, int n2) const { return MapEqualCheck(edge_exists_lookup, MakeEdgeKey(n1,n2), true); }
    int Degree(int node) const { return GetAdjacency(node).size(); }
    vector<int> GetConnectedComponentOf(int node, vector<bool>& visited) const;
    vector<int> GetConnectedComponentOf(int node) const;
    vector<vector<int>> GetAllConnectedComponents() const;
    bool DoesDisconnect(const vector<int>& selection_rem) const;
    vector<pair<int,int>> GetAllExistingEdges() const;
    bool IsClique(const vector<int>& vertex_set) const;
    double GetEdwardsErdosBound() const;
    int CountExternalVertices(const vector<int> &vertex_set) const; // G[vertex_set] considered.
    string GetGraphNaming() const;
    void SetGraphNaming(const string name) { graph_naming = name; }
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
    vector<vector<int>> GetAllR8Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyR8Candidate(const vector<int> &clique);

    // Returns a vector of (x,(pair1, pair2)) where x is the shared vertex of triangles (x,pair1.first,pair1.second),
    // (x,pair2.first,pair2.second).
    vector<pair<int,vector<pair<int,int>>>> GetAllR9Candidates(const bool break_on_first = false) const;
    bool ApplyR9Candidate(const pair<int,vector<pair<int,int>>> &candidates);

    // Returns a vector of (C, X) pairs that all satisfy rule 9 from https://arxiv.org/abs/1212.6848 
    // Warning! X >= C/2, therefore, deletion of some vertices in X is necessary when applying the rule.
    vector<pair<vector<int>, vector<int>>> GetAllR9XCandidates(const bool break_on_first = false) const;
    void ApplyR9XCandidate(const pair<vector<int>, vector<int>> &candidate);

    // Returns a vector of (u, (x,y)) satisfying rule 10 from https://arxiv.org/abs/1212.6848 
    vector<tuple<bool, int, int, int>> GetAllR10Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyR10Candidate(const tuple<bool, int, int, int> &candidate);

    // Returns a vector of 5-tuples a' b c d d'
    vector<tuple<int,int,int,int,int>> GetAllR10ASTCandidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyR10ASTCandidate(const tuple<int,int,int,int,int>& candidate);

    // Returns a vector of cliques with less than ceil(n/2) external vertices.
    vector<int> GetS2Candidates(const bool consider_dirty_only = true, const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {});
    bool ApplyS2Candidate(const int root, const unordered_map<int,bool>& preset_is_external = {});

    // Get "almost cliques" (missing one edge) with at least one internal vertex.
    vector<vector<int>> GetS3Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyS3Candidate(const vector<int> &clique, const unordered_map<int,bool>& preset_is_external = {});

    vector<tuple<bool,int,int,int,int>> GetAllS4Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    void ApplyS4Candidate(tuple<bool,int,int,int,int> &candidate);
    
    vector<tuple<int,int,int,int>> GetAllS5Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
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
    // Applications.
    bool ApplySpecialRule1(const tuple<int,int,int,int> &candidate);
    bool ApplySpecialRule2(const tuple<int,int,int> &candidate);
    // Returns all edges with integer weight > 1.
    vector<pair<int,int>> GetAllRevSpecialRule1Candidates() const;
    // Returns all edges with integer weight < 0.
    vector<pair<int,int>> GetAllRevSpecialRule2Candidates() const;
    //Applications.
    bool ApplyRevSpecialRule1(const pair<int,int> &candidate);
    bool ApplyRevSpecialRule2(const pair<int,int> &candidate);


    /**
     *  Transformations of all G / create G' with certain properties.
     **/
    bool PerformKernelization(const RuleIds rule_id, const unordered_map<int,bool>& preset_is_external = {});
    void MakeUnweighted();
    void MakeWeighted();
    double ExecuteLinearKernelization();
    void ExecuteExhaustiveKernelizationExternalsSupport(const unordered_map<int,bool>& preset_is_external);
    

    /**
     *  Heuristic Max-Cut computations
     **/
    // Pregroup[i] in {-1,0,1}. -1 = no predefined group, 0/1 group 0 or 1.
    pair<int, vector<int>> ComputeLocalSearchCut(const vector<int> pregroup = {}) const;
    // max_exec_time in seconds.
    pair<int, vector<int>> ComputeMaxCutWithMQLib(const double max_exec_time = 0.2) const;



    /**
     * Misc.
     **/
    vector<vector<int>> GetCliquesWithAtLeastOneInternal() const;
    vector<int> GetAClique(const int min_size, const int max_runs, const bool make_maximum = false) const;
    void PrintGraph(std::ostream& out) const;
    string PrintDegrees(const unordered_map<int,bool>& preset_is_external = {}) const;
    void PrintReductionsUsage() const;
    int GetRuleUsage(RuleIds rule) const;
    int GetRuleChecks(RuleIds rule) const;
    vector<int> GetUsageVector() const;
    // Get cut size according to 0/1 coloring of nodes. grouping is a 0-1 vector. Vertex x is colored by grouping[x]. 
    int GetCutSize(const vector<int> &grouping) const;
    double GetInflictedCutChangeToKernelized() const { return inflicted_cut_change_to_kernelized; }
    void SetMixingId(int id) { mixing_id = id; }
    int GetMixingId() const { return mixing_id; }

    int info_mult_edge = 0;

    struct {
        int S2 = 0;
    } CURRENT_TIMESTAMPS;

    void ResetTimestamps() {
        CURRENT_TIMESTAMPS.S2 = 0;
        auto current_v = GetAllExistingNodes();
        for (auto node : current_v)
            UpdateVertexTimestamp(node);
    }

private:
    constexpr static long long kMaxNumNodes = 1000000000LL;
    inline long long MakeEdgeKey(int a, int b) const { return a * kMaxNumNodes + b; }
    inline long long MakeEdgeKey(const pair<int,int> &e) const { return MakeEdgeKey(e.first, e.second); }

    void UpdateVertexTimestamp(int node) {
        custom_assert(current_timestamp[node] != -1);

        current_timestamp[node] = current_kernelization_time;
        vertex_timetable_pq.push(make_pair(current_kernelization_time, node));

        current_kernelization_time += 1;
    }

    vector<int> GetVerticesAfterTimestamp(int timestamp, bool include_neighbhors = false) {
        vector<pair<int,int>> selected;
        while (!vertex_timetable_pq.empty()) {
            auto u = vertex_timetable_pq.top();
            vertex_timetable_pq.pop();
            if (current_timestamp[u.second] != u.first) continue; // has been made invalid.

            if (u.first < timestamp) {
                vertex_timetable_pq.push(u); // return the one we don't want.
                break;
            }
            
            selected.push_back(u);
        }
        
        unordered_map<int,bool> visi;
        if (!include_neighbhors) {
            for (auto entry : selected) {
                assert(!visi[entry.second]);
                visi[entry.second] = true;
            }
        } else {
            for (auto entry : selected) {
                visi[entry.second] = true;
                auto adj = GetAdjacency(entry.second);
                for (auto w : adj)
                    visi[w] = true;
            }
        }

        vector<int> ret;
        for (auto it : visi)
            ret.push_back(it.first);
        
        for (auto entry : selected) {
            if (current_timestamp[entry.second] == entry.first)
                vertex_timetable_pq.push(entry); // put back as it was not changed nor requested to be removed.
        }

        sort(ret.begin(), ret.end()); // insignificant, but allows us to check if result remains the same compared to checking all vertices -- UNDER CERTAIN CIRCUMSTANCES! (if the ignored ones do not influence the result at all!).
        return ret;
    }

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

    int num_nodes = 0;

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

    // Following is used to track timestamps on vertices. REASON: We don't want to applicability checks on same components multiple times.
    priority_queue<pair<int,int>> vertex_timetable_pq;
    vector<int> current_timestamp; // will hold the most recent timestamp for each vertex. Used to identify outdated values in pq!
    int current_kernelization_time = 1;

    

    double inflicted_cut_change_to_kernelized = 0; // absolute! beta(G') = beta(G) + inflicted_cut_change_to_kernelized
    unordered_map<RuleIds, int> rules_usage_count;
    unordered_map<RuleIds, int> rules_check_count;

    string graph_naming;
    int mixing_id;
};