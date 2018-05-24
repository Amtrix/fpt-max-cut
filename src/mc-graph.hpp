#pragma once

#include "./utils.hpp"

#include <bits/stdc++.h>
using namespace std;

// Definition articulation node:
// Its removal parts the graph in at least two non-empty graphs.

// Undirected, Unweighted.
// pointers not allowed due to usage of default copy in benchmark.cpp !!!!
class MaxCutGraph {
public:
    MaxCutGraph();

    MaxCutGraph(int n, int m);

    // File based input is 1-index based.
    MaxCutGraph(const string path);

    // Create induced subgraph
    MaxCutGraph(const MaxCutGraph& source, const vector<int>& subset);

    int GetNumNodes() const { return num_nodes; }
    int GetNumEdges() const { return num_edges; }

    void AddEdge(int a, int b) {
        g_adj_list[a].push_back(b);
        g_adj_list[b].push_back(a);
        edge_exists_lookup[make_pair(a,b)] = true;
        edge_exists_lookup[make_pair(b,a)] = true;

        bicomponents_computed = false;
        articulations_computed = false;
    }

    const vector<int>& GetAdjacency(int node) const {
        return g_adj_list[node];
    }
    bool AreAdjacent(int n1, int n2) {
        return edge_exists_lookup[make_pair(n1,n2)];
    }

    // https://github.com/niklasb/tcr/blob/master/graphentheorie/arti-bruecken.cpp
    void ComputeArticulationAndBiconnected();

    void CalculateSingleSourceDistance(int source) {
        single_source_dist.assign(num_nodes, -1);
        single_source_prev.assign(num_nodes, -1);

        queue<int> q;
        q.push(source);
        single_source_dist[source] = 0;


        while (!q.empty()) {
            int u = q.front();
            q.pop();
            
            for (unsigned int i = 0; i < g_adj_list[u].size(); ++i) {
                int w = g_adj_list[u][i];

                if (single_source_dist[w] > -1) continue;
                single_source_prev[w] = u;
                single_source_dist[w] = single_source_dist[u] + 1;
                q.push(w);
            }
        }
    }

    /*
    MISSING: ignoring removed nodes.
    vector<int> GetReachableNodes(int snode, const vector<int>& ignore) {
        vector<bool> visited(num_nodes, false);
        for (auto node : ignore) visited[node] = true;

        queue<int> q;
        q.push(snode);

        vector<int> ret;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            ret.push_back(u);

            for (unsigned int i = 0; i < g_adj_list[u].size(); ++i) {
                int w = g_adj_list[u][i];
                if (visited[w]) continue;
                visited[w] = true;
                q.push(w);
            }
        }
        
        return ret;
    }*/

    vector<vector<int>> GetAllConnectedComponents() {
        vector<bool> visited(num_nodes, false);
        vector<vector<int>> ret;

        for (int curr_node = 0; curr_node < num_nodes; ++curr_node) {
            if (visited[curr_node] || removed_node[curr_node]) continue;

            queue<int> q;
            visited[curr_node] = true;
            q.push(curr_node);

            vector<int> component;
            while (!q.empty()) {
                int u = q.front();
                q.pop();

                component.push_back(u);

                for (unsigned int i = 0; i < g_adj_list[u].size(); ++i) {
                    int w = g_adj_list[u][i];

                    if (visited[w] || removed_node[w]) continue;
                    visited[w] = true;
                    q.push(w);
                }
            }

            ret.push_back(component);
        }

        return ret;
    }

    bool DoesDisconnect(vector<int> selection_rem) {
        auto before = GetAllConnectedComponents();
        auto vset_after_sub = SetSubstract(GetAllExistingNodes(), selection_rem);
        MaxCutGraph ng(*this, vset_after_sub);
        auto after = ng.GetAllConnectedComponents();

        // If a whole component gets deleted, it doesn't make the graph disconnected, so account for these cases.
        int num_deleted_whole_components = 0;
        for (auto component : before)
            if (IsASubsetOfB(component, selection_rem))
                num_deleted_whole_components++;
        
        return before.size() != (after.size() + num_deleted_whole_components);
    }

    bool Breaks2Connected(vector<int> selection_rem) {
        if (!bicomponents_computed) ComputeArticulationAndBiconnected();
        assert(biconnected_components.size() == 1);

        auto vset = GetAllExistingNodes();
        auto subvset = SetSubstract(vset, selection_rem);
        MaxCutGraph nwg(*this, subvset);
        nwg.ComputeArticulationAndBiconnected();

        return nwg.GetBiconnectedComponents().size() != 1;
    }

    int GetSingleSourceDistance(int dest) {
        return single_source_dist[dest];
    }

    // Return: {r,...,dest}
    vector<int> GetSingleSourcePathFromRoot(int dest) {
        if (single_source_dist[dest] == -1)
            throw logic_error("Requested path to unreachable destination.");
        
        vector<int> ret;
        int curr = dest;
        while (curr != -1) {
            ret.push_back(curr);
            curr = single_source_prev[curr];
        }
        
        reverse(ret.begin(), ret.end());
        return ret;
    }

    void RemoveNode(int node) {
        articulations_computed = false;
        bicomponents_computed = false;

        g_adj_list[node].clear();

        for (int i = 0; i < num_nodes; ++i) {
            const auto it = std::find(g_adj_list[i].cbegin(), g_adj_list[i].cend(), node);
            if (it != g_adj_list[i].end())
                g_adj_list[i].erase(it);
            
            edge_exists_lookup.erase(make_pair(node, i));
            edge_exists_lookup.erase(make_pair(i, node));
        }

        removed_node[node] = true;
    }

    vector<int> GetAllExistingNodes() {
        vector<int> ret;
        for (int i = 0; i < num_nodes; ++i)
            if (!removed_node[i])
                ret.push_back(i);
        return ret;
    }
    vector<pair<int,int>> GetAllExistingEdges() {
        vector<pair<int,int>> ret;
        for (int i = 0; i < num_nodes; ++i) {
            if (removed_node[i]) continue;
            for (int w : g_adj_list[i]) {
                if (removed_node[w] || i > w) continue;
                ret.push_back(make_pair(i, w));
            }
        }
        return ret;
    }

    bool IsClique(const vector<int>& vertex_set) {
        for (unsigned int i = 0; i < vertex_set.size(); ++i)
            for (unsigned int j = i + 1; j < vertex_set.size(); ++j)
                if (edge_exists_lookup[make_pair(vertex_set[i], vertex_set[j])] == false)
                    return false;
        return true;
    }

    // (c_with_v - v) is clique
    void ApplyRule3(const vector<int>& c_with_v, const int v) {
        int any_adj_to_v_node = -1;
        int any_nonadj_to_v_node = -1;
        for (const int node : c_with_v) {
            if (node == v) continue;

            if (edge_exists_lookup[make_pair(node, v)])
                any_adj_to_v_node = node;
            if (!edge_exists_lookup[make_pair(node, v)])
                any_nonadj_to_v_node = node;
            
            if (any_adj_to_v_node >= 0 && any_nonadj_to_v_node >= 0)
                break;
        }

        if (any_adj_to_v_node == -1) {
            throw std::logic_error("Rule 3 assertion fail: No adjacent node to v found in C");
        }

        OutputDebugLog("a = " + to_string(any_adj_to_v_node));
        OutputDebugLog("b = " + to_string(any_nonadj_to_v_node));

        RemoveNode(any_adj_to_v_node);
        RemoveNode(any_nonadj_to_v_node);
        paper_S.push_back(any_adj_to_v_node);
        paper_S.push_back(any_nonadj_to_v_node);
    }

    void ApplyRule5(const vector<int>& c_with_v, const int v) {
        for (const int node : c_with_v) {
            if (node == v) continue;
            RemoveNode(node);
        }
    }

    vector<int> GetInducedPathByLemma2(const vector<int>& component, int r) {
        assert(GetBiconnectedComponents().size() == 1);

        auto component_minus_r = SetSubstract(component, vector<int>{r});
        MaxCutGraph c_graph(*this, component);
        MaxCutGraph c_minus_r_graph(c_graph, component_minus_r);

        OutputDebugLog("Computing induced path for Rule 6 by using Lemma, since X - r not 2-connected.");

        // 1.
        auto bicomponents = c_minus_r_graph.GetBiconnectedComponents();
        auto anodes = c_minus_r_graph.GetArticulationNodes();
        assert(anodes.size() > 0);
        int v = anodes[0];

        vector<int> Z[3];
        for (auto component : bicomponents) {
            if (find(component.begin(), component.end(), v) != component.end()) {
                if (Z[1].empty()) Z[1] = component;
                else if (Z[2].empty()) { Z[2] = component; break; }
            }
        }

        assert(!Z[2].empty());
        OutputDebugLog("Cut vertex v = " + to_string(v));
        OutputDebugVector("Z1", Z[1]);
        OutputDebugVector("Z2", Z[2]);

        // 2.
        auto component_minus_v = SetSubstract(component, vector<int>{v});
        MaxCutGraph c_minus_v_graph(c_graph, component_minus_v);
        c_minus_v_graph.CalculateSingleSourceDistance(r);
        OutputDebugLog("X - v graph computed.");

        int u_dist[3] = {-1, 1 << 30, 1 << 30};
        int u[3] = {-1, -1, -1};
        for (unsigned int i = 1; i <= 2; ++i) {
            for (auto node : Z[i]) {
                if (node == v) continue;
                int d = c_minus_v_graph.GetSingleSourceDistance(node);
                if (d < u_dist[i]) {
                    u_dist[i] = d;
                    u[i] = node;
                }
            }
        }

        assert(u[1] != -1 && u[2] != -1);
        assert(u_dist[1] != -1 && u_dist[2] != -1);

        vector<int> P[3] = {(vector<int>{}), c_minus_v_graph.GetSingleSourcePathFromRoot(u[1]),
            c_minus_v_graph.GetSingleSourcePathFromRoot(u[2])};
        
        // 3.
        MaxCutGraph T[3];
        for (unsigned int i = 1; i <= 2; ++i) {
            T[i] = MaxCutGraph(c_graph, Z[i]);
            T[i].CalculateLemma4DFSTree(v, u[i]);
        }

        // 4.
        int w[3] = {-1, -1, -1};
        int w_depth[3] = {-1, -1, -1};
        for (unsigned int i = 1; i <= 2; ++i) {
            const auto& adj_v = T[i].GetAdjacency(v);
            for (auto w_candidate : adj_v) {
                int depth = T[i].GetDfsTreeDepthFromRoot(w_candidate);
                if (depth > w_depth[i]) { // I ASSUME!!!!!!!!! lowest in dfs tree => largest depth.
                    w_depth[i] = depth;
                    w[i] = w_candidate;
                }
            }
        }

        assert(w[1] != -1 && w[2] != -1);
        OutputDebugLog("(w1,w2) = (" + to_string(w[1]) + "," + to_string(w[2]) + ")");

        // 5.
        return vector<int>{w[1], v, w[2]};
    }

    vector<int> FindInducedPathForRule6(const vector<int>& component, const int r);

    void CalculateLemma4DFSTree(int root, int ui) {
        dfs_tree_parent.assign(num_nodes, -1);
        dfs_tree_depth.assign(num_nodes, -1);
        dfs_tree_ui = ui;

        dfs_tree_depth[root] = 0;
        CalculateLemma4DFSTree_(root);
    }

    int GetDfsTreeDepthFromRoot(int node) {
        return dfs_tree_depth[node];
    }

    void ApplyRule6(const vector<int>& induced_2path) {
        assert(induced_2path.size() == 3);

        for (auto node : induced_2path) {
            RemoveNode(node);
            paper_S.push_back(node);
        }
    }

    void ApplyRule7(const vector<int>& c, const int v, const int b) {
        RemoveNode(v);
        RemoveNode(b);
        paper_S.push_back(v);
        paper_S.push_back(b);

        for (const int node : c)
            RemoveNode(node);
    }

    vector<int> GetArticulationNodes() {
        if (!articulations_computed) ComputeArticulationAndBiconnected();

        vector<int> ret;
        for (int i = 0; i < num_nodes; ++i)
            if (is_articulation[i])
                ret.push_back(i);
                
        return ret;
    }

    bool IsArticulation(int node) {
        if (!articulations_computed) ComputeArticulationAndBiconnected();

        return is_articulation[node];
    }

    bool IsCliqueForest();

    vector<vector<int>> GetBiconnectedComponents();

    vector<int> GetMarkedVerticesByOneWayRules() const;

    double GetEdwardsErdosBound() const;

    // Returns -1 if no component of size >= 1 was found.
    tuple<vector<int>, int> GetLeafBlockAndArticulation(bool print_components = false);

    // Makes assumption(!) that S is subset of G. We can't check this, as this is time-critical.
    int ComputeCut(const vector<int>& S, const vector<int>& S_color);
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
    void CalculateLemma4DFSTree_(int node) {
        // ui child of v if (v,ui) in E; here v = root, dfs_tree_ui = ui.
        if (dfs_tree_depth[node] == 0) {
            for (auto child : g_adj_list[node]) {
                if (child == dfs_tree_ui) {
                    dfs_tree_parent[child] = node;
                    dfs_tree_depth[child] = dfs_tree_depth[node] + 1;
                    CalculateLemma4DFSTree_(child);
                }
            }
        }

        for (auto child : g_adj_list[node]) {
            if (dfs_tree_depth[child] > -1) continue; // visited

            dfs_tree_parent[child] = node;
            dfs_tree_depth[child] = dfs_tree_depth[node] + 1;
            CalculateLemma4DFSTree_(child);
        }
    }

    vector<int> dfs_tree_parent;
    vector<int> dfs_tree_depth;
    int dfs_tree_ui;

    int num_nodes, num_edges;

    bool bicomponents_computed = false;
    bool articulations_computed = false;

    // Invariant: no edges should exist from/to removed_nodes.
    unordered_map<int, bool> removed_node;

    map<pair<int,int>, bool> edge_exists_lookup; // IMPROVE TO O(1)!!!!
    vector<vector<int>> g_adj_list;
    vector<vector<int>> biconnected_components;
    vector<int> paper_S;

    vector<bool> is_articulation;

    // Used inside CalculateSingleSourceDistance
    vector<int> single_source_dist;
    vector<int> single_source_prev;
};