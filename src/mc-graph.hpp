#pragma once

#include "./utils.hpp"

#include <bits/stdc++.h>
using namespace std;

// Definition articulation node:
// Its removal parts the graph in at least two non-empty graphs.

// Undirected, Unweighted.
class MaxCutGraph {
public:
    MaxCutGraph() {}

    MaxCutGraph(int n, int m) {
        num_nodes = n, num_edges = m;
        g_adj_list.resize(num_nodes);
    }

    // File based input is 1-index based.
    MaxCutGraph(const string path) {
        ifstream in(path.c_str());
        if (in.fail()) {
            throw std::logic_error("File doesn't exist.");
        }

        in >> num_nodes >> num_edges;
        g_adj_list.resize(num_nodes);

        string w;
        getline(in, w); // previous EOL
        for (int i = 0; i < num_edges; ++i) {
            getline(in, w);

            vector<int> params;
            stringstream line_in(w);
            while (line_in.eof() == false) {
                int val; line_in >> val;
                params.push_back(val);
            }

            if (params.size() < 2) throw std::logic_error("Line malformed: " + to_string(i));

            AddEdge(params[0] - 1, params[1] - 1);
        }
    }

    // Create induced subgraph
    MaxCutGraph(const MaxCutGraph& source, const vector<int>& subset) : MaxCutGraph(source.GetNumNodes(), -1) {
        int source_num_nodes = source.GetNumNodes();

        for (int i = 0; i < source_num_nodes; ++i)
            removed_node[i] = true;
        
        for (const int node : subset)
            removed_node[node] = false;

        num_edges = 0;
        for (const int node : subset) {
            auto adj = source.GetAdjacency(node);
            for (const int w : adj) {
                if (removed_node[w])
                    continue;

                AddEdge(node, w);
                num_edges++;
            }
        }
    }

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
    void ComputeArticulationAndBiconnected() {
        biconnected_components.clear();
        is_articulation.assign(num_nodes, false);

        vector<bool> visited(num_nodes, false);
        vector<int> depth(num_nodes, -1);
        vector<int> low(num_nodes, -1);
        vector<int> child_count(num_nodes, 0);
        vector<int> parent(num_nodes, -1);

        // DFS tree tracking data:
        stack<pair<int,int>> component_edges;
        vector<int> component;

        // In case multiple components exist, go over all nodes:
        for (int curr_node = 0; curr_node < num_nodes; ++curr_node) {
            if (depth[curr_node] >= 0 || removed_node[curr_node]) continue;

            stack<tarjan_dfs_data> stk;
            stk.push({0, curr_node, tarjan_dfs_data_type::FIRST_VISIT, 0});
            
            while (!stk.empty()) {
                tarjan_dfs_data u = stk.top();
                stk.pop();
                
                int start_it_dx = 0;
                if (u.type == tarjan_dfs_data_type::FIRST_VISIT) {
                    low[u.node] = depth[u.node] = u.depth;
                } else if (u.type == tarjan_dfs_data_type::REVISIT) {
                    int w = g_adj_list[u.node][u.last_dx];
                    if (low[u.node] > low[w]) low[u.node] = low[w];
                    if (low[w] >= u.depth) {
                        is_articulation[u.node] = true;
                       // cout << "COMPONENT-X: " << parent[u.node] << " " << u.node<< endl;
                        while (!component_edges.empty()) {
                            pair<int,int> e = component_edges.top();
                            component_edges.pop();
                            // component.push_back(e.first); -- we dont want this since it makes double vertices
                            component.push_back(e.second);
                            //cout << e.first << " " << e.second << endl;

                            if (e.first == u.node) {
                                component.push_back(e.first);
                                break;
                            }
                        }
                        biconnected_components.push_back(component);
                        component.clear();
                    }
                    start_it_dx = u.last_dx + 1;
                }

                for (unsigned int i = start_it_dx; i < g_adj_list[u.node].size(); ++i) {
                    int w = g_adj_list[u.node][i];
                    
                    if (depth[w] < 0) {
                        component_edges.push(make_pair(u.node,w));
                        child_count[u.node]++;
                        parent[w] = u.node;
                        stk.push({u.depth, u.node, tarjan_dfs_data_type::REVISIT, i});
                        stk.push({u.depth + 1, w, tarjan_dfs_data_type::FIRST_VISIT, 0});
                        break;
                    } else if (w != parent[u.node]) {
                        if (low[u.node] > depth[w]) low[u.node] = depth[w];
                    }
                }
            }

            if (g_adj_list[curr_node].size() == 0) {
               // cout << "COMPONENT: " << endl;
               // cout << curr_node << endl;
                biconnected_components.push_back(vector<int>{curr_node});
            }

            is_articulation[curr_node] = child_count[curr_node] > 1;
        }
      //  cout << "DONE" << endl;

        articulations_computed = true;
        bicomponents_computed = true;
    }

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

        if (any_adj_to_v_node) {
            throw std::logic_error("Rule 3 assertion fail: No adjacent node to v found in C");
        }

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
       // MaxCutGraph c_minus_r_graph(c_graph, component_minus_r);
      //  MaxCutGraph minus_r
    }

    vector<int> FindInducedPathForRule6(const vector<int>& component, const int r) {
        MaxCutGraph c_graph(*this, component);

        auto component_minus_r = SetSubstract(component, vector<int>{r});
        MaxCutGraph c_minus_r_graph(c_graph, component_minus_r);
        c_minus_r_graph.ComputeArticulationAndBiconnected();
        auto bicomponents_sub = c_minus_r_graph.GetBiconnectedComponents();
        OutputDebugLog("Number of biconnected components in X - r: " + to_string(bicomponents_sub.size()));

        if (bicomponents_sub.size() == 1) { // X - r is 2-connected
            c_graph.CalculateSingleSourceDistance(r);

            // Calculate L_i's
            int mx = 0;
            for (auto node : component) mx = max(mx, c_graph.GetSingleSourceDistance(node));
            vector<vector<int>> Li(mx + 1);
            OutputDebugLog("Maximum distance from r in X: " + to_string(mx));
            for (auto node : component) Li[c_graph.GetSingleSourceDistance(node)].push_back(node); // different from paper since we also take r
            sort(Li[1].begin(), Li[1].end());// need for comparing {x,y} = Li[1]
            OutputDebugLog("Li computation successful");

            // make sure lexicographically sorted
            sort(component_minus_r.begin(), component_minus_r.end());

            // find the x,y
            int current_min_d_xr = 1e9;
            int selected_x = -1, selected_y = -1;
            for (unsigned int i = 0; i < component_minus_r.size(); ++i) {
                for (unsigned int j = 0; j < component_minus_r.size(); ++j) { // we can't do j = i + 1, because symmetry not given in x and y because of min(r,x) condition.
                    if (i == j) continue;

                    int x = component_minus_r[i];
                    int y = component_minus_r[j];

                    if (edge_exists_lookup[make_pair(x,y)]) continue;
                    if (Li[1] == vector<int>{x,y}) continue;
                    
                    int d_xr = c_graph.GetSingleSourceDistance(x);
                    if (current_min_d_xr > d_xr) {
                        current_min_d_xr = d_xr;
                        selected_x = x;
                        selected_y = y;
                    }
                }
            }
            OutputDebugLog("Selection of (x,y) = (" + to_string(selected_x) + "," + to_string(selected_y) + ")");

            // Shortest path Q from r to x
            auto Q = c_graph.GetSingleSourcePathFromRoot(selected_x);
            assert(Li[1].size() > 0 && Li[3].size() > 0);
            assert(Q.size() <= 3); // length of Q <= 2, meaning at most 3 nodes on path
            OutputDebugVector("Q", Q);

            vector<int> C_minus_Q_minus_x = SetSubstract(Q, {selected_x});
            C_minus_Q_minus_x = SetSubstract(component, C_minus_Q_minus_x);
            
            MaxCutGraph G_CmQmx(c_graph, C_minus_Q_minus_x);
            G_CmQmx.CalculateSingleSourceDistance(selected_x);
            auto P = G_CmQmx.GetSingleSourcePathFromRoot(selected_y);
            
            if (P.size() >= 3) {
                assert(P[0] == selected_x);

                vector<int> PP;
                for (unsigned int i = 0; i < P.size(); ++i)
                    PP.push_back(P[i]);
                
                if(!DoesDisconnect(PP)) return PP; // G!
            } else {
                assert(false);
            }

        //   vector<int> xy = c_
        } else { // not 2-connected => use Lemma 4
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

            return vector<int>{w[1], v, w[2]};
        }

        return vector<int>();
    }

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

    vector<vector<int>> GetBiconnectedComponents() {
        if (!bicomponents_computed) ComputeArticulationAndBiconnected();

        return biconnected_components;
    }

    vector<int> GetMarkedVerticesByOneWayRules() const {
        return paper_S;
    }

    double GetEdwardsErdosBound() {
        return (num_edges / 2.0) + (num_nodes - 1) / 4.0;
    }

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