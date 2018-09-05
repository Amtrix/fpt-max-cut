#include "mc-graph.hpp"
#include "heuristics/qubo/glover1998a.h"
#include "heuristics/maxcut/burer2002.h"


 MaxCutGraph::MaxCutGraph() {

 }

 MaxCutGraph::MaxCutGraph(int n, int /* not used m */) {
    num_nodes = n;
    g_adj_list.resize(num_nodes);
}

MaxCutGraph::MaxCutGraph(const string path) {
    ifstream in(path.c_str());
    if (in.fail()) {
        throw std::logic_error("File doesn't exist.");
    }

    vector<string> sparams = ReadLine(in);

    // we take last two entries as dimacs prefixes each line with type of line
    if (sparams[0] != "#edge-list-0") {
        num_nodes = stoi(sparams[0 + (sparams[0]=="p")]);

        int num_edges = stoi(sparams[1 + (sparams[0]=="p")]);
        g_adj_list.resize(num_nodes);

        for (int i = 0; i < num_edges; ++i) {
            sparams = ReadLine(in);
            if (sparams.size() < 2) throw std::logic_error("Line malformed: " + to_string(i));
            AddEdge(stoi(sparams[0 + (sparams[0]=="e")]) - 1, stoi(sparams[1 + (sparams[0]=="e")]) - 1);
        }
    } else {
        num_nodes = 0;
        g_adj_list.resize(2000);
        while (in.eof() == false) {
            sparams = ReadLine(in);
            if (sparams.size() == 0) continue;
            if (sparams.size() < 2) throw std::logic_error("Line malformed: " + to_string(-1));
            int a = stoi(sparams[0]);
            int b = stoi(sparams[1]);
            if (a >= 2000 || b >= 2000) throw std::logic_error("Graph size not supported.. yet. Input line: " + to_string(-1));
            num_nodes = max(num_nodes, max(a + 1, b + 1));
            AddEdge(a, b);
        }
    }
    
    OutputDebugLog("Reading from file done.");
}

MaxCutGraph::MaxCutGraph(const vector<pair<int,int>> &elist) {
    num_nodes = 0;
    g_adj_list.resize(2000);
    for (auto e : elist) {
        int a = e.first;
        int b = e.second;
        if (a >= 2000 || b >= 2000) throw std::logic_error("Graph size not supported.. yet. Input line: " + to_string(-1));
        num_nodes = max(num_nodes, max(a + 1, b + 1));
        AddEdge(a, b);
    }
}

MaxCutGraph::MaxCutGraph(const MaxCutGraph& source, const vector<int>& subset) : MaxCutGraph(source.GetNumNodes(), -1) {
    int source_num_nodes = source.GetNumNodes();

    for (int i = 0; i < source_num_nodes; ++i)
        removed_node[i] = true;
    
    for (const int node : subset)
        removed_node[node] = false;

    for (const int node : subset) {
        auto adj = source.GetAdjacency(node);
        for (const int w : adj) {
            if (removed_node[w] || edge_exists_lookup[make_pair(node, w)])
                continue;

            AddEdge(node, w);
        }
    }
}

void MaxCutGraph::ComputeArticulationAndBiconnected() {
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

                    while (!component_edges.empty()) {
                        pair<int,int> e = component_edges.top();
                        component_edges.pop();
                        // component.push_back(e.first); -- we dont want this since it makes double vertices
                        component.push_back(e.second);

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
            biconnected_components.push_back(vector<int>{curr_node});
        }

        is_articulation[curr_node] = child_count[curr_node] > 1;
    }

    articulations_computed = true;
    bicomponents_computed = true;
}

void MaxCutGraph::CalculateSingleSourceDistance(int source) {
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

vector<int> MaxCutGraph::GetConnectedComponentOf(int node) {
    vector<bool> visited(num_nodes, false);
    queue<int> q;
    visited[node] = true;
    q.push(node);

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

    return component;
}

vector<vector<int>> MaxCutGraph::GetAllConnectedComponents() {
    vector<bool> visited(num_nodes, false);
    vector<vector<int>> ret;

    for (int curr_node = 0; curr_node < num_nodes; ++curr_node) {
        if (visited[curr_node] || removed_node[curr_node]) continue;

        auto component = GetConnectedComponentOf(curr_node);
        ret.push_back(component);

        for (auto node : component)
            visited[node] = true;
    }

    return ret;
}

bool MaxCutGraph::DoesDisconnect(vector<int> selection_rem) {
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

bool MaxCutGraph::Breaks2Connected(vector<int> selection_rem) {
    if (!bicomponents_computed) ComputeArticulationAndBiconnected();
    assert(biconnected_components.size() == 1);

    auto vset = GetAllExistingNodes();
    auto subvset = SetSubstract(vset, selection_rem);
    MaxCutGraph nwg(*this, subvset);
    nwg.ComputeArticulationAndBiconnected();

    return nwg.GetBiconnectedComponents().size() != 1;
}

// Return: {r,...,dest}
vector<int> MaxCutGraph::GetSingleSourcePathFromRoot(int dest) {
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

void MaxCutGraph::RemoveNode(int node) {
    articulations_computed = false;
    bicomponents_computed = false;
    bridges_computed = false;

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
void MaxCutGraph::ReAddNode(int node) {
    removed_node[node] = false;
}

void MaxCutGraph::RemoveEdgesBetween(int nodex, int nodey) {
    auto it = std::find(g_adj_list[nodex].cbegin(), g_adj_list[nodex].cend(), nodey);
    if (it != g_adj_list[nodex].end())
        g_adj_list[nodex].erase(it);
    
    it = std::find(g_adj_list[nodey].cbegin(), g_adj_list[nodey].cend(), nodex);
    if (it != g_adj_list[nodey].end())
        g_adj_list[nodey].erase(it);
    
    edge_exists_lookup.erase(make_pair(nodex, nodey));
    edge_exists_lookup.erase(make_pair(nodey, nodex));
}

void MaxCutGraph::RemoveEdgesInComponent(const vector<int> &component) {
    for (int i = 0; i < (int)component.size(); ++i) {
        for (int j = i + 1; j < (int)component.size(); ++j) {
            RemoveEdgesBetween(component[i], component[j]);
        }
    }
}

vector<int> MaxCutGraph::GetAllExistingNodes() {
    vector<int> ret;
    for (int i = 0; i < num_nodes; ++i)
        if (!removed_node[i])
            ret.push_back(i);
    return ret;
}

vector<pair<int,int>> MaxCutGraph::GetAllExistingEdges() {
    vector<pair<int,int>> ret;
    for (int i = 0; i < num_nodes; ++i) {
        if (removed_node[i]) continue;
        for (int w : g_adj_list[i]) {
            if (removed_node[w] || i >= w) continue;
            ret.push_back(make_pair(i, w));
        }
    }
    return ret;
}

bool MaxCutGraph::IsClique(const vector<int>& vertex_set) {
    for (unsigned int i = 0; i < vertex_set.size(); ++i)
        for (unsigned int j = i + 1; j < vertex_set.size(); ++j)
            if (edge_exists_lookup[make_pair(vertex_set[i], vertex_set[j])] == false)
                return false;
    return true;
}

void MaxCutGraph::ApplyRule3(const vector<int>& c_with_v, const int v) {
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

void MaxCutGraph::ApplyRule5(const vector<int>& c_with_v, const int v) {
    for (const int node : c_with_v) {
        if (node == v) continue;
        RemoveNode(node);
    }
}

void MaxCutGraph::ApplyRule6(const vector<int>& induced_2path) {
    assert(induced_2path.size() == 3);

    for (auto node : induced_2path) {
        RemoveNode(node);
        paper_S.push_back(node);
    }
}

void MaxCutGraph::ApplyRule7(const vector<int>& c, const int v, const int b) {
    RemoveNode(v);
    RemoveNode(b);
    paper_S.push_back(v);
    paper_S.push_back(b);

    for (const int node : c)
        RemoveNode(node);
}

vector<int> MaxCutGraph::GetInducedPathByLemma2(const vector<int>& component, int r) {
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


vector<int> MaxCutGraph::FindInducedPathForRule6(const vector<int>& component, const int r) {
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
        // assert(Li[1].size() > 0 && Li[3].size() > 0); This assertion was wrong -- misread in paper.
        assert(Q.size() <= 3); // length of Q <= 2, meaning at most 3 nodes on path
        OutputDebugVector("Q", Q);

        vector<int> C_minus_Q_minus_x = SetSubstract(Q, {selected_x});
        C_minus_Q_minus_x = SetSubstract(component, C_minus_Q_minus_x);
        
        MaxCutGraph G_CmQmx(c_graph, C_minus_Q_minus_x);
        G_CmQmx.CalculateSingleSourceDistance(selected_x);
        auto P = G_CmQmx.GetSingleSourcePathFromRoot(selected_y);
        OutputDebugVector("P", P);
        
        vector<int> PP;
        if (P.size() >= 3) {
            assert(P[0] == selected_x);

            for (unsigned int i = 0; i < 3; ++i)
                PP.push_back(P[i]);
            
            OutputDebugVector("PP", PP);
            if(!DoesDisconnect(PP)) return PP; // G!
        } else {
            // just comsmetic comment here
            // if P.size() < 3, then P doesn't exist.
            // P.size() >= 3 has to hold because of the way we selected x,y
            // {x,y} != L[1]
        }

        // We still didn't succeed => we do lemma 2 now.

        vector<int> QPP = Q; // Q.size <= 3
        for (unsigned int i = 1; i < PP.size(); ++i) // we start at 1 because of the overlap between end of Q and start of PP
            QPP.push_back(PP[i]); // PP.size <= 3, therefore QPP.size <= 6
        
        assert(QPP.size() <= 6);
        assert(QPP[0] == r);
        //assert(DoesDisconnect(QPP));

        vector<int> prefix_that_disconnects;
        for (unsigned int i = 0; i < QPP.size(); ++i) {
            prefix_that_disconnects.push_back(QPP[i]);

            if (c_graph.Breaks2Connected(prefix_that_disconnects))
                break;
        }

        if (!c_graph.Breaks2Connected(prefix_that_disconnects)) {
            prefix_that_disconnects.clear();

            for (unsigned int i = 1; i < QPP.size(); ++i) {
                prefix_that_disconnects.push_back(QPP[i]);

                if (c_graph.Breaks2Connected(prefix_that_disconnects))
                    break;
            }
        }

        assert(c_graph.Breaks2Connected(prefix_that_disconnects));

        OutputDebugVector("Prefix that disconnects", prefix_that_disconnects);

        auto vertex_that_disconnected = prefix_that_disconnects.back();
        prefix_that_disconnects.pop_back(); // doesn't make != 2 connected anymore
        auto new_component = SetSubstract(component, prefix_that_disconnects);
        prefix_that_disconnects.push_back(vertex_that_disconnected); // to keep it meaningful
        
        MaxCutGraph graph_for_lemma(*this, new_component);
        graph_for_lemma.ComputeArticulationAndBiconnected();
        assert(graph_for_lemma.GetBiconnectedComponents().size() == 1);

        return graph_for_lemma.GetInducedPathByLemma2(new_component, vertex_that_disconnected);

    //   vector<int> xy = c_
    } else { // not 2-connected => use Lemma 4
        return c_graph.GetInducedPathByLemma2(component, r);
    }

    return vector<int>();
}

void MaxCutGraph::CalculateLemma4DFSTree(int root, int ui) {
    dfs_tree_parent.assign(num_nodes, -1);
    dfs_tree_depth.assign(num_nodes, -1);
    dfs_tree_ui = ui;

    dfs_tree_depth[root] = 0;
    CalculateLemma4DFSTree_(root);
}

vector<int> MaxCutGraph::GetArticulationNodes() {
    if (!articulations_computed) ComputeArticulationAndBiconnected();

    vector<int> ret;
    for (int i = 0; i < num_nodes; ++i)
        if (is_articulation[i])
            ret.push_back(i);
            
    return ret;
}

bool MaxCutGraph::IsBridgeBetween(int nodeA, int nodeB) {
    if (bridges_computed == false) {
        is_bridge_between.clear();

        if (bicomponents_computed == false) ComputeArticulationAndBiconnected();

        auto components = GetBiconnectedComponents();
        for (auto& component : components) {
            if (component.size() != 2) continue;

            is_bridge_between[make_pair(component[0], component[1])] = true;
            is_bridge_between[make_pair(component[1], component[0])] = true;
        }

        bridges_computed = true;
    }

    return is_bridge_between[make_pair(nodeA, nodeB)];
}

bool MaxCutGraph::IsCliqueForest() {
    if (bicomponents_computed) ComputeArticulationAndBiconnected();

    auto components = GetBiconnectedComponents();
    for (const auto& component : components)
        if (!IsClique(component))
            return false;

    return true;
}

vector<vector<int>> MaxCutGraph::GetBiconnectedComponents() {
    if (!bicomponents_computed) ComputeArticulationAndBiconnected();

    return biconnected_components;
}

vector<int> MaxCutGraph::GetMarkedVerticesByOneWayRules() const {
    return paper_S;
}

double MaxCutGraph::GetEdwardsErdosBound() {
    auto ccomponents = GetAllConnectedComponents();
    
    double res = -(GetAllConnectedComponents().size() / 4.0);
    for (auto component : ccomponents) {
        MaxCutGraph ng(*this, component);
        res += (ng.GetRealNumEdges() / 2.0) + (ng.GetRealNumNodes()) / 4.0;
    }

    return res;
}

tuple<vector<int>, int> MaxCutGraph::GetLeafBlockAndArticulation(bool print_components) {
    int selected_block_dx = -1;
    auto bicomponents = GetBiconnectedComponents();
    
    if (print_components) {
        OutputDebugLog("Number of biconnected components in graph: " + to_string(bicomponents.size()) + ", all components:");
        for (unsigned int dx = 0; dx < bicomponents.size(); ++dx) {
            const auto& component = bicomponents[dx];
            OutputDebugVector("Component " + to_string(dx), component);
#ifdef DEBUG
            MaxCutGraph cg(*this, component);
            OutputDebugVector("  edges", cg.GetAllExistingEdges());
#endif
        }
        OutputDebugLog("-- END-COMPONENTS --");
    }

    for (unsigned int i = 0; i < bicomponents.size(); ++i) {
        const auto& component = bicomponents[i];
        if (component.size() == 1) continue; // We ignore 1-vertex components

        int articulation_count = 0;
        for (const int node : component) articulation_count += IsArticulation(node);
        if (articulation_count <= 1 && (selected_block_dx == -1 || component.size() < bicomponents[selected_block_dx].size()))
            selected_block_dx = i;
    }

    // No leaf block with more than one vertex found => graph contains only isolated vertices.
    if (selected_block_dx == -1) return make_tuple(vector<int>(), -1);
    const auto& component = bicomponents[selected_block_dx];

    // Find r as outlined in paper. r is cut vertex if exists, or arbitrary vertex if not.
    int r = 0;
    for (unsigned int i = 1; i < component.size(); ++i)
        if (IsArticulation(component[i]))
            r = i;
    r = component[r];
    
    if (print_components)
        OutputDebugLog("r = " + to_string(r) + ", X = Component " + to_string(selected_block_dx));

    return make_tuple(component, r);
}

tuple<int, vector<int>> MaxCutGraph::MaxCutExtension(const vector<int>& S, const vector<int>& S_color) {
    vector<int> weight[2] = {vector<int>(num_nodes, 0), vector<int>(num_nodes, 0)};
    unordered_map<int,int> S_to_color;

    int p = 0;
    for (unsigned int i = 0; i < S.size(); ++i)
        for (unsigned int j = i + 1; j < S.size(); ++j)
            if (AreAdjacent(S[i], S[j]))
                p += S_color[i] != S_color[j];

    for (int u = 0; u < num_nodes; ++u) {
        if (removed_node[u]) continue;

        for (unsigned int i = 0; i < S.size(); ++i) {
            if (!AreAdjacent(u, S[i])) continue;

            weight[0][u] += S_color[i] == 1;
            weight[1][u] += S_color[i] == 0;
        }
    }


    auto G_minus_S_vertex_set = SetSubstract(GetAllExistingNodes(), S);
    MaxCutGraph G_minus_S(*this, G_minus_S_vertex_set);

    vector<tuple<int, int, int, vector<int>, vector<pair<int,int>>, vector<pair<int,int>> >> all_dp_substraction_steps;
    while(1) {
        vector<int> leaf_block;
        int r;

        tie(leaf_block, r) = G_minus_S.GetLeafBlockAndArticulation(false);
        
        if (r == -1) break;
        leaf_block = SetSubstract(leaf_block, vector<int>{r}); // make X U {r} represent whole block as in paper. X U {r} is clique (!!!!!!).
        const int block_size = leaf_block.size();

        vector<pair<int,int>> eps[2];
        int w_sum[2];
        int V[2];
        int V_step[2] = {-1, -1};

        for (int dx = 0; dx < 2; ++dx) {
            w_sum[0]  = 0, w_sum[1] = 0;
            for (int node : leaf_block) {
                int w[2] = {weight[0][node], weight[1][node]};
                eps[!dx].push_back(make_pair(dx == 0 ? w[1] - w[0] : w[0] - w[1], node));
                w_sum[dx] += w[dx];
            }
            sort(eps[!dx].rbegin(), eps[!dx].rend());

            V[!dx] = weight[!dx][r] + w_sum[dx] + block_size; // assume all in X are set to dx
            int all_to_all_flip_add = block_size;
            for (unsigned int i = 0; i < eps[!dx].size(); ++i) {
                auto entry = eps[!dx][i];
                int w[2] = {weight[0][entry.second], weight[1][entry.second]};
                
                w_sum[0] += dx == 0 ? -w[0] : w[0];
                w_sum[1] += dx == 0 ? w[1] : -w[1];

                all_to_all_flip_add += (block_size - (i + 1)) - (i + 1);
                int v_check = weight[!dx][r] + w_sum[!dx] + w_sum[dx] + all_to_all_flip_add;
                V[!dx] = max(v_check, V[!dx]);
                if (V[!dx] == v_check) V_step[!dx] = i;
            }
        }

        weight[0][r] = V[0];
        weight[1][r] = V[1];

        all_dp_substraction_steps.push_back(make_tuple(r, V_step[0], V_step[1], leaf_block, eps[0], eps[1]));
        auto allv = G_minus_S.GetAllExistingNodes();
        auto nextv = SetSubstract(allv, leaf_block);
        G_minus_S = MaxCutGraph(G_minus_S, nextv);
    }

    vector<int>computed_maxcut_coloring_tmp(num_nodes, -1);
    for (unsigned int i = 0; i < S.size(); ++i)
        computed_maxcut_coloring_tmp[S[i]] = S_color[i];

    int sol = 0;
    auto bicomponents = G_minus_S.GetBiconnectedComponents();
    for (auto component : bicomponents) {
        assert(component.size() == 1);
        int u = component[0];
        sol += max(weight[0][u], weight[1][u]);

        if (weight[0][u] > weight[1][u]) computed_maxcut_coloring_tmp[u] = 0;
        else computed_maxcut_coloring_tmp[u] = 1;
    }

    reverse(all_dp_substraction_steps.begin(), all_dp_substraction_steps.end());
    for (unsigned int i = 0; i < all_dp_substraction_steps.size(); ++i) {
        auto entry = all_dp_substraction_steps[i];
        auto component = get<3>(entry);

        int dx = computed_maxcut_coloring_tmp[get<0>(entry)];
        assert(dx != -1);

        int idx = -1;
        vector<pair<int,int>> eps;
        if (dx == 0) idx = get<1>(entry), eps = get<4>(entry);
        else if (dx == 1 ) idx = get<2>(entry), eps = get<5>(entry);

        for (int i = 0; i <= idx; ++i) computed_maxcut_coloring_tmp[eps[i].second] = dx;
        for (int i = idx + 1; i < (int)eps.size(); ++i) computed_maxcut_coloring_tmp[eps[i].second] = !dx;
    }

    return make_tuple(sol + p, computed_maxcut_coloring_tmp);
}

void MaxCutGraph::CalculateLemma4DFSTree_(int node) {
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

void MaxCutGraph::ReduceMarksetVertexSet() {
    vector<int> save_start_S = paper_S;
    for (int i = 0; i < 10; ++i) {
        vector<int> S = save_start_S;
        while (1) {
            bool was_possible = false;
            for (int node : S) {
                auto G_minus_S_vertex_set = SetSubstract(GetAllExistingNodes(), S);
                auto when_node_added = SetUnion(G_minus_S_vertex_set, vector<int>{node});
                MaxCutGraph G_minus_newS(*this, when_node_added);
                
                if (G_minus_newS.IsCliqueForest()) {
                    S.erase(std::remove(S.begin(), S.end(), node), S.end());
                    was_possible = true;
                    break;
                }
            }

            if (!was_possible) break;
        }

        if (paper_S.size() > S.size()) {
            paper_S = S;
        }
    }
}

int MaxCutGraph::Algorithm2MarkedComputation() {
    vector<pair<int,int>> vorder;
    for (int i = 0; i < num_nodes; ++i)
        vorder.push_back(make_pair(g_adj_list[i].size(), i));
    
    sort(vorder.begin(), vorder.end());
    vector<int> selection;

    while(1) {
        bool was_possible = false;

        for (unsigned int i = 0; i < vorder.size(); ++i) {
            auto nGvset = SetUnion(selection, vector<int>{vorder[i].second});
            MaxCutGraph nG(*this, nGvset);

            if (nG.IsCliqueForest()) {
                selection = nGvset;
                vorder.erase(std::remove(vorder.begin(), vorder.end(), vorder[i]), vorder.end());
                was_possible = true;
                break;
            }
        }

        if (!was_possible) break;
    }
    
    return num_nodes - (int)selection.size();
}

int MaxCutGraph::Algorithm3MarkedComputation_Randomized() {
    if (paper_S.size() == 0) return 0; // already optimal
    srand((unsigned)time(0));
    vector<int> save_start_S = paper_S;
    save_start_S.clear();
    for (int i = 0; i < num_nodes; ++i)
        save_start_S.push_back(i);
   
    unsigned int ret = (unsigned int)1e9;
    for (int i = 0; i < 20; ++i) {
        vector<int> S = save_start_S;

        while (S.size() > 0) {
            bool was_possible = false;
            for (int i = 0; i < 10; ++i) {
                auto node = S[rand()%S.size()];
                auto G_minus_S_vertex_set = SetSubstract(GetAllExistingNodes(), S);
                auto when_node_added = SetUnion(G_minus_S_vertex_set, vector<int>{node});
                MaxCutGraph G_minus_newS(*this, when_node_added);
                
                if (G_minus_newS.IsCliqueForest()) {
                    S.erase(std::remove(S.begin(), S.end(), node), S.end());
                    was_possible = true;
                    break;
                }
            }
            for (int node : S) {
                auto G_minus_S_vertex_set = SetSubstract(GetAllExistingNodes(), S);
                auto when_node_added = SetUnion(G_minus_S_vertex_set, vector<int>{node});
                MaxCutGraph G_minus_newS(*this, when_node_added);
                
                if (G_minus_newS.IsCliqueForest()) {
                    S.erase(std::remove(S.begin(), S.end(), node), S.end());
                    was_possible = true;
                    break;
                }
            }
            if (!was_possible) break;
        }


        if (S.size() < ret) ret = S.size();
     //   cout << S.size() << " ";
    }//cout<<endl;
    return ret;
}

int MaxCutGraph::ComputeOptimalColoringBruteforce(const vector<int>& S) {
    int mx_sol = 0;
    for (int mask = 0; mask < (1 << S.size()); ++mask) {
        vector<int> s_color;
        for (unsigned int i = 0; i < S.size(); ++i)
            if (mask & (1<<i)) s_color.push_back(1);
            else s_color.push_back(0);
        
        auto sol = MaxCutExtension(S, s_color);
        mx_sol = max(mx_sol, get<0>(sol));
        if (mx_sol == get<0>(sol))
            computed_maxcut_coloring = get<1>(sol);
    }
    return mx_sol;
}

int MaxCutGraph::ComputeOptimalColoring(const vector<int>& S, const vector<int>& S_color) {
    (void) S;
    (void) S_color;

    for (unsigned i = 0; i < S.size(); ++i) {
        auto adj = g_adj_list[S[i]];
        

        vector<int> adjs;
        for (unsigned i = 0; i < adj.size(); ++i)
            if (std::find(S.begin(), S.end(), adj[i]) != S.end()) adjs.push_back(adj[i]);

        int adjsz  = adjs.size();
        cout << S[i] << " = " << adjsz << " ::: ";
        
        for (unsigned j = 0; j < adjs.size(); ++j)
            cout << adjs[j] << " ";
        cout << endl;
    }

    return -1;
}

vector<int> MaxCutGraph::GetAClique(const int min_size, const int runs, const bool make_maximum) {
    vector<int> max_clique;
    for (int r = 0; r < runs; ++r) {
        vector<int> current_v = GetAllExistingNodes();
        vector<int> clique;

        while (current_v.size() > 0 && (make_maximum || (int)clique.size() < min_size)) {
            int w = current_v[rand()%current_v.size()];
            clique.push_back(w);
            auto adj = GetAdjacency(w);
            current_v = SetIntersection(current_v, adj);
        }

        if ((int)clique.size() >= min_size && min_size != -1) return clique;
        else if (clique.size() > max_clique.size()) max_clique = clique;
    }

    return max_clique;
}


vector<vector<int>> MaxCutGraph::GetAllR8Candidates(const unordered_map<int,bool>& preset_is_external) {
    vector<vector<int>> ret;
    
    auto prune_candidates = [&](vector<int> marked, vector<int> candidates) {
        vector<int> ret;
        for (auto node : candidates) {
            auto adj = GetAdjacency(node);
            auto adj_goal = GetAdjacency(marked[0]);
            adj_goal = SetSubstract(adj_goal, vector<int>{node});
            adj_goal = SetUnion(adj_goal, vector<int>{marked[0]});
            if (adj.size() == adj_goal.size() && SetIntersection(adj, adj_goal).size() == adj.size())
                ret.push_back(node);
        }
        return ret;
    };

    vector<bool> visited(num_nodes, false);
    vector<int> current_v = GetAllExistingNodes();

    for (auto root : current_v) {
        if (visited[root]) continue;

        vector<int> marked = vector<int>{root};
        vector<int> candidates = prune_candidates(marked, GetAdjacency(root));
        
        marked = SetUnion(marked, candidates);
        if (marked.size() > 1 && marked.size() > ((int)GetAdjacency(root).size()) - (marked.size() - 1)) { // -1 since root included here.
            bool ok = true;
            for (auto node : marked)
                if (KeyExists(node, preset_is_external))
                    ok = false;

            if (ok) ret.push_back(marked);
            
            for (auto node : marked)
                visited[node] = true;
        }
    }

    return ret;
}

void MaxCutGraph::ApplyR8Candidate(const vector<int>& clique, double &cut_change) {
    assert(clique.size() >= 2);

    int frem = rand() % clique.size();
    int srem = (frem + 1) % clique.size();
    int rem_node1 = clique[frem], rem_node2 = clique[srem];

    cut_change -= (2*GetAdjacency(rem_node1).size() + 1) / 4.0;
    RemoveNode(rem_node1);

    cut_change -= (2*GetAdjacency(rem_node2).size() + 1) / 4.0;
    RemoveNode(rem_node2);
}

vector<pair<int,vector<pair<int,int>>>> MaxCutGraph::GetAllR9Candidates() {
    vector<pair<int,vector<pair<int,int>>>> ret;
    vector<int> current_v = GetAllExistingNodes();
    vector<bool> makes_nonspecial(num_nodes + 1, false);

    for (auto root : current_v) {
        auto adj = GetAdjacency(root);
        if (adj.size() == 2) {
            makes_nonspecial[root] = true;
        }
    }

    vector<vector<pair<int,int>>> pairing(num_nodes + 1);
    for (auto root : current_v) {
        auto adj = GetAdjacency(root);
        for (auto na : adj) {
            for (auto nb : adj) {
                if (na >= nb || edge_exists_lookup[make_pair(na,nb)] == false)
                    continue;

                // skipped case Cint = 3, Cext = 0. We can't have a shared vertex among two triag-blocks hereby.
                
                // C_{int} intersect N_{G}(S) is empty.
                if (makes_nonspecial[na] == false && makes_nonspecial[nb] == false) // <=> !(mnonspec[na] || mnonspecial[nb]) => Cext >= 2
                    continue;
                
                pairing[root].push_back(make_pair(na, nb));
            }
        }
    }

    for (auto root : current_v) {
        auto& tris = pairing[root];
        for (int i = 0; i < (int)tris.size(); ++i) {
            for (int j = i + 1; j < (int)tris.size(); ++j) {
                auto  &b1 = tris[i], &b2 = tris[j];

                bool ok = true;
                for (int x1 : {b1.first, b1.second})
                    for (int x2: {b2.first, b2.second})
                        if (edge_exists_lookup[make_pair(x1,x2)])
                            ok = false;

                if (!ok) continue;
                ret.push_back(make_pair(root, vector<pair<int,int>>{b1,b2}));
            }
        }
    }

    return ret;
}

void MaxCutGraph::ApplyR9Candidate(const pair<int,vector<pair<int,int>>> &candidate, double &cut_change) {
    const pair<int,int> &triag1 = candidate.second[0];
    const pair<int,int> &triag2 = candidate.second[1];
    AddEdge(triag1.first, triag2.first);
    AddEdge(triag1.first, triag2.second);

    AddEdge(triag1.second, triag2.first);
    AddEdge(triag1.second, triag2.second);

    cut_change += 2; // increases (4 edges / 2 in EE)
}

vector<pair<vector<int>, vector<int>>> MaxCutGraph::GetAllR9XCandidates() {
    vector<pair<vector<int>, vector<int>>> ret;

    vector<bool> visited(num_nodes, false);
    vector<int> current_v = GetAllExistingNodes();
    for (auto root : current_v) {
        if (visited[root]) continue;

        vector<int> clique = GetAdjacency(root);
        clique.push_back(root);

        if (IsClique(clique) == false) continue; // root not in Cint

        vector<int> X;
        for (auto x : clique) {
            auto adj = GetAdjacency(x);
            auto inter = SetIntersection(clique, adj);
            if (adj.size() == inter.size()) { // all adjacent vertices of x are in clique.
                X.push_back(x);
            }
        }

        if (clique.size() % 2 == 0 && clique.size() / 2 <= X.size()) {
            for (auto x : X)
                visited[x] = true;
            ret.push_back(make_pair(clique, X));
        }
    }

    return ret;
}
void MaxCutGraph::ApplyR9XCandidate(const pair<vector<int>, vector<int>>& candidate, double &cut_change) {
    assert(candidate.second.size() >= 1);

    int rem_node = candidate.second[rand() % candidate.second.size()];
    cut_change -= (2*GetAdjacency(rem_node).size() + 2 /* +1 for cut change, +1 for removal of node in EE */) / 4.0; // why is this an integer? GetAdjacency(rem_node).size() is odd, since clique is even and doesnt contain itself.

    RemoveNode(rem_node);
}

vector<tuple<bool, int, int, int>> MaxCutGraph::GetAllR10Candidates(const unordered_map<int,bool>& preset_is_external) {
    vector<tuple<bool, int, int, int>> ret;

    vector<int> current_v = GetAllExistingNodes();
    for (auto u : current_v) {
        auto adj = GetAdjacency(u);

        if (adj.size() != 2) continue;

        int nodex = adj[0], nodey = adj[1];

        if (AreAdjacent(nodex, nodey) == false) continue;

        auto new_v = SetSubstract(current_v, vector<int>{u});
        MaxCutGraph newG(*this, new_v);

        bool bridge_case = false;
        if (newG.IsBridgeBetween(nodex, nodey)) {
            newG.RemoveEdgesBetween(nodex, nodey);
            
            auto has_external = [&](const vector<int> &component) {
                for (auto node : component)
                    if (KeyExists(node, preset_is_external))
                        return true;
                return false;
            };

            auto C1 = newG.GetConnectedComponentOf(nodex);
            auto C2 = newG.GetConnectedComponentOf(nodey);
            
            if (has_external(C1) == false || has_external(C2) == false) {
                ret.push_back(make_tuple(true, u, nodex, nodey));
                bridge_case = true;
            }
        }

        if (!bridge_case) {
            ret.push_back(make_tuple(false, u, nodex, nodey));
        }
    }
    
    return ret;
}
void MaxCutGraph::ApplyR10Candidate(const tuple<bool, int, int, int>& candidate, double &k) {
    bool bridge_case = get<0>(candidate);
    int u = get<1>(candidate);
    int nodex = get<2>(candidate);
    int nodey = get<3>(candidate);

    if (bridge_case) {
        vector<int> adj_nodex = GetAdjacency(nodex);
        vector<int> adj_nodey = GetAdjacency(nodey);
        vector<int> adj = SetUnion(adj_nodex, adj_nodey);

        k -= (2 * GetAdjacency(u).size() + 1) / 4.0;
        RemoveNode(u);

        k -= (2 * GetAdjacency(nodex).size() + 1) / 4.0;
        RemoveNode(nodex);

        k -= (2 * GetAdjacency(nodey).size() + 1) / 4.0;
        RemoveNode(nodey);

        k += 1 / 4.0;
        ReAddNode(u);

        for (auto v : adj) {
            if (v != u && v != nodex && v != nodey) {
                k += 1 / 2.0;
                AddEdge(u, v);
            }
        }
    } else {
        k -= (2 * GetAdjacency(u).size() + 2 /* +1 for node (/4 before already) and +1 for the change in cut (/4 before already) */) / 4.0; // k-- in EE included here.
        RemoveNode(u);

        k -= 1 / 2.0; // m/2 in EE
        RemoveEdgesBetween(nodex, nodey);
    }
}

// O(|V| + |E|)
vector<tuple<int,int,int,int,int>> MaxCutGraph::GetAllR10ASTCandidates(const unordered_map<int,bool>& preset_is_external) {
    vector<tuple<int,int,int,int,int>> ret;

    vector<int> current_v = GetAllExistingNodes();
    for (auto b : current_v) {
        auto b_adj = GetAdjacency(b);
        if (b_adj.size() != 2 || KeyExists(b, preset_is_external)) continue;

        int a = b_adj[0], c = b_adj[1];
        auto a_adj = GetAdjacency(a), c_adj = GetAdjacency(c);
        if (a_adj.size() != 2 || c_adj.size() != 2 || KeyExists(a, preset_is_external) || KeyExists(c, preset_is_external)) continue;

        int ex_L = a_adj[0] != b ? a_adj[0] : a_adj[1];
        int ex_R = c_adj[0] != b ? c_adj[0] : c_adj[1];

        if (ex_L == a || ex_L == b || ex_L == c) continue;
        if (ex_R == a || ex_R == b || ex_R == c) continue;
        if (ex_L == ex_R) continue; // ------------------------------ THIS COULD! BE SUPPORTED, BUT REQUIRED DOUBLE EDGES. USED TO BE A BUG BECAUSE THIS CONDITION MISSED.

        ret.push_back(make_tuple(ex_L, a, b, c, ex_R));
    }
    return ret;
}
void MaxCutGraph::ApplyR10ASTCandidate(const tuple<int,int,int,int,int>& candidate, double &cut_change) {
    int ex_L = get<0>(candidate), a = get<1>(candidate), b = get<2>(candidate),
           c = get<3>(candidate), ex_R = get<4>(candidate);

    RemoveNode(a);
    RemoveNode(c);
    AddEdge(ex_L, b);
    AddEdge(b, ex_R);
    cut_change -= 2;
}

// Interesting facts on this rule:
// -- in case of RGG graphs: manages to process ALL cliques with at least one internal vertex -- therefore, cliques tend to be small.
// 
vector<vector<int>> MaxCutGraph::GetS2Candidates(const bool break_on_first, const unordered_map<int,bool>& preset_is_external) {
    vector<vector<int>> ret;

    auto current_v = GetAllExistingNodes();
    for (auto root : current_v) { // an internal vertex
        if (KeyExists(root, preset_is_external)) continue;

        const auto adj_root = GetAdjacency(root);
        vector<int> curr_clique = SetUnion(adj_root, {root});

        if (!IsClique(curr_clique))
            continue;

        vector<int> externals;
        for (auto node : curr_clique) {
            const auto adj = GetAdjacency(node);
            if (adj.size() + 1 != curr_clique.size() || KeyExists(node, preset_is_external))
                externals.push_back(node);
        }

    //    cout << "CLIQUE WITH AT LEAST ON INTERNAL: " << curr_clique.size() << " " << externals.size() << endl;

        if (externals.size() <= ((curr_clique.size() >> 1) + (curr_clique.size() % 2))) {
            ret.push_back(curr_clique);
            if (break_on_first) return ret;
        }


        // there cant be more, add proof in thesis.
    }

    return ret;
}
void MaxCutGraph::ApplyS2Candidate(const vector<int>& clique, double &cut_change, const unordered_map<int,bool>& preset_is_external) {// Clique cut.
    int n = clique.size();
    int add_tot = 0;
    for (int i = 1; i <= n; ++i) {
        int add = (n - i) - (i - 1);
        if (add <= 0) break;
        add_tot += add;
    }
    cut_change -= add_tot;
    
    // Apply changes to graph:
    vector<int> rem_nodes;
    for (auto node : clique) {   
        const auto adj = GetAdjacency(node);
        if (adj.size() + 1 == clique.size() && !KeyExists(node, preset_is_external)) { // is internal
            rem_nodes.push_back(node);
        }
    }

    for (auto node : clique) {   
        for (auto node2 : clique) {
            if (edge_exists_lookup[make_pair(node, node2)]) {
                RemoveEdgesBetween(node, node2);
            }
        }
    }

    for (auto node : rem_nodes)
        RemoveNode(node);
}

vector<vector<int>> MaxCutGraph::GetS3Candidates(const bool break_on_first, const unordered_map<int,bool>& preset_is_external) {
    vector<vector<int>> ret;

    auto current_v = GetAllExistingNodes();
    for (auto root : current_v) { // an internal vertex
        const auto adj_root = GetAdjacency(root);
        if (KeyExists(root, preset_is_external) || adj_root.size() == 0) continue;

        
        vector<int> clique = SetUnion(adj_root, {root});
        
        auto missingnodes = GetAdjacency(adj_root[0]);
        for (auto node : adj_root)
            missingnodes = SetIntersection(missingnodes, GetAdjacency(node));
        
        missingnodes = SetSubstract(missingnodes, clique);
        if (missingnodes.empty()) continue;
        clique = SetUnion(clique, {missingnodes[0]});

        bool ok = true;

        vector<int> externals;
        unordered_map<int,bool> is_external = preset_is_external;
        for (auto node : clique) {
            const auto adj = GetAdjacency(node);
            const auto sub = SetSubstract(adj, clique);
            if (sub.empty() == false || KeyExists(node, preset_is_external)) {
                externals.push_back(node);
                is_external[node] = true;
            }
        }

        int missing_edges_cnt = 0;
        for (int i = 0; i < (int)clique.size() && ok; ++i) {
            for (int j = i + 1; j < (int)clique.size() && ok; ++j) {
                if (AreAdjacent(clique[i], clique[j]) == false) {
                    missing_edges_cnt++;

                    if (is_external[clique[i]] || is_external[clique[j]])
                        ok = false;
                }
            }
        }

        ok = ok && (missing_edges_cnt == 1);
        if (clique.size() % 2 == 0 && (int)externals.size() == ((int)clique.size()) - 2)
            ok = false;

        if (ok) {
            ret.push_back(clique);
            if (break_on_first)
                break;
        }
    }

    return ret;
}

void MaxCutGraph::ApplyS3Candidate(const vector<int>& clique, double &cut_change, const unordered_map<int,bool>& preset_is_external) {
    (void) cut_change;
    (void) preset_is_external;

    for (int i = 0; i < (int)clique.size(); ++i)
        for (int j = i + 1; j < (int)clique.size(); ++j)
            if (AreAdjacent(clique[i], clique[j]) == false)
                AddEdge(clique[i], clique[j]);
}

vector<tuple<bool,int,int,int,int>> MaxCutGraph::GetAllS4Candidates(const unordered_map<int,bool>& preset_is_external) {
    vector<tuple<bool,int,int,int,int>> ret;
    auto current_v = GetAllExistingNodes();
    for (int i = 0; i < (int)current_v.size(); ++i) {
        for (int j = i + 1; j < (int)current_v.size(); ++j) {
            int nodeA = current_v[i];
            int nodeB = current_v[j];
            if (AreAdjacent(nodeA, nodeB)) continue;

            const auto adjA = GetAdjacency(nodeA);
            int nodeC = -1, nodeD = -1;
            bool ok = false;
            for (int k = 0; k < (int)adjA.size() && !ok; ++k) {
                for (int l = k + 1; l < (int)adjA.size() && !ok; ++l) {
                    nodeC = adjA[k], nodeD = adjA[l];
                    if (KeyExists(nodeC, preset_is_external)) continue;
                    if (KeyExists(nodeD, preset_is_external)) continue;
                    if (AreAdjacent(nodeB, nodeC) && AreAdjacent(nodeB, nodeD))
                        ok = true;
                }
            }

            if (!ok) continue;

            if (Degree(nodeC) == 2 && Degree(nodeD) == 2 && ((KeyExists(nodeA, preset_is_external) == false && Degree(nodeA) == 2)
                                                          || (KeyExists(nodeB, preset_is_external) == false && Degree(nodeB) == 2)))
                ret.push_back(make_tuple(0, nodeA, nodeB, nodeC, nodeD));
            else if (Degree(nodeC) == 3 && Degree(nodeD) == 3 && AreAdjacent(nodeC, nodeD))
                ret.push_back(make_tuple(1, nodeA, nodeB, nodeC, nodeD));
        }
    }
    return ret;
}
void MaxCutGraph::ApplyS4Candidate(tuple<bool,int,int,int,int> &candidate, double &cut_change) {
    bool type = get<0>(candidate);
    int nodeA = get<1>(candidate);
    int nodeB = get<2>(candidate);
    int nodeC = get<3>(candidate);
    int nodeD = get<4>(candidate);

    if (type == 0) {
        RemoveEdgesBetween(nodeA, nodeC);
        RemoveEdgesBetween(nodeA, nodeD);
        RemoveEdgesBetween(nodeB, nodeC);
        RemoveEdgesBetween(nodeB, nodeD);
        cut_change -= 4;
    } else {
        RemoveNode(nodeC); // or nodeD
        cut_change -= 2;
    }
}

// O(|V| + |E|)
vector<tuple<int,int,int,int>> MaxCutGraph::GetAllS5Candidates(const unordered_map<int,bool>& preset_is_external) {
    vector<tuple<int,int,int,int>> ret;

    vector<int> current_v = GetAllExistingNodes();
    for (auto a : current_v) {
        const auto a_adj = GetAdjacency(a);
        if (a_adj.size() != 2 || KeyExists(a, preset_is_external)) continue;

        int b = a_adj[0], ex_L = a_adj[1];
        if (Degree(b) != 2 ||  KeyExists(b, preset_is_external)) swap(b, ex_L);
        if (Degree(b) != 2 ||  KeyExists(b, preset_is_external)) continue;

        const auto b_adj = GetAdjacency(b);
        int ex_R = b_adj[0];
        if (ex_R == a) ex_R = b_adj[1];

        if (ex_L == ex_R || AreAdjacent(ex_L, ex_R)) continue; // ------------------------------ THIS COULD! BE SUPPORTED, BUT REQUIRED DOUBLE EDGES. USED TO BE A BUG BECAUSE THIS CONDITION MISSED.

        ret.push_back(make_tuple(ex_L, a, b, ex_R));
    }
    return ret;
}
void MaxCutGraph::ApplyS5Candidate(const tuple<int,int,int,int>& candidate, double &cut_change) {
    int ex_L = get<0>(candidate), a = get<1>(candidate), b = get<2>(candidate),
        ex_R = get<3>(candidate);

    RemoveNode(a);
    RemoveNode(b);
    AddEdge(ex_L, ex_R);
    cut_change -= 2;
}

void MaxCutGraph::ExecuteExhaustiveKernelization() {
    double k_change = 0;
    while (true) {
        auto res_rs2 = GetS2Candidates(true);
        if (!res_rs2.empty()) {
            ApplyS2Candidate(res_rs2[0], k_change);
            continue;
        }

        auto res_r9x = GetAllR9XCandidates();
        if (!res_r9x.empty()) {
            ApplyR9XCandidate(res_r9x[0], k_change);
            continue;
        }
        
        auto res_r8 = GetAllR8Candidates();
        if (!res_r8.empty()) {
            ApplyR8Candidate(res_r8[0], k_change);
            continue;
        }
        
        auto res_r10 = GetAllR10Candidates();
        if (!res_r10.empty()) {
            ApplyR10Candidate(res_r10[0], k_change);
            continue;
        }
        
        auto res_r9 = GetAllR9Candidates();
        if (!res_r9.empty()) {
            ApplyR9Candidate(res_r9[0], k_change);
            continue;
        }

        auto res_r10ast = GetAllR10ASTCandidates();
        if (!res_r10ast.empty()) {
            ApplyR10ASTCandidate(res_r10ast[0], k_change);
            continue;
        }
        
        break;
    }
}

void MaxCutGraph::ExecuteExhaustiveKernelizationExternalsSupport(const unordered_map<int,bool> &preset_is_external) {
    double k_change = 0;
    while (true) {
        
        auto res_rs2 = GetS2Candidates(true, preset_is_external);
        if (!res_rs2.empty()) {
            ApplyS2Candidate(res_rs2[0], k_change, preset_is_external);
            continue;
        }

        auto res_s3 = GetS3Candidates(true, preset_is_external);
        if (!res_s3.empty()) {
            ApplyS3Candidate(res_s3[0], k_change, preset_is_external);
            continue;
        }

        auto res_r10ast = GetAllR10ASTCandidates(preset_is_external);
        if (!res_r10ast.empty()) {
            ApplyR10ASTCandidate(res_r10ast[0], k_change);
            continue;
        }

        auto res_r8 = GetAllR8Candidates(preset_is_external);
        if (!res_r8.empty()) {
            ApplyR8Candidate(res_r8[0], k_change);
            continue;
        }

        auto res_s4 = GetAllS4Candidates(preset_is_external);
        if (!res_s4.empty()) {
            ApplyS4Candidate(res_s4[0], k_change);
            continue;
        }

      
        auto res_s5 = GetAllS5Candidates(preset_is_external);
        if (!res_s5.empty()) {
            ApplyS5Candidate(res_s5[0], k_change);
            continue;
        }
        
        break;
    }
}

vector<vector<int>> MaxCutGraph::DecomposeIntoCliques() {
    vector<vector<int>> ret;
    vector<int> current;

    int cnt_extra_edges = 0;
    unordered_map<int,int> is_current;
    int cdx = 1;
    int sz = 50;
    while ((int)current.size() != num_nodes) {
        vector<int> clique = GetAClique(max(sz--,2), 10, false);

        cout << clique.size() << " : ";
        for (auto node : clique)
            cout << node << " ";
        cout << endl;

        vector<int> clique_intersect = SetIntersection(clique, current);

        if (clique_intersect.size() == clique.size()) continue;

        unordered_map<int,bool> is_intersect;
        
        for (auto node : clique_intersect) {
            is_intersect[node] = true;
        }

        int mem_cnt_extra_edges = cnt_extra_edges;
        int typec = 0;
        for (auto node : clique) {
            if (is_intersect[node]) continue;

            auto adj = GetAdjacency(node);
            for (auto w : adj) {
                if (is_current[w] != 0) {
                    cnt_extra_edges ++;
                    if (typec == 0) typec = is_current[w];
                    else if (typec != is_current[w]) typec = -1;
                }
            }
        }

        //if (cnt_extra_edges > 0) {
        //    cnt_extra_edges = 0;
        //    continue;
        //}
        if (typec == -1 || cnt_extra_edges > 0) {
            cnt_extra_edges = mem_cnt_extra_edges;
            continue;
        }

        ret.push_back(clique);
        current = SetUnion(current, clique);
        cout << "ADD " << clique.size() << " , intersecting @ " << clique_intersect.size() << " vertices. New cover size: " << current.size() << endl;
        for (auto node : clique)
            cout << node << " ";
        cout << endl;
        cout << "Extra edges between cliques: " << cnt_extra_edges << endl;

        for (auto node : clique)
            is_current[node] = cdx;
        cdx++;
        cout << endl;
    }
    

    return ret;
}

void MaxCutGraph::PrintGraph(std::ostream& out) {
    out << num_nodes << " " << GetRealNumEdges() << endl;

    for (int i = 0; i < num_nodes; ++i) {
        for (auto node : g_adj_list[i]) {
            if (i < node)
                out << i + 1<< " " << node + 1<< " " << 1 << endl;
        }
    }
}

int MaxCutGraph::GetCutSize(const vector<int> &grouping) {
    int ret = 0;
    for (int i = 0; i < num_nodes; ++i) {
        if (removed_node[i]) continue;

        int adj0 = 0, adj1 = 0;
        auto adj = GetAdjacency(i);
        for (auto x : adj) {
            if (removed_node[x] || i >= x) continue;

            adj0 += grouping[x] == 0;
            adj1 += grouping[x] == 1;
        }

        ret += (grouping[i] == 0) * adj1;
        ret += (grouping[i] == 1) * adj0;
    }

    return ret;
}

// http://pages.cs.wisc.edu/~shuchi/courses/880-S07/scribe-notes/lecture07.pdf
pair<int, vector<int>> MaxCutGraph::ComputeLocalSearchCut(const vector<int> pregroup) {
    assert((int)pregroup.size() == num_nodes || pregroup.empty());
    
    vector<int> grouping(num_nodes, -1);
    for (int i = 0; i < num_nodes; ++i) {
        grouping[i] = rand()%2;
        if ((int)pregroup.size() == num_nodes && pregroup[i] != -1)
            grouping[i] = pregroup[i];
    }

    bool change = true;
    while (change) {
        change = false;
        for (int i = 0; i < num_nodes; ++i) {
            if (removed_node[i]) continue;

            int adj0 = 0, adj1 = 0;
            auto adj = GetAdjacency(i);
            for (auto x : adj) {
                if (removed_node[x]) continue;

                adj0 += grouping[x] == 0;
                adj1 += grouping[x] == 1;
            }

            if (adj0 > adj1 && grouping[i] != 1) {
                change = true;
                grouping[i] = 1;
            }

            if (adj1 > adj0 && grouping[i] != 0) {
                change = true;
                grouping[i] = 0;
            }
        }
    }

    return make_pair(GetCutSize(grouping), grouping);
}

// https://github.com/MQLib/MQLib
pair<int, vector<int>> MaxCutGraph::ComputeMaxCutHeuristically() {
    std::vector<Instance::InstanceTuple> edgeList;
    for (int i = 0; i < num_nodes; ++i) {
        if (removed_node[i]) continue;

        auto adj = GetAdjacency(i);
        for (auto w : adj) {
            if (removed_node[w] || i >= w) continue;
            assert(edge_exists_lookup[make_pair(w, i)]);
            edgeList.push_back(Instance::InstanceTuple(std::make_pair(i+1, w+1), 1));
        }
    }

    MaxCutInstance mi(edgeList, num_nodes + 1);
    Burer2002 heur(mi, 1, false, NULL);
    const MaxCutSimpleSolution& mcSol = heur.get_best_solution();

    return make_pair(mcSol.get_weight(), mcSol.get_assignments());
}


// O(|V| + |E|)
vector<vector<int>> MaxCutGraph::GetCliquesWithAtLeastOneInternal() {
    vector<vector<int>> ret;
    vector<bool> visited(num_nodes, false);

    auto cmp = [&](int a, int b) {
        return g_adj_list[a].size() < g_adj_list[b].size();
    };

    auto current_v = GetAllExistingNodes();
    sort(current_v.begin(), current_v.end(), cmp);

    for (auto root : current_v) { // an internal vertex
        if (visited[root])
            continue;

        const auto adj_root = GetAdjacency(root);
        vector<int> clique = SetUnion(adj_root, {root});

        for (auto node : clique)
            visited[node] = true;

        if (!IsClique(clique))
            continue;

        ret.push_back(clique);
    }

    return ret;
}