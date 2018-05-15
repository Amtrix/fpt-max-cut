#pragma once

#include <bits/stdc++.h>
using namespace std;

// Definition articulation node:
// Its removal parts the graph in at least two non-empty graphs.

// Undirected, Unweighted.
class MaxCutGraph {
public:
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

    void AddEdge(int a, int b) {
        g_adj_list[a].push_back(b);
        g_adj_list[b].push_back(a);
        edge_exists_lookup[make_pair(a,b)] = true;
        edge_exists_lookup[make_pair(b,a)] = true;

        bicomponents_computed = false;
        articulations_computed = false;
    }

    const vector<int>& GetAdjacency(int node) {
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
            if (depth[curr_node] >= 0) continue;

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

    int num_nodes, num_edges;

    bool bicomponents_computed = false;
    bool articulations_computed = false;

    map<pair<int,int>, bool> edge_exists_lookup; // IMPROVE TO O(1)!!!!
    vector<vector<int>> g_adj_list;
    vector<vector<int>> biconnected_components;
    vector<int> paper_S;

    vector<bool> is_articulation;
};