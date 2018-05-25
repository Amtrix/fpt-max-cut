#include "mc-graph.hpp"



 MaxCutGraph::MaxCutGraph() {

 }

 MaxCutGraph::MaxCutGraph(int n, int m) {
    num_nodes = n, num_edges = m;
    g_adj_list.resize(num_nodes);
}

MaxCutGraph::MaxCutGraph(const string path) {
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

MaxCutGraph::MaxCutGraph(const MaxCutGraph& source, const vector<int>& subset) : MaxCutGraph(source.GetNumNodes(), -1) {
    int source_num_nodes = source.GetNumNodes();

    for (int i = 0; i < source_num_nodes; ++i)
        removed_node[i] = true;
    
    for (const int node : subset)
        removed_node[node] = false;

    num_edges = 0;
    for (const int node : subset) {
        auto adj = source.GetAdjacency(node);
        for (const int w : adj) {
            if (removed_node[w] || edge_exists_lookup[make_pair(node, w)])
                continue;

            AddEdge(node, w);
            num_edges++;
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

double MaxCutGraph::GetEdwardsErdosBound() const {
    return (num_edges / 2.0) + (num_nodes - 1) / 4.0;
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

int MaxCutGraph::ComputeCut(const vector<int>& S, const vector<int>& S_color) {
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
    while(1) {
        vector<int> leaf_block;
        int r;

        tie(leaf_block, r) = G_minus_S.GetLeafBlockAndArticulation(true);
        
        if (r == -1)
            break;

        leaf_block = SetSubstract(leaf_block, vector<int>{r}); // make X U {r} represent whole block as in paper. X U {r} is clique (!!!!!!).
        const int block_size = leaf_block.size();

        vector<pair<int,int>> eps;
        int w0_sum, w1_sum;
        ////////////////////////////// REDUCE CODE BELLOW ?//////////////////////////////
        // try color(r) == 1
        w0_sum  = 0;
        for (int node : leaf_block) {
            int w0 = weight[0][node], w1 = weight[1][node];
            eps.push_back(make_pair(w1 - w0, node));
            w0_sum += w0;
           // cout << "leaf " << node << " = " << weight[0][node] << " " << weight[1][node] << endl;
        }
        sort(eps.rbegin(), eps.rend());

        int V1 = weight[1][r] + w0_sum + block_size; // assume all in X are set to 0
        int all_to_all_flip_add = 0;
       // cout << "pre_v1 = " << V1 << endl;
        w1_sum = 0;
        for (unsigned int i = 0; i < eps.size(); ++i) {
            auto entry = eps[i];
            int w0 = weight[0][entry.second], w1 = weight[1][entry.second];
            
            // we now color entry.second to '1'
            w0_sum -= w0;
            w1_sum += w1;

            all_to_all_flip_add += block_size - (i + 1);
            int V1_check = weight[1][r] + w1_sum + w0_sum + (block_size - (i + 1)) + all_to_all_flip_add;
           // cout << weight[1][r] << " " << w1_sum << " " << w0_sum << " " << block_size << " " << (i+1) << " = " << V1_check << " ( " << eps.size() << endl;
            V1 = max(V1_check, V1);
        }
        ////////////////// done with color(r) == 1

        // try color(r) == 0
        w1_sum = 0;
        eps.clear();
        for (int node : leaf_block) {
            int w0 = weight[0][node], w1 = weight[1][node];
            eps.push_back(make_pair(w0 - w1, node));
            w1_sum += w1;
        }
        sort(eps.rbegin(), eps.rend());

        int V0 = weight[0][r] + w1_sum + block_size; // assume all in X are set to 1
       // cout << "pre_v0 = " << V0 << endl;
        w0_sum = 0;
        all_to_all_flip_add = 0;
        for (unsigned int i = 0; i < eps.size(); ++i) {
            auto entry = eps[i];
            int w0 = weight[0][entry.second], w1 = weight[1][entry.second];
            
            // we now color entry.second to '1'
            w0_sum += w0;
            w1_sum -= w1;

            all_to_all_flip_add += block_size - (i + 1);
            int V0_check = weight[0][r] + w1_sum + w0_sum + (block_size - (i + 1)) + all_to_all_flip_add;
            //cout << entry.second << ". " << weight[0][r] << " " << w1_sum << " " << w0_sum << " " << block_size << " " << (i+1) << " = " << V0_check << " ( " << eps.size() << endl;
            V0 = max(V0_check, V0);
        }
        ////////////////// done with color(r) == 0
       // cout << "prv: " << weight[0][r] << " " << weight[1][r] << endl;
        weight[0][r] = V0;
        weight[1][r] = V1;
        //cout << r << " = " << V0 << " " << V1 << endl;
        auto allv = G_minus_S.GetAllExistingNodes();
        auto nextv = SetSubstract(allv, leaf_block);
        G_minus_S = MaxCutGraph(G_minus_S, nextv);
    }

    int sol = 0;
    auto bicomponents = G_minus_S.GetBiconnectedComponents();
    for (auto component : bicomponents) {
        assert(component.size() == 1);
        int u = component[0];
        sol += max(weight[0][u], weight[1][u]);
    }

    return sol + p;
}