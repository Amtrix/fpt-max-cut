#pragma once

#define LIMIT_NUM_NODES 500000
#define LIMIT_ABS_WEIGHT 1000000001
#define TRANSFORM_SPLITTER false
#define SCALED_FROM 100000LL

#include "input-parser.hpp"
#include "./src/output-filter.hpp"
#include "utils.hpp"
#include <vector>
#include <unordered_map>
#include <map>
#include <queue>
#include <functional>

#include "maxcut-localsolver.hpp"
#include <heuristics/qubo/glover1998a.h>
#include <heuristics/maxcut/burer2002.h>
using namespace std;

typedef long long EdgeWeight;

enum class RuleIds : int {
    SpecialRule1,
    SpecialRule2,
    RevSpecialRule1,
    RevSpecialRule2,
    SpecialRule2Signed,
    Rule8, Rule9, Rule9X, Rule10, Rule10AST, RuleS2, RuleS3, RuleS4, RuleS5, RuleS6, Rule8Signed, Rule8SpecialCase, RuleS2SpecialCase, MegaRule,
    RuleS2Weighted, RuleWeightedTriag
};

extern const map<RuleIds, string> kRuleDescriptions;

extern const map<RuleIds, string> kRuleNames;

extern const vector<RuleIds> kAllRuleIds;

class Burer2002Callback : public MaxCutCallback {
public:
    Burer2002Callback(double total_allowed_time_, InputParser *input_parser_, const string graph_name_, int mixingid_, int num_nodes_, int num_edges_, double added_preprocess_time_, int cutadd_, string sfxout_) :
                total_allowed_time(total_allowed_time_),
                input_parser(input_parser_),
                graph_name(graph_name_),
                mixingid(mixingid_),
                num_nodes(num_nodes_),
                num_edges(num_edges_),
                added_preprocess_time(added_preprocess_time_),
                cutadd(cutadd_),
                sfxout(sfxout_)
        {
            if (input_parser->cmdOptionExists("-exact-early-stop-v")) {
                int v_limit = stoi(input_parser->getCmdOption("-exact-early-stop-v"));
                if (v_limit < num_nodes)
                    added_preprocess_time = 1e9;
            }
        }

    bool Report(const MaxCutSimpleSolution& sol, bool /* newBest */, double runtime) {
        runtime += added_preprocess_time;
        OutputLiveMaxcut(*input_parser, graph_name, mixingid, num_nodes, num_edges, runtime, sol.get_weight() + cutadd, sfxout);

        if (runtime > total_allowed_time) {
            timelimit_exceeded = true;
            return false;
        }
        
        if (sol.get_weight() + cutadd == terminating_cut_size)
            return false;

        return true;
    }

    bool Report(const MaxCutSimpleSolution& sol, bool newBest, double runtime, int /* iter */) {
        return Report(sol, newBest, runtime);
    }

    void SetTerminatingCutSize(int cutsz) {
        terminating_cut_size = cutsz;
    }

    bool HasExceededTimelimit() {
        return timelimit_exceeded;
    }
    
 private:
    double total_allowed_time;
    InputParser *input_parser;
    string graph_name;
    int mixingid;
    int num_nodes;
    int num_edges;
    double added_preprocess_time;
    int cutadd;
    string sfxout;
    int terminating_cut_size = -1;
    bool timelimit_exceeded = false;
};

// Definition articulation node:
// Its removal parts the graph in at least two non-empty graphs.

// Undirected, Unweighted.
// pointers not allowed in this class due to usage of default copy !!!!
// All node indices are 0-based.
class MaxCutGraph {
public:
    
    MaxCutGraph();

    MaxCutGraph(int n);

    // File based input is 1-index based. Transformed to 0-based while reading in.
    MaxCutGraph(const string path);

    // Create graph based on list of edges.
    MaxCutGraph(const vector<pair<int,int>> &elist, int n = 0);
    MaxCutGraph(const vector<tuple<int,int,EdgeWeight>> &elist, int n = 0);

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
    void AddEdge(int a, int b, EdgeWeight weight = 1, bool inc_weight_on_double = true);
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
    int GetRealNumNodes() const {
        const auto& nodes = GetAllExistingNodes();
        int ret = nodes.size();
        for (auto node : nodes)
            if (Degree(node) == 0)
                ret--;
        return ret;
    }
    int GetRealNumEdges() const { return GetAllExistingEdges().size(); }
    vector<int> GetAllExistingNodes() const;
    const vector<int>& GetAdjacency(int node) const { return g_adj_list.at(node); }
    EdgeWeight GetEdgeWeight(const int edgekey) const { return edge_weight.at(edgekey); }
    EdgeWeight GetEdgeWeight(const pair<int,int> &e) const { return edge_weight.at(MakeEdgeKey(e)); }
    bool AreAdjacent(int n1, int n2) const { return MapEqualCheck(edge_exists_lookup, MakeEdgeKey(n1,n2), true); }
    int Degree(int node) const { return g_adj_list.at(node).size(); }
    vector<int> GetConnectedComponentOf(int node, vector<bool>& visited) const;
    vector<int> GetConnectedComponentOf(int node) const;
    vector<vector<int>> GetAllConnectedComponents() const;
    bool DoesDisconnect(const vector<int>& selection_rem) const;
    vector<pair<int,int>> GetAllExistingEdges() const;
    vector<tuple<int,int,EdgeWeight>> GetAllExistingEdgesWithWeights() const;
    bool IsClique(const vector<int>& vertex_set, const EdgeWeight verify_weight = 0) const;
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
    void UpdateMarkedVertices(const vector<int>& S) { if (S.size() < paper_S.size()) paper_S = S; }
    vector<int> GetMarkedVertexSet() const { return paper_S; }
    void ReduceMarksetVertexSet();
    int Algorithm2MarkedComputation();
    vector<int> Algorithm3MarkedComputation(const vector<int> = {});


    /**
     * Brute force max cut computations (optimal) on S + extension to G - S (clique forest)
     **/
    // Makes assumption(!) that S is subset of G. We can't check this, as this is time-critical.
    tuple<EdgeWeight, vector<int>> MaxCutExtension(const vector<int>& S, const vector<int>& S_color);
    // Bruteforces coloring of S and does MaxCutExtension to induce a maxcut result for remainder of graph.
    EdgeWeight ComputeOptimalColoringBruteforce(const vector<int>& S, const int break_after_time_sec = 1);
    tuple<EdgeWeight,double> GetMaxCutWithMarkedVertexSet(const int limit_S, const int limit_time_sec);


    /**
     *  Two way reduction rules.
     **/
    // Returns a vector of X that satisfy rule 8 from https://arxiv.org/abs/1212.6848  
    vector<vector<int>> GetR8Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {});
    bool ApplyR8Candidate(const vector<int> &clique);

    void ApplyMegaRuleCandidates(const bool break_on_first, const unordered_map<int,bool>& preset_is_external = {});

    // Returns a vector of (x,(pair1, pair2)) where x is the shared vertex of triangles (x,pair1.first,pair1.second),
    // (x,pair2.first,pair2.second).
    vector<pair<int,vector<pair<int,int>>>> GetR9Candidates(const bool break_on_first = false) const;
    bool ApplyR9Candidate(const pair<int,vector<pair<int,int>>> &candidates);

    // Returns a vector of (C, X) pairs that all satisfy rule 9 from https://arxiv.org/abs/1212.6848 
    // Warning! X >= C/2, therefore, deletion of some vertices in X is necessary when applying the rule.
    vector<pair<vector<int>, vector<int>>> GetR9XCandidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    bool ApplyR9XCandidate(const pair<vector<int>, vector<int>> &candidate);

    // Returns a vector of (u, (x,y)) satisfying rule 10 from https://arxiv.org/abs/1212.6848 
    vector<tuple<bool, int, int, int>> GetR10Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    bool ApplyR10Candidate(const tuple<bool, int, int, int> &candidate);

    // Returns a vector of 5-tuples a' b c d d'
    vector<tuple<int,int,int,int,int>> GetR10ASTCandidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    bool ApplyR10ASTCandidate(const tuple<int,int,int,int,int>& candidate);

    // Returns a vector of cliques with less than ceil(n/2) external vertices.
    vector<int> GetS2Candidates(const bool consider_dirty_only = true, const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {});
    bool ApplyS2Candidate(const int root, const unordered_map<int,bool>& preset_is_external = {});

    // Get "almost cliques" (missing one edge) with at least one internal vertex.
    vector<pair<int,int>> GetS3Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    bool ApplyS3Candidate(const pair<int,int>& candidate, const unordered_map<int,bool>& preset_is_external = {});

    vector<tuple<bool,int,int,int,int>> GetS4Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    bool ApplyS4Candidate(tuple<bool,int,int,int,int> &candidate);
    
    vector<tuple<int,int,int,int>> GetS5Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}, bool applynow = false);
    bool ApplyS5Candidate(const tuple<int,int,int,int>& candidate);

    vector<pair<int,int>> GetS6Candidates(const bool break_on_first = false, const unordered_map<int,bool>& preset_is_external = {}) const;
    bool ApplyS6Candidate(const pair<int,int> &candidate, const unordered_map<int,bool>& preset_is_external = {});


    vector<int> GetWeightedTriagCandidates();
    bool ApplyWeightedTriagCandidate(const int root);


    /**
     *  Special kernelization rules: weighted <-> unweighted
     **/
    bool CandidateSatisfiesSpecialRule1(const tuple<int,int,int,int> &candidate) const;
    bool CandidateSatisfiesSpecialRule2(const tuple<int,int,int> &candidate) const;
    // 3-paths. Used for handling integer weights > 1.
    //vector<tuple<int,int,int,int>> GetSpecialRule1Candidates() const;
    // 2-paths. Used for handlging integer weights < 0.
    vector<tuple<int,int,int>> GetSpecialRule2Candidates() const;
    // Applications.
    //bool ApplySpecialRule1(const tuple<int,int,int,int> &candidate);
    bool ApplySpecialRule2(const tuple<int,int,int> &candidate, const bool make_signed = false);
    // Returns all edges with integer weight > 1.
    vector<pair<int,int>> GetRevSpecialRule1Candidates() const;
    // Returns all edges with integer weight < 0.
    vector<pair<int,int>> GetRevSpecialRule2Candidates() const;
    //Applications.
    bool ApplyRevSpecialRule1(const pair<int,int> &candidate);
    bool ApplyRevSpecialRule2(const pair<int,int> &candidate);


    /**
     *  Transformations of all G / create G' with certain properties.
     **/
    bool PerformKernelization(const RuleIds rule_id, const unordered_map<int,bool>& preset_is_external = {});
    void MakeUnweighted();
    void MakeWeighted();
    void MakeSigned();
    double ExecuteLinearKernelization();
    void ExecuteExhaustiveKernelizationExternalsSupport(const unordered_map<int,bool>& preset_is_external);
    

    /**
     *  Heuristic Max-Cut computations
     **/
    // Pregroup[i] in {-1,0,1}. -1 = no predefined group, 0/1 group 0 or 1.
    pair<EdgeWeight, vector<int>> ComputeLocalSearchCut(const vector<int> pregroup = {}) const;
    // max_exec_time in seconds.
    pair<EdgeWeight, vector<int>> ComputeMaxCutWithMQLib(const double max_exec_time = 0.2, Burer2002Callback* callback = nullptr) const;



    /**
     * Misc.
     **/
    bool GraphIsValid() const { return graph_is_supported == true; }
    void MakeRandomVertexPermutation();
    vector<vector<int>> GetCliquesWithAtLeastOneInternal() const;
    vector<int> GetAClique(const int min_size, const int max_runs, const bool make_maximum = false) const;
    void PrintGraph(std::ostream& out, bool printweight = false) const;
    void PrintGraph(const std::string path, bool printweight = false) const;
    string PrintDegrees(const unordered_map<int,bool>& preset_is_external = {}) const;
    void PrintReductionsUsage() const;
    int GetRuleUsage(RuleIds rule) const;
    int GetRuleChecks(RuleIds rule) const;
    double GetRuleSpentTime(RuleIds rule) const;
    vector<int> GetUsageVector() const;
    // Get cut size according to 0/1 coloring of nodes. grouping is a 0-1 vector. Vertex x is colored by grouping[x]. 
    EdgeWeight GetCutSize(const vector<int> &grouping) const;
    double GetInflictedCutChangeToKernelized() const {
        if (fabs(inflicted_cut_change_to_kernelized) < 1e-9) return -1e-18;
        return inflicted_cut_change_to_kernelized;
    }
    void SetMixingId(int id) { mixing_id = id; }
    int GetMixingId() const { return mixing_id; }

    int info_mult_edge = 0;
    int info_self_loop_edge = 0;

    struct {
        int S2 = 0;
        int R8 = 0;
        int MRULE = 0;
    } CURRENT_TIMESTAMPS;

    void ResetTimestamps() {
        CURRENT_TIMESTAMPS.S2 = 0;
        CURRENT_TIMESTAMPS.R8 = 0;
        CURRENT_TIMESTAMPS.MRULE = 0;

        auto current_v = GetAllExistingNodes();
        for (auto node : current_v)
           UpdateVertexTimestamp(node);
    }



#ifdef LOCALSOLVER_EXISTS
    pair<EdgeWeight, vector<int>> ComputeMaxCutWithLocalsolver(const int max_exec_time = 1, LocalSolverCallback* callback = nullptr) const;
#endif

private:
    constexpr static long long kMaxNumNodes = 1000000000LL;
    inline long long MakeEdgeKey(int a, int b) const { return a * kMaxNumNodes + b; }
    inline long long MakeEdgeKey(const pair<int,int> &e) const { return MakeEdgeKey(e.first, e.second); }

    enum class TimestampType{
        DegreeDecrease = 1,
        DegreeIncrease = 2,
        Both = 3
    };

    
    template <class mark_type>
    void MarkDefinitelyNotInternal(const vector<int>& vertices, unordered_map<int, mark_type>& visi) const {
        for (auto root : vertices) { // with this, we exclude all external vertices in cliques.
            const auto& adj = GetAdjacency(root);
            int min_deg = adj.size();
            for (auto w : adj) {
                min_deg = min(min_deg, Degree(w));
            }
            
            if (min_deg != (int)adj.size()) {
                visi[root] = true;
            }
        }
    }

    void UpdateVertexTimestamp(int node, bool force = false, TimestampType type = TimestampType::DegreeDecrease) {
        if (!force)
            custom_assert(current_timestamp[node] != -1);

        current_timestamp[node] = current_kernelization_time;
        vertex_timetable.push_back(make_pair(current_kernelization_time, make_pair(node,type)));

        current_kernelization_time += 1;
    }

    vector<int> GetVerticesAfterTimestamp(int timestamp, int include_neighbhors = 0) {
        vector<pair<int,pair<int,TimestampType>>> selected;
        /*while (!vertex_timetable.empty()) {
            auto u = vertex_timetable.top();
            vertex_timetable.pop();
            if (current_timestamp[u.second.first] != u.first) continue; // has been made invalid.

            if (u.first < timestamp) {
                vertex_timetable.push(u); // return the one we don't want.
                break;
            }
            
            selected.push_back(u);
        }*/

        for (int i = max(0, timestamp - 1); i < (int)vertex_timetable.size(); ++i) {
            auto u = vertex_timetable[i];

            if (current_timestamp[u.second.first] != u.first) continue; // has been made invalid.
            selected.push_back(u);
        }
        
        unordered_map<int,bool> visi;
        for (auto entry : selected) {
            //assert(!visi[entry.second.first]);
            visi[entry.second.first] = true;

            const auto& adj = GetAdjacency(entry.second.first);
            if (((include_neighbhors & static_cast<int>(TimestampType::DegreeDecrease)) > 0 && (static_cast<int>(entry.second.second) & static_cast<int>(TimestampType::DegreeDecrease)))
                    || ((include_neighbhors & static_cast<int>(TimestampType::DegreeIncrease)) > 0 && (static_cast<int>(entry.second.second) & static_cast<int>(TimestampType::DegreeIncrease)))
                ) {
                for (auto w : adj)
                    visi[w] = true;
            }
        }


        vector<int> ret;
        for (auto it : visi)
            ret.push_back(it.first);
        
        //for (auto entry : selected) {
        //    if (current_timestamp[entry.second.first] == entry.first)
        //        vertex_timetable.push(entry); // put back as it was not changed nor requested to be removed.
        //}

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

    unordered_map<long long, EdgeWeight> edge_weight;
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
    vector<pair<int,pair<int,TimestampType>>> vertex_timetable;
    vector<int> current_timestamp; // will hold the most recent timestamp for each vertex. Used to identify outdated values in pq!
    int current_kernelization_time = 1;

    

    double inflicted_cut_change_to_kernelized = 0; // absolute! beta(G') = beta(G) + inflicted_cut_change_to_kernelized
    unordered_map<RuleIds, int> rules_usage_count;
    unordered_map<RuleIds, int> rules_check_count;
    unordered_map<RuleIds, double> rules_time_usage_total;
    unordered_map<RuleIds, double> rules_vrem;

    string graph_naming;
    int mixing_id = -1;

    bool is_scaled = false;
    bool graph_is_supported = true;
};