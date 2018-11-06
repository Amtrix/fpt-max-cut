#include <iostream>
#include <cassert>
#include <algorithm>
#include <stack>
#include <fstream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>

#include "mc-graph.hpp"
#include <heuristics/qubo/glover1998a.h>
#include <heuristics/maxcut/burer2002.h>



const map<RuleIds, string> kRuleDescriptions = {
    {RuleIds::SpecialRule1,    "Special Reduction Rule for handling weighted<->unweighted: Compresses paths of length 3 to a single edge."},
    {RuleIds::SpecialRule2,    "Special Reduction Rule for handling weighted<->unweighted: Compresses paths of length 2 to a single edge."},
    {RuleIds::RevSpecialRule1, "Reversed Special Reduction Rule for handling weighted<->unweighted."},
    {RuleIds::RevSpecialRule2, "Reversed Special Reduction Rule for handling weighted<->unweighted."},
    {RuleIds::Rule8,           "Reduction Rule for handling cliques with uniform neighbhor set. Rule 8 in linear kernel paper."},
    {RuleIds::Rule9,           "Reduction Rule for handling special triangles sharing one common vertex. Rule 9 in linear kernel paper."},
    {RuleIds::Rule9X,          "Reduction Rule for handling cliques in cliques with > /2 size. Rule 9 in max-balanced-subgraph paper."},
    {RuleIds::Rule10,          "Reduction Rule for handling special bridge induced by a single vertex 'u'. Rule 10 in max-balanced-subgraph paper."},
    {RuleIds::Rule10AST,       "Reduction Rule for handling induced paths of length 4. Rule 10 in AST paper."},
    {RuleIds::RuleS2,          "Selfmade Reduction Rule for handling cliques with <= n/2 external vertices."},
    {RuleIds::RuleS3,          "Selfmade Reduction Rule for handling cliques with an missing edge -> adds it to it."},
    {RuleIds::RuleS4,          "Selfmade Reduction Rule for handling two quads cases."},
    {RuleIds::RuleS5,          "Selfmade Reduction Rule for handling induced paths of length 3."},
    {RuleIds::RuleS6,          "?????????????????"},
};

const map<RuleIds, string> kRuleNames = {
    {RuleIds::SpecialRule1,    "SpecialRule1"},
    {RuleIds::SpecialRule2,    "SpecialRule2"},
    {RuleIds::RevSpecialRule1, "RevSpecialRule1"},
    {RuleIds::RevSpecialRule2, "RevSpecialRule2"},
    {RuleIds::Rule8,           "Rule8"},
    {RuleIds::Rule9,           "Rule9"},
    {RuleIds::Rule9X,          "Rule9X"},
    {RuleIds::Rule10,          "Rule10"},
    {RuleIds::Rule10AST,       "Rule10AST"},
    {RuleIds::RuleS2,          "RuleS2"},
    {RuleIds::RuleS3,          "RuleS3"},
    {RuleIds::RuleS4,          "RuleS4"},
    {RuleIds::RuleS5,          "RuleS5"},
    {RuleIds::RuleS6,          "RuleS6"},
};

const vector<RuleIds> kAllRuleIds = {
    RuleIds::SpecialRule1, RuleIds::SpecialRule2, RuleIds::RevSpecialRule1, RuleIds::RevSpecialRule2,
    RuleIds::Rule8, RuleIds::Rule9, RuleIds::Rule9X, RuleIds::Rule10, RuleIds::Rule10AST, RuleIds::RuleS2, RuleIds::RuleS3, RuleIds::RuleS4, RuleIds::RuleS5, RuleIds::RuleS6
};

struct trie_node_r8 {
    unordered_map<int, unique_ptr<trie_node_r8>> children;
    vector<int> elems;
};

struct trie_r8 {
    trie_node_r8* Traverse(const vector<int>& seq) {
        trie_node_r8 *curr = &root;
        for (unsigned int i = 0; i < seq.size(); ++i) {
            auto &children = curr->children;
            if (children.find(seq[i]) == children.end())
                children[seq[i]] = make_unique<trie_node_r8>();
            
            curr = children[seq[i]].get();
        }
        return curr;
    }

    void Insert(const vector<int>& seq, int elem) {
        trie_node_r8 *curr = Traverse(seq);
        curr->elems.push_back(elem);
    }

    const vector<int>& Retrieve(const vector<int>& seq) {
        trie_node_r8 *curr = Traverse(seq);
        return curr->elems;
    }

    trie_node_r8 root;
};

MaxCutGraph::MaxCutGraph() {

}

MaxCutGraph::MaxCutGraph(int n, int /* not used m */) {
    SetNumNodes(n);
}

MaxCutGraph::MaxCutGraph(const string path) {
    ifstream in(path.c_str());
    if (in.fail()) {
        throw std::logic_error("File doesn't exist.");
    }

    graph_naming = path;

    const string adj_sfx = ".graph";
    bool treat_as_adj_list_file = path.size() > adj_sfx.size() && path.substr(path.size() - adj_sfx.size()) == adj_sfx;

    vector<string> sparams = ReadLine(in);
    while (sparams.size() == 0 || sparams[0][0] == '#' || sparams[0][0] == '%') // skip comments
        sparams = ReadLine(in);

    // we take last two entries as dimacs prefixes each line with type of line
    if (!treat_as_adj_list_file) {
        if (sparams[0] != "#edge-list-0") {
            SetNumNodes(stoi(sparams[0 + (sparams[0]=="p")]));

            int num_edges = stoi(sparams.back());

            for (int i = 0; i < num_edges; ++i) {
                sparams = ReadLine(in);
                if (sparams.size() < 2) throw std::logic_error("Line malformed: " + to_string(i));

                int offset = sparams[0]=="e";
                int a = stoi(sparams[0 + offset]) - 1;
                int b = stoi(sparams[1 + offset]) - 1;
                int w = 2 + offset < (int)sparams.size() && sparams[2 + offset].size() > 0 ? stoi(sparams[2 + offset]) : 1;
                AddEdge(a, b, w, false);
            }
        } else {
            int num_nodes_calc = 0;
            vector<tuple<int,int,int>> elist;
            while (in.eof() == false) {
                sparams = ReadLine(in);
                if (sparams.size() == 0) continue;
                if (sparams.size() < 2) throw std::logic_error("Line malformed: " + to_string(-1));

                int a = stoi(sparams[0]);
                int b = stoi(sparams[1]);
                int w = 2 < sparams.size() && sparams[2].size() > 0 ? stoi(sparams[2]) : 1;
                num_nodes_calc = max(num_nodes_calc, max(a + 1, b + 1));
                elist.push_back(make_tuple(a, b, w));
            }
            
            SetNumNodes(num_nodes_calc);

            for (auto e : elist)
                AddEdge(get<0>(e), get<1>(e), get<2>(e), false);
        }
    } else {
        const bool is_weighted_instance = sparams.size() >= 3 && stoi(sparams[2]); 
        OutputDebugLog("Adjacency list. Is weighted: " + to_string(is_weighted_instance));

        SetNumNodes(stoi(sparams[0]));

        if (is_weighted_instance && stoi(sparams[2]) != 1) {
            OutputDebugLog("UNSUPPORTED FORMAT. Skipping.");
            return;
        }

        for (int i = 0; i < num_nodes; ++i) {
            sparams = ReadLine(in);

            if (sparams.size() % 2 != 0 && is_weighted_instance) {
                cout << "ERROR: ";
                for (unsigned int j = 0; j < sparams.size(); ++j)
                    cout << sparams[j] << " ";
                cout << endl;
                throw std::logic_error("Line malformed: " + to_string(i) + " -- odd count, but weighted instance.");
            }

            for (unsigned int j = 0; j < sparams.size(); j += 1 + is_weighted_instance) {
                int dest = stoi(sparams[j]) - 1;
                int weight = is_weighted_instance ? stoi(sparams[j + 1]) : 1;
                AddEdge(i, dest, weight, false);
            }
        }
    }
    
    OutputDebugLog("Reading from file done.");
}

MaxCutGraph::MaxCutGraph(const vector<tuple<int,int,int>> &elist, int n) {
    int num_nodes_calc = n;
    
    for (auto e : elist) {
        int a = get<0>(e);
        int b = get<1>(e);
        num_nodes_calc = max(num_nodes_calc, max(a + 1, b + 1));
    }

    SetNumNodes(num_nodes_calc);

    for (auto e : elist)
        AddEdge(get<0>(e), get<1>(e), get<2>(e));
}

MaxCutGraph::MaxCutGraph(const vector<pair<int,int>> &elist, int n) {
    vector<tuple<int,int,int>> nelist;
    for (auto e : elist)
        nelist.push_back(make_tuple(e.first, e.second, 1));
    MaxCutGraph(nelist, n);
}

MaxCutGraph::MaxCutGraph(const MaxCutGraph& source, const vector<int>& subset) : MaxCutGraph(source.GetNumNodes(), -1) {
    removed_node.clear();

    for (int i = 0; i < num_nodes; ++i) RemoveNode(i);
    for (const int node : subset) ReAddNode(node);

    for (const int node : subset) {
        auto &adj = source.GetAdjacency(node);
        for (const int w : adj) {
            if (w <= node || MapEqualCheck(removed_node, w, true)) // The added condition w >= node may actually influence results. It creates differently ordered adjacency lists in the graph.
                continue;
            
            AddEdge(node, w, source.GetEdgeWeight(make_pair(node, w)));
        }
    }
}

void MaxCutGraph::ResetComputedTopology() {
    articulations_computed = false;
    bicomponents_computed = false;
    bridges_computed = false;
}

void MaxCutGraph::SetNumNodes(int _num_nodes) {
    int prev_num_nodes = num_nodes;
    num_nodes = _num_nodes;
    g_adj_list.resize(num_nodes);
    current_timestamp.resize(num_nodes, 0);

    for (int i = prev_num_nodes; i < num_nodes; ++i)
        UpdateVertexTimestamp(i);
}

void MaxCutGraph::AddEdge(int a, int b, int weight, bool inc_weight_on_double) {
    auto keyAB = MakeEdgeKey(a,b), keyBA = MakeEdgeKey(b,a);

    if(edge_exists_lookup[keyAB]) {
        //OutputDebugLog("Warning: Multiple edges added between: " + to_string(a) + " and " + to_string(b) + ". Weight has been increased.");
        info_mult_edge++;

        if (inc_weight_on_double) {
            edge_weight[keyAB] += weight;
            edge_weight[keyBA] += weight;
            if (edge_weight[keyAB] == 0) RemoveEdgesBetween(a, b);
        }
        return;
    } else if (a == b) {
        throw std::logic_error("Warning: self-loop on " + to_string(a) + " detected."); // .... or maybe allow it?
        exit(0);
    } else if (weight == 0) {
        return;
    }

    g_adj_list[a].push_back(b);
    g_adj_list[b].push_back(a);
    
    edge_exists_lookup[keyAB] = true;
    edge_exists_lookup[keyBA] = true;

    edge_weight[keyAB] += weight;
    edge_weight[keyBA] += weight;

    ResetComputedTopology();
}

void MaxCutGraph::RemoveNode(int node) {
    ResetComputedTopology();

    const auto adj = GetAdjacency(node);
    for (unsigned int i = 0; i < adj.size(); ++i)
        RemoveEdgesBetween(adj[i], node);

    g_adj_list[node].clear();

    removed_node[node] = true;
    current_timestamp[node] = -1;
}

void MaxCutGraph::ReAddNode(int node) {
    custom_assert(removed_node[node]);
    ResetComputedTopology();

    removed_node.erase(removed_node.find(node));
}

int MaxCutGraph::CreateANode() {
    auto it = removed_node.begin();
    while (it != removed_node.end() && !it->second) {
        cout << "Warning. removed_node was accessed by = false." << endl;
        it++;
    }

    int sel_node = -1;
    if (it != removed_node.end()) {
        custom_assert(it->second);
        sel_node = it->first;
        ReAddNode(sel_node);
    } else {
        sel_node = num_nodes;
    }

    if (num_nodes <= sel_node)
        SetNumNodes(sel_node + 1); // expand num_nodes to accommodate.
    return sel_node;
}

void MaxCutGraph::RemoveEdgesBetween(int nodex, int nodey) {
    ResetComputedTopology();

    auto it = std::find(g_adj_list[nodex].cbegin(), g_adj_list[nodex].cend(), nodey);
    if (it != g_adj_list[nodex].end())
        g_adj_list[nodex].erase(it);
    
    it = std::find(g_adj_list[nodey].cbegin(), g_adj_list[nodey].cend(), nodex);
    if (it != g_adj_list[nodey].end())
        g_adj_list[nodey].erase(it);
    
    auto keyAB = MakeEdgeKey(nodex, nodey), keyBA = MakeEdgeKey(nodey, nodex);
    edge_exists_lookup.erase(keyAB);
    edge_exists_lookup.erase(keyBA);
    edge_weight.erase(keyAB);
    edge_weight.erase(keyBA);
}

void MaxCutGraph::RemoveEdgesInComponent(const vector<int> &component) {
    for (int i = 0; i < (int)component.size(); ++i) {
        for (int j = i + 1; j < (int)component.size(); ++j) {
            RemoveEdgesBetween(component[i], component[j]);
        }
    }
}

vector<int> MaxCutGraph::GetAllExistingNodes() const {
    vector<int> ret;
    for (int i = 0; i < num_nodes; ++i)
        if (MapEqualCheck(removed_node, i, false))
            ret.push_back(i);
    return ret;
}

vector<int> MaxCutGraph::GetConnectedComponentOf(int node, vector<bool>& visited) const {
    visited.resize(num_nodes, false);
    queue<int> q;
    visited[node] = true;
    q.push(node);

    vector<int> component;
    while (!q.empty()) {
        int u = q.front();
        q.pop();

        component.push_back(u);

        const auto& adj_list = g_adj_list.at(u);
        for (unsigned int i = 0; i < adj_list.size(); ++i) {
            int w = adj_list.at(i);

            if (visited[w] || MapEqualCheck(removed_node, w, true)) continue;
            visited[w] = true;
            q.push(w);
        }
    }

    return component;
}

vector<int> MaxCutGraph::GetConnectedComponentOf(int node) const {
    vector<bool> visited;
    return GetConnectedComponentOf(node, visited);
}

vector<vector<int>> MaxCutGraph::GetAllConnectedComponents() const {
    vector<bool> visited(num_nodes, false);
    vector<vector<int>> ret;

    for (int curr_node = 0; curr_node < num_nodes; ++curr_node) {
        if (visited[curr_node] || MapEqualCheck(removed_node, curr_node, true)) continue;

        auto component = GetConnectedComponentOf(curr_node, visited);
        ret.push_back(component);
    }

    return ret;
}

bool MaxCutGraph::DoesDisconnect(const vector<int>& selection_rem) const {
    auto before = GetAllConnectedComponents();
    auto vset_after_sub = SetSubstract(GetAllExistingNodes(), selection_rem);
    MaxCutGraph ng(*this, vset_after_sub); // this is a bit slow....
    auto after = ng.GetAllConnectedComponents();

    unordered_map<int,bool> mark_rem;
    for (auto node : selection_rem)
        mark_rem[node] = true;

    // If a whole component gets deleted, it doesn't make the graph disconnected, so account for these cases.
    int num_deleted_whole_components = 0;
    for (auto component : before) {
        bool wholly_removed = true;
        for (auto node : component)
            if (mark_rem[node] == false)
                wholly_removed = false;

        if (wholly_removed)
            num_deleted_whole_components++;
    }
    
    return before.size() != (after.size() + num_deleted_whole_components);
}

vector<pair<int,int>> MaxCutGraph::GetAllExistingEdges() const {
    vector<pair<int,int>> ret;
    for (int i = 0; i < num_nodes; ++i) {
        if (MapEqualCheck(removed_node, i, true)) continue;
        const auto& adj_list = g_adj_list[i];
        for (int w : adj_list) {
            if (MapEqualCheck(removed_node, w, true) || i >= w) continue;
            ret.push_back(make_pair(i, w));
        }
    }
    return ret;
}

bool MaxCutGraph::IsClique(const vector<int>& vertex_set) const {
    for (unsigned int i = 0; i < vertex_set.size(); ++i)
        for (unsigned int j = i + 1; j < vertex_set.size(); ++j) {
            auto edge_key = MakeEdgeKey(vertex_set[i], vertex_set[j]);
            if (!MapEqualCheck(edge_exists_lookup, edge_key, true))
                return false;
        }
    return true;
}

double MaxCutGraph::GetEdwardsErdosBound() const {
    auto ccomponents = GetAllConnectedComponents();
    
    double res = -(GetAllConnectedComponents().size() / 4.0);
    res += (GetRealNumEdges() / 2.0) + (GetRealNumNodes()) / 4.0;
    //for (auto component : ccomponents) {
    //    MaxCutGraph ng(*this, component);
   //     res += (ng.GetRealNumEdges() / 2.0) + (ng.GetRealNumNodes()) / 4.0;
   // }

    return res;
}

string MaxCutGraph::GetGraphNaming() const {
    return graph_naming;
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
        if (depth[curr_node] >= 0 || MapEqualCheck(removed_node, curr_node, true)) continue;

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

vector<int> MaxCutGraph::GetArticulationNodes() {
    if (!articulations_computed) ComputeArticulationAndBiconnected();

    vector<int> ret;
    for (int i = 0; i < num_nodes; ++i)
        if (is_articulation[i])
            ret.push_back(i);
            
    return ret;
}

bool MaxCutGraph::IsArticulation(int node) {
    if (!articulations_computed) ComputeArticulationAndBiconnected();

    return is_articulation[node];
}

vector<vector<int>> MaxCutGraph::GetBiconnectedComponents() {
    if (!bicomponents_computed) ComputeArticulationAndBiconnected();

    return biconnected_components;
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



bool MaxCutGraph::Breaks2Connected(vector<int> selection_rem) {
    if (!bicomponents_computed) ComputeArticulationAndBiconnected();
    custom_assert(biconnected_components.size() == 1);

    auto vset = GetAllExistingNodes();
    auto subvset = SetSubstract(vset, selection_rem);
    MaxCutGraph nwg(*this, subvset);
    nwg.ComputeArticulationAndBiconnected();

    return nwg.GetBiconnectedComponents().size() != 1;
}

// Return: {r,...,dest}
vector<int> MaxCutGraph::GetSingleSourcePathFromRoot(int dest) const {
    if (single_source_dist.at(dest) == -1)
        throw logic_error("Requested path to unreachable destination.");
    
    vector<int> ret;
    int curr = dest;
    while (curr != -1) {
        ret.push_back(curr);
        curr = single_source_prev.at(curr);
    }
    
    reverse(ret.begin(), ret.end());
    return ret;
}

tuple<vector<int>, int> MaxCutGraph::GetLeafBlockAndArticulation(bool print_components) {
    int selected_block_dx = -1;
    auto bicomponents = GetBiconnectedComponents();

#ifdef DEBUG
    if (print_components) {
        OutputDebugLog("Number of biconnected components in graph: " + to_string(bicomponents.size()) + ", all components:");
        for (unsigned int dx = 0; dx < bicomponents.size(); ++dx) {
            const auto& component = bicomponents[dx];
            OutputDebugVector("Component " + to_string(dx), component);

            MaxCutGraph cg(*this, component);
            OutputDebugVector("  edges", cg.GetAllExistingEdges());
        }
        OutputDebugLog("-- END-COMPONENTS --");
    }
#endif

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

void MaxCutGraph::ApplyOneWayRule3(const vector<int>& c_with_v, const int v) {
    int any_adj_to_v_node = -1;
    int any_nonadj_to_v_node = -1;
    for (const int node : c_with_v) {
        if (node == v) continue;

        if (edge_exists_lookup[MakeEdgeKey(node, v)])
            any_adj_to_v_node = node;
        if (!edge_exists_lookup[MakeEdgeKey(node, v)])
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

void MaxCutGraph::ApplyOneWayRule5(const vector<int>& c_with_v, const int v) {
    for (const int node : c_with_v) {
        if (node == v) continue;
        RemoveNode(node);
    }
}

void MaxCutGraph::ApplyOneWayRule6(const vector<int>& induced_2path) {
    custom_assert(induced_2path.size() == 3);

    for (auto node : induced_2path) {
        RemoveNode(node);
        paper_S.push_back(node);
    }
}

void MaxCutGraph::ApplyOneWayRule7(const vector<int>& c, const int v, const int b) {
    RemoveNode(v);
    RemoveNode(b);
    paper_S.push_back(v);
    paper_S.push_back(b);

    for (const int node : c)
        RemoveNode(node);
}

vector<int> MaxCutGraph::GetInducedPathByLemma2(const vector<int>& component, int r) {
        custom_assert(GetBiconnectedComponents().size() == 1);

        auto component_minus_r = SetSubstract(component, vector<int>{r});
        MaxCutGraph c_graph(*this, component);
        MaxCutGraph c_minus_r_graph(c_graph, component_minus_r);

        OutputDebugLog("Computing induced path for Rule 6 by using Lemma, since X - r not 2-connected.");

        // 1.
        auto bicomponents = c_minus_r_graph.GetBiconnectedComponents();
        auto anodes = c_minus_r_graph.GetArticulationNodes();
        custom_assert(anodes.size() > 0);
        int v = anodes[0];

        vector<int> Z[3];
        for (auto component : bicomponents) {
            if (find(component.begin(), component.end(), v) != component.end()) {
                if (Z[1].empty()) Z[1] = component;
                else if (Z[2].empty()) { Z[2] = component; break; }
            }
        }

        custom_assert(!Z[2].empty());
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

        custom_assert(u[1] != -1 && u[2] != -1);
        custom_assert(u_dist[1] != -1 && u_dist[2] != -1);

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

        custom_assert(w[1] != -1 && w[2] != -1);
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
            int x = component_minus_r[i];
            int d_xr = c_graph.GetSingleSourceDistance(x);
            if (current_min_d_xr <= d_xr) continue;

            for (unsigned int j = 0; j < component_minus_r.size(); ++j) { // we can't do j = i + 1, because symmetry not given in x and y because of min(r,x) condition.
                if (i == j) continue;

                int y = component_minus_r[j];

                if (edge_exists_lookup[MakeEdgeKey(x,y)]) continue;
                if (Li[1] == vector<int>{x,y}) continue;
                
                current_min_d_xr = d_xr;
                selected_x = x;
                selected_y = y;
                break;
            }
        }
        OutputDebugLog("Selection of (x,y) = (" + to_string(selected_x) + "," + to_string(selected_y) + ")");

        // Shortest path Q from r to x
        auto Q = c_graph.GetSingleSourcePathFromRoot(selected_x);
        // custom_assert(Li[1].size() > 0 && Li[3].size() > 0); This assertion was wrong -- misread in paper.
        custom_assert(Q.size() <= 3); // length of Q <= 2, meaning at most 3 nodes on path
        OutputDebugVector("Q", Q);

        vector<int> C_minus_Q_minus_x = SetSubstract(Q, {selected_x});
        C_minus_Q_minus_x = SetSubstract(component, C_minus_Q_minus_x);
        
        MaxCutGraph G_CmQmx(c_graph, C_minus_Q_minus_x);
        G_CmQmx.CalculateSingleSourceDistance(selected_x);
        auto P = G_CmQmx.GetSingleSourcePathFromRoot(selected_y);
        OutputDebugVector("P", P);
        
        vector<int> PP;
        if (P.size() >= 3) {
            custom_assert(P[0] == selected_x);

            for (unsigned int i = 0; i < 3; ++i)
                PP.push_back(P[i]);
            
            OutputDebugVector("PP", PP);
            if(!DoesDisconnect(PP)) return PP; // G!
        } else {
            // just comsmetic comment here
            // if P.size() < 3, then P doesn't exist.
            // P.size() >= 3 has to hold because of the way we selected x,y
            // {x,y} != L[1]
            custom_assert(false);
        }

        // We still didn't succeed => we do lemma 2 now.

        vector<int> QPP = Q; // Q.size <= 3
        for (unsigned int i = 1; i < PP.size(); ++i) // we start at 1 because of the overlap between end of Q and start of PP
            QPP.push_back(PP[i]); // PP.size <= 3, therefore QPP.size <= 6
        
        custom_assert(QPP.size() <= 6);
        custom_assert(QPP[0] == r);
        //custom_assert(DoesDisconnect(QPP));

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

        custom_assert(c_graph.Breaks2Connected(prefix_that_disconnects));

        OutputDebugVector("Prefix that disconnects", prefix_that_disconnects);

        auto vertex_that_disconnected = prefix_that_disconnects.back();
        prefix_that_disconnects.pop_back(); // doesn't make != 2 connected anymore
        auto new_component = SetSubstract(component, prefix_that_disconnects);
        prefix_that_disconnects.push_back(vertex_that_disconnected); // to keep it meaningful
        
        MaxCutGraph graph_for_lemma(*this, new_component);
        graph_for_lemma.ComputeArticulationAndBiconnected();
        custom_assert(graph_for_lemma.GetBiconnectedComponents().size() == 1);

        return graph_for_lemma.GetInducedPathByLemma2(new_component, vertex_that_disconnected);

    //   vector<int> xy = c_
    } else { // not 2-connected => use Lemma 4
        return c_graph.GetInducedPathByLemma2(component, r);
    }

    return vector<int>();
}

void MaxCutGraph::CalculateLemma4DFSTree(int root, int ui) {
    lemma4_dfs_tree_parent.assign(num_nodes, -1);
    lemma4_dfs_tree_depth.assign(num_nodes, -1);
    lemma4_dfs_tree_ui = ui;

    lemma4_dfs_tree_depth[root] = 0;
    CalculateLemma4DFSTree_(root);
}

bool MaxCutGraph::IsCliqueForest() {
    if (bicomponents_computed) ComputeArticulationAndBiconnected();

    auto components = GetBiconnectedComponents();
    for (const auto& component : components)
        if (!IsClique(component))
            return false;

    return true;
}

vector<int> MaxCutGraph::GetMarkedVerticesByOneWayRules() const {
    return paper_S;
}


void MaxCutGraph::CalculateLemma4DFSTree_(int node) {
    // ui child of v if (v,ui) in E; here v = root, dfs_tree_ui = ui.
    if (lemma4_dfs_tree_depth[node] == 0) {
        for (auto child : g_adj_list[node]) {
            if (child == lemma4_dfs_tree_ui) {
                lemma4_dfs_tree_parent[child] = node;
                lemma4_dfs_tree_depth[child] = lemma4_dfs_tree_depth[node] + 1;
                CalculateLemma4DFSTree_(child);
            }
        }
    }

    for (auto child : g_adj_list[node]) {
        if (lemma4_dfs_tree_depth[child] > -1) continue; // visited

        lemma4_dfs_tree_parent[child] = node;
        lemma4_dfs_tree_depth[child] = lemma4_dfs_tree_depth[node] + 1;
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
    }
    return ret;
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
        if (MapEqualCheck(removed_node, u, true)) continue;

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
        custom_assert(component.size() == 1);
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
        custom_assert(dx != -1);

        int idx = -1;
        vector<pair<int,int>> eps;
        if (dx == 0) idx = get<1>(entry), eps = get<4>(entry);
        else if (dx == 1 ) idx = get<2>(entry), eps = get<5>(entry);

        for (int i = 0; i <= idx; ++i) computed_maxcut_coloring_tmp[eps[i].second] = dx;
        for (int i = idx + 1; i < (int)eps.size(); ++i) computed_maxcut_coloring_tmp[eps[i].second] = !dx;
    }

    return make_tuple(sol + p, computed_maxcut_coloring_tmp);
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
    }
    return mx_sol;
}

vector<int> MaxCutGraph::GetAClique(const int min_size, const int runs, const bool make_maximum) const {
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

// Right now: not O(|V| + |E|) because of sorting. Only because of that. To achieve full linear time, use counting sort.
vector<vector<int>> MaxCutGraph::GetAllR8Candidates(const bool break_on_first, const unordered_map<int,bool>& preset_is_external) const {
    vector<vector<int>> ret;
    vector<int> current_v = GetAllExistingNodes();
    vector<bool> visited(num_nodes, false);

    trie_r8 partitions;
    for (auto root : current_v) {
        auto key = SetUnion(GetAdjacency(root), vector<int>{root});
        sort(key.begin(), key.end());
        partitions.Insert(key, root);
    }

    for (auto root : current_v) {
        if (visited[root]) continue;

        auto key = SetUnion(GetAdjacency(root), vector<int>{root}); // O(Ng(root))
        sort(key.begin(), key.end());
        
        const auto X = partitions.Retrieve(key); // O(Ng(root))
        const auto NG = SetSubstract(key, X); // O(Ng(root))

        bool ok = true;
        for (auto x : X) { // O(Ng(root)), guarantees only that vertices in X are not visited again, not Ng(root)!! Still, an edge is visited at most twice.
            visited[x] = true;
            if (KeyExists(x, preset_is_external))
                ok = false;
        }

        if (ok && X.size() > NG.size() && IsClique(X) && X.size() > 1) {
            ret.push_back(X);

            if (break_on_first)
                return ret;
        }
    }

    return ret;
}

void MaxCutGraph::ApplyR8Candidate(const vector<int>& clique) {
    custom_assert(clique.size() >= 2);

    int frem = rand() % clique.size();
    int srem = (frem + 1) % clique.size();
    int rem_node1 = clique[frem], rem_node2 = clique[srem];

    inflicted_cut_change_to_kernelized -= GetAdjacency(rem_node1).size();
    RemoveNode(rem_node1);
    RemoveNode(rem_node2);

    rules_usage_count[RuleIds::Rule8]++;
}

vector<pair<int,vector<pair<int,int>>>> MaxCutGraph::GetAllR9Candidates(const bool break_on_first) const {
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
                if (na >= nb || MapEqualCheck(edge_exists_lookup, MakeEdgeKey(na,nb), false))
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
                        if (MapEqualCheck(edge_exists_lookup, MakeEdgeKey(x1, x2), true))
                            ok = false;

                if (!ok) continue;

                ret.push_back(make_pair(root, vector<pair<int,int>>{b1,b2}));

                if (break_on_first)
                    return ret;
            }
        }
    }

    return ret;
}

void MaxCutGraph::ApplyR9Candidate(const pair<int,vector<pair<int,int>>> &candidate) {
    const pair<int,int> &triag1 = candidate.second[0];
    const pair<int,int> &triag2 = candidate.second[1];
    AddEdge(triag1.first, triag2.first);
    AddEdge(triag1.first, triag2.second);

    AddEdge(triag1.second, triag2.first);
    AddEdge(triag1.second, triag2.second);

    inflicted_cut_change_to_kernelized += 2; // increases (4 edges / 2 in EE)
    rules_usage_count[RuleIds::Rule9]++;
}

vector<pair<vector<int>, vector<int>>> MaxCutGraph::GetAllR9XCandidates(const bool break_on_first) const {
    vector<pair<vector<int>, vector<int>>> ret;

    vector<bool> visited(num_nodes, false);
    vector<int> current_v = GetAllExistingNodes();
    for (auto root : current_v) {
        if (visited[root]) continue;

        vector<int> clique = GetAdjacency(root);
        clique.push_back(root);

        for (auto x : clique)
            visited[x] = true;

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

            if (break_on_first)
                return ret;
        }
    }

    return ret;
}
void MaxCutGraph::ApplyR9XCandidate(const pair<vector<int>, vector<int>>& candidate) {
    custom_assert(candidate.second.size() >= 1);

    int rem_node = candidate.second[rand() % candidate.second.size()];
    inflicted_cut_change_to_kernelized -= (2*GetAdjacency(rem_node).size() + 2 /* +1 for cut change, +1 for removal of node in EE */) / 4.0; // why is this an integer? GetAdjacency(rem_node).size() is odd, since clique is even and doesnt contain itself.

    RemoveNode(rem_node);
    rules_usage_count[RuleIds::Rule9X]++;
}

vector<tuple<bool, int, int, int>> MaxCutGraph::GetAllR10Candidates(const bool break_on_first, const unordered_map<int,bool>& preset_is_external) const {
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

                if (break_on_first)
                    return ret;
            }
        }

        if (!bridge_case) {
            ret.push_back(make_tuple(false, u, nodex, nodey));

            if (break_on_first)
                return ret;
        }
    }
    
    return ret;
}
void MaxCutGraph::ApplyR10Candidate(const tuple<bool, int, int, int>& candidate) {
    bool bridge_case = get<0>(candidate);
    int u = get<1>(candidate);
    int nodex = get<2>(candidate);
    int nodey = get<3>(candidate);

    if (bridge_case) {
        vector<int> adj_nodex = GetAdjacency(nodex);
        vector<int> adj_nodey = GetAdjacency(nodey);
        vector<int> adj = SetUnion(adj_nodex, adj_nodey);

        inflicted_cut_change_to_kernelized -= (2 * GetAdjacency(u).size() + 1) / 4.0;
        RemoveNode(u);

        inflicted_cut_change_to_kernelized -= (2 * GetAdjacency(nodex).size() + 1) / 4.0;
        RemoveNode(nodex);

        inflicted_cut_change_to_kernelized -= (2 * GetAdjacency(nodey).size() + 1) / 4.0;
        RemoveNode(nodey);

        inflicted_cut_change_to_kernelized += 1 / 4.0;
        ReAddNode(u);

        for (auto v : adj) {
            if (v != u && v != nodex && v != nodey) {
                inflicted_cut_change_to_kernelized += 1 / 2.0;
                AddEdge(u, v);
            }
        }
    } else {
        inflicted_cut_change_to_kernelized -= (2 * GetAdjacency(u).size() + 2 /* +1 for node (/4 before already) and +1 for the change in cut (/4 before already) */) / 4.0; // k-- in EE included here.
        RemoveNode(u);

        inflicted_cut_change_to_kernelized -= 1 / 2.0; // m/2 in EE
        RemoveEdgesBetween(nodex, nodey);
    }

    rules_usage_count[RuleIds::Rule10]++; // beware of possibly having return in one of if above!
}

// O(|V| + |E|)
vector<tuple<int,int,int,int,int>> MaxCutGraph::GetAllR10ASTCandidates(const bool break_on_first, const unordered_map<int,bool>& preset_is_external) const {
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

        if (break_on_first)
            return ret;
    }

    return ret;
}
void MaxCutGraph::ApplyR10ASTCandidate(const tuple<int,int,int,int,int>& candidate) {
    int ex_L = get<0>(candidate), a = get<1>(candidate), b = get<2>(candidate),
           c = get<3>(candidate), ex_R = get<4>(candidate);

    RemoveNode(a);
    RemoveNode(c);
    AddEdge(ex_L, b);
    AddEdge(b, ex_R);
    inflicted_cut_change_to_kernelized -= 2;
    rules_usage_count[RuleIds::Rule10AST]++;
}

// Interesting facts on this rule:
// -- in case of RGG graphs: manages to process ALL cliques with at least one internal vertex -- therefore, cliques tend to be small.
// 
vector<int> MaxCutGraph::GetS2Candidates(const bool consider_dirty_only, const bool break_on_first, const unordered_map<int,bool>& preset_is_external) {
    auto cmp = [&](int a, int b) {
        return Degree(a) < Degree(b);
    };

    //vector<bool> visited(num_nodes, false);
    unordered_map<int, int> visited;

    vector<int> current_v;
    if (consider_dirty_only) {
        current_v = GetVerticesAfterTimestamp(CURRENT_TIMESTAMPS.S2, false); //GetAllExistingNodes();
        if (!break_on_first)
            CURRENT_TIMESTAMPS.S2 = current_kernelization_time;
    } else {
        current_v = GetAllExistingNodes();
    }

    sort(current_v.begin(), current_v.end(), cmp); // remove later

    //std::mutex critical;
   // vector<thread> threads;

    int cnt = 0;
    vector<int> ret;
    for (auto root : current_v) { // WE SEARCH FOR INTERNAL VERTICES!
        if (visited[root]) continue;
        if (KeyExists(root, preset_is_external)) continue;
        
        const auto adj_root = GetAdjacency(root);
        vector<int> curr_clique = SetUnion(adj_root, {root});

        // With the following section we want to make sure that two cliques in the result do not intersect.   (1)
        // Since the application of one might invalidate the applicability of the other.
        // This is checked by visited[node] == 2 (which identifies vertices in a clique)
        bool intersect = false; 
        for (auto node : curr_clique)
            if (visited[node] == 2) intersect = true;
        
        if (!IsClique(curr_clique)) { // NOT an internal vertex.
            // this may only possibly change if an edge in curr_clique is added or a vertex is removed.
            // the edge case justifies calling GetVerticesAfterTimestamp with neighbhors retrieval.
            continue;
        }

        // Following marking only makes sense because we have a clique.
        // Reason: External nodes are external in any clique and internal vertices of curr_clique are only internal in curr_clique
        // (in other words: they are not relevant to any other cliques)
        // All of this does NOT hold when root \cup N_{G}(root) is not a clique.
        // It also does NOT hold if curr_clique is being changed in any way.
        for (auto node : curr_clique) {
            if (visited[node] == 0) // We take care not to remove information on those marked visited[node] == 2!
                visited[node] = 1;
        }

        vector<int> externals;
        for (auto node : curr_clique) {
            const auto adj = GetAdjacency(node);
            if (adj.size() + 1 != curr_clique.size() || KeyExists(node, preset_is_external))
                externals.push_back(node);
        }

        // If this fails, we may assume that we require more vertices as internal. Therefore! We can expect a change on one of those vertices
        // in the future and may also assume that this change will pop up through a feasibly update on the timetable.
        if (!(externals.size() <= ((curr_clique.size() >> 1) + (curr_clique.size() % 2)))) {
            continue;
        }

        // As outlined in (1), we skip this case as it is not advisable to mix multiple candidates in the result that can invalidate each other.
        // We update the skipped vertex's timestamp as we want to consider it in the future again.
        if (intersect) {
            // Unless edges of all vertices are affected in curr_clique, no vertex is able to become a candidate.
            // Therefore! Application of S2 will destroy the inducibility of cliques for all vertices in curr_clique.
            // This is because at least one vertex is connected to all other vertices, but that one does not induce a clique.
            // THEREFORE, we do not do this:
            // for (auto node : curr_clique)
            //    UpdateVertexTimestamp(node);
            UpdateVertexTimestamp(root); // Case where this is needed: two candidates share external vertex.
            continue;
        }

        
        //std::lock_guard<std::mutex> lock(critical);
        ret.push_back(root);
        for (auto node : curr_clique)
            visited[node] = 2;

        if (break_on_first) return ret;
    }

   // cout << "CNT: " << cnt << endl;
   // cout << "Candidates: ";
   // sort(ret.begin(), ret.end());
    //for (auto c : ret) cout << c << " ";
   // cout << endl;

    //const size_t nthreads = 1;//std::thread::hardware_concurrency();
    //int blocksz = (current_v.size() / nthreads) + (current_v.size() % nthreads != 0);
    //for (size_t i = 0; i < nthreads; ++i) {
    //    threads.push_back(thread(handle_vertex, i * blocksz, min(current_v.size(), (i+1) * blocksz) - 1));
    //}
    //for (auto root : current_v) { // an internal vertex
    //    threads.push_back(thread(handle_vertex, root));
        // there cant be more, add proof in thesis.
   // }

    //std::for_each(threads.begin(), threads.end(), [](std::thread& x){x.join();});

    return ret;
}
bool MaxCutGraph::ApplyS2Candidate(const int root, const unordered_map<int,bool>& preset_is_external) {// Clique cut.
    vector<int> clique = GetAdjacency(root);
    clique = SetUnion(clique, {root});

    int n = clique.size();
    int add_tot = 0;
    for (int i = 1; i <= n; ++i) {
        int add = (n - i) - (i - 1);
        if (add <= 0) break;
        add_tot += add;
    }
    inflicted_cut_change_to_kernelized -= add_tot;
    
    // Apply changes to graph:
    vector<int> rem_nodes;
    for (auto node : clique) {   
        const auto adj = GetAdjacency(node);
        if (adj.size() + 1 == clique.size() && !KeyExists(node, preset_is_external)) { // is internal
            rem_nodes.push_back(node);
        } else {
            UpdateVertexTimestamp(node);
        }
    }

    // cout << "Remove: " << SerializeVector(clique) << endl;

#ifdef DEBUG
    int ext = clique.size() - rem_nodes.size();
    if (!(ext <= (int)((clique.size() >> 1) + (clique.size() % 2)))) {
        throw std::logic_error("SERIOUS ERROR IN ASSUMPTION.");
        exit(0);
    }

    if (!IsClique(clique)) {
        throw std::logic_error("EXPECTED CLIQUE. DID NOT GET ONE. " + to_string(root) + " -> " + SerializeVector(clique));
        exit(0);
    }
#endif

    for (auto node : clique) { // rem_nodes works?
        for (auto node2 : clique) {
            if (edge_exists_lookup[MakeEdgeKey(node, node2)]) {
                RemoveEdgesBetween(node, node2);
            }
        }
    }

    for (auto node : rem_nodes)
        RemoveNode(node);
    
    return true;
}

vector<vector<int>> MaxCutGraph::GetS3Candidates(const bool break_on_first, const unordered_map<int,bool>& preset_is_external) const {
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

        if (clique.size() % 2 == 0 && (int)externals.size() == ((int)clique.size()) - 2)
            continue;

        bool ok = true;
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
        

        if (ok) {
            ret.push_back(clique);
            if (break_on_first)
                break;
        }
    }

    return ret;
}

void MaxCutGraph::ApplyS3Candidate(const vector<int>& clique, const unordered_map<int,bool>& preset_is_external) {
    (void) preset_is_external;

    for (int i = 0; i < (int)clique.size(); ++i)
        for (int j = i + 1; j < (int)clique.size(); ++j)
            if (AreAdjacent(clique[i], clique[j]) == false)
                AddEdge(clique[i], clique[j]);
    rules_usage_count[RuleIds::RuleS3]++;
}

vector<tuple<bool,int,int,int,int>> MaxCutGraph::GetAllS4Candidates(const bool break_on_first, const unordered_map<int,bool>& preset_is_external) const {
    vector<tuple<bool,int,int,int,int>> ret;
    auto current_v = GetAllExistingNodes();
    for (int i = 0; i < (int)current_v.size(); ++i) {
        int nodeA = current_v[i];
        const auto &adjA = GetAdjacency(nodeA);
        for (auto nodeB : adjA) {
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

            if (ret.empty() == false && break_on_first)
                return ret;
        }
    }
    return ret;
}
void MaxCutGraph::ApplyS4Candidate(tuple<bool,int,int,int,int> &candidate) {
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
        inflicted_cut_change_to_kernelized -= 4;
    } else {
        RemoveNode(nodeC); // or nodeD
        inflicted_cut_change_to_kernelized -= 2;
    }

    rules_usage_count[RuleIds::RuleS4]++;
}

// O(|V| + |E|)
vector<tuple<int,int,int,int>> MaxCutGraph::GetAllS5Candidates(const bool break_on_first, const unordered_map<int,bool>& preset_is_external) const {
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

        if (break_on_first)
            return ret;
    }
    return ret;
}
void MaxCutGraph::ApplyS5Candidate(const tuple<int,int,int,int>& candidate) {
    int ex_L = get<0>(candidate), a = get<1>(candidate), b = get<2>(candidate),
        ex_R = get<3>(candidate);

    RemoveNode(a);
    RemoveNode(b);
    AddEdge(ex_L, ex_R);
    inflicted_cut_change_to_kernelized -= 2;
    rules_usage_count[RuleIds::RuleS5]++;
}


vector<pair<int,int>> MaxCutGraph::GetAllS6Candidates(const bool break_on_first, const unordered_map<int,bool>& preset_is_external) const {
    vector<pair<int,int>> ret;

    const auto &current_v = GetAllExistingNodes();
    for (auto root_A : current_v) { // an internal vertex
        auto adj_root_A = GetAdjacency(root_A);
        if (KeyExists(root_A, preset_is_external) || adj_root_A.size() == 0) continue;
        sort(adj_root_A.begin(), adj_root_A.end());

        for (auto root_B : adj_root_A) { // root_A and root_B are connected.
            if (KeyExists(root_B, preset_is_external)) continue;
            auto adj_root_B = GetAdjacency(root_B);
            sort(adj_root_B.begin(), adj_root_B.end());
            auto NG = SetSubstract(adj_root_A, {root_B});
            
            if (NG == SetSubstract(adj_root_B, {root_A}) && IsClique(NG)) {
                ret.push_back(make_pair(root_A, root_B));

                if (break_on_first)
                    return ret;
            }
        }
    }

    return ret;
}

void MaxCutGraph::ApplyS6Candidate(const pair<int,int> &candidate, const unordered_map<int,bool>& preset_is_external) {
    (void) preset_is_external;

    RemoveEdgesBetween(candidate.first, candidate.second);
    rules_usage_count[RuleIds::RuleS6]++;
}

bool MaxCutGraph::CandidateSatisfiesSpecialRule1(const tuple<int,int,int,int> &candidate) const {
    int a = get<0>(candidate), b = get<1>(candidate), c = get<2>(candidate), d = get<3>(candidate);

    if (MapEqualCheck(removed_node, a, true)) return false;
    if (MapEqualCheck(removed_node, b, true)) return false;
    if (MapEqualCheck(removed_node, c, true)) return false;
    if (MapEqualCheck(removed_node, d, true)) return false;

    auto adj1 = GetAdjacency(b);
    auto adj2 = GetAdjacency(c);

    if (adj1.size() != 2 || adj2.size() != 2) return false;

   // if (MapEqualCheck(edge_weight, MakeEdgeKey(a,b), 1) == false) return false;
   // if (MapEqualCheck(edge_weight, MakeEdgeKey(b,c), 1) == false) return false;
  //  if (MapEqualCheck(edge_weight, MakeEdgeKey(c,d), 1) == false) return false;

    return min(adj1[0], adj1[1]) == min(a, c) && max(adj1[0], adj1[1]) == max(a, c)
        && min(adj2[0], adj2[1]) == min(b, d) && max(adj2[0], adj2[1]) == max(b, d);
}

bool MaxCutGraph::CandidateSatisfiesSpecialRule2(const tuple<int,int,int> &candidate) const {
    if (MapEqualCheck(removed_node, get<0>(candidate), true)) return false;
    if (MapEqualCheck(removed_node, get<1>(candidate), true)) return false;
    if (MapEqualCheck(removed_node, get<2>(candidate), true)) return false;
    
    int b = get<1>(candidate);
    auto adj = GetAdjacency(b);
    if (adj.size() != 2) return false;

    int a = get<0>(candidate), c = get<2>(candidate);

   //// if (MapEqualCheck(edge_weight, MakeEdgeKey(a,b), 1) == false) return false;
   // if (MapEqualCheck(edge_weight, MakeEdgeKey(b,c), 1) == false) return false;

    return min(adj[0], adj[1]) == min(a,c) && max(adj[0], adj[1]) == max(a,c);
}

vector<tuple<int,int,int,int>> MaxCutGraph::GetAllSpecialRule1Candidates() const {
    vector<tuple<int,int,int, int>> ret;
    
    const auto &current_v = GetAllExistingNodes();
    for (auto b : current_v) {
        auto &adj1 = GetAdjacency(b);

        if (adj1.size() != 2) continue;

        for (int i = 0; i < (int)adj1.size(); ++i) {
            int c = adj1[i];
            auto &adj2 = GetAdjacency(c);
            if (adj2.size() != 2) continue;

            int a = adj1[1 - i];
            int d = adj2[0] == b ? adj2[1] : adj2[0];

            if (a == d) continue;

            auto candidate = make_tuple(a, b, c, d);
            custom_assert(CandidateSatisfiesSpecialRule1(candidate));
            ret.push_back(candidate);
        }
    }

    return ret;
}

vector<tuple<int,int,int>> MaxCutGraph::GetAllSpecialRule2Candidates() const {
    vector<tuple<int,int,int>> ret;
    
    const auto &current_v = GetAllExistingNodes();
    for (auto root : current_v) {
        auto &adj = GetAdjacency(root);

        if (adj.size() != 2 || adj[0] == adj[1]) continue; // later case possible to handle differently?
        auto candidate = make_tuple(adj[0], root, adj[1]);
        custom_assert(CandidateSatisfiesSpecialRule2(candidate));
        ret.push_back(candidate);
    }

    return ret;
}

bool MaxCutGraph::ApplySpecialRule1(const tuple<int,int,int,int> &candidate) {
    if (!CandidateSatisfiesSpecialRule1(candidate))
        return false;

    int a = get<0>(candidate), b = get<1>(candidate), c = get<2>(candidate), d = get<3>(candidate);
    int w1 = edge_weight.at(MakeEdgeKey(a,b)), w2 = edge_weight.at(MakeEdgeKey(b,c)), w3 = edge_weight.at(MakeEdgeKey(c,d));
    RemoveNode(b);
    RemoveNode(c);
    AddEdge(a, d, min(w1, min(w2, w3)));
    inflicted_cut_change_to_kernelized -= w1 + w2 + w3 - min(w1, min(w2, w3));

    rules_usage_count[RuleIds::SpecialRule1]++;
    return true;
}

bool MaxCutGraph::ApplySpecialRule2(const tuple<int,int,int> &candidate) {
    if (!CandidateSatisfiesSpecialRule2(candidate))
        return false;

    int a = get<0>(candidate), b = get<1>(candidate), c = get<2>(candidate);
    int w1 = edge_weight.at(MakeEdgeKey(a,b)), w2 = edge_weight.at(MakeEdgeKey(b,c));
    RemoveNode(b);
    AddEdge(a, c, -min(w1, w2));
    inflicted_cut_change_to_kernelized -= w1 + w2;

    rules_usage_count[RuleIds::SpecialRule2]++;
    return true;
}


vector<pair<int,int>> MaxCutGraph::GetAllRevSpecialRule1Candidates() const {
    vector<pair<int,int>> ret;

    auto elist = GetAllExistingEdges();
    for (auto e : elist) {
        int w = edge_weight.at(MakeEdgeKey(e));
        if (w > 1) ret.push_back(e);
    }

    return ret;
}

vector<pair<int,int>> MaxCutGraph::GetAllRevSpecialRule2Candidates() const {
    vector<pair<int,int>> ret;

    auto elist = GetAllExistingEdges();
    for (auto e : elist) {
        int w = edge_weight.at(MakeEdgeKey(e));
        if (w < 0) ret.push_back(e);
    }

    return ret;
}

bool MaxCutGraph::ApplyRevSpecialRule1(const pair<int,int> &candidate) {
    int a = candidate.first, b = candidate.second;
    int w = edge_weight.at(MakeEdgeKey(candidate));

    custom_assert(w > 1);

    RemoveEdgesBetween(a, b);
    AddEdge(a, b, 1);
    for (int i = 0; i < w - 1; ++i) {
        int middle1 = CreateANode();
        int middle2 = CreateANode();
        AddEdge(a, middle1, 1);
        AddEdge(middle1, middle2, 1);
        AddEdge(middle2, b, 1);
        inflicted_cut_change_to_kernelized += 2;
    }

    rules_usage_count[RuleIds::RevSpecialRule1]++;
    return true;
}

bool MaxCutGraph::ApplyRevSpecialRule2(const pair<int,int> &candidate) {
    int a = candidate.first, b = candidate.second;
    int w = edge_weight.at(MakeEdgeKey(candidate));

    custom_assert(w < 0);

    RemoveEdgesBetween(a, b);
    for (int i = 0; i < -w; ++i) {
        int middle = CreateANode();
        AddEdge(a, middle, 1);
        AddEdge(b, middle, 1);
        inflicted_cut_change_to_kernelized += 2;
    }

    rules_usage_count[RuleIds::RevSpecialRule2]++;
    return true;
}

bool MaxCutGraph::PerformKernelization(const RuleIds rule_id) {
    rules_check_count[rule_id]++;

    switch(rule_id) {
        case RuleIds::RuleS2: {
            auto candidates = GetS2Candidates();
            for (auto candidate : candidates)
                rules_usage_count[rule_id] += ApplyS2Candidate(candidate);

            return !candidates.empty();
        }
        case RuleIds::Rule8: { 
            auto candidates = GetAllR8Candidates();
            for (auto candidate : candidates)
                ApplyR8Candidate(candidate);

            return !candidates.empty();
        }
        case RuleIds::Rule9: { 
            auto candidates = GetAllR9Candidates(true);
            if (!candidates.empty()) {
                ApplyR9Candidate(candidates[0]);
                return true;
            }
            return false;
        }
        case RuleIds::Rule9X: { 
            auto candidates = GetAllR9XCandidates();
            for (auto candidate : candidates)
                ApplyR9XCandidate(candidate);

            return !candidates.empty();
        }
        case RuleIds::Rule10: {
            auto candidates = GetAllR10Candidates(true);
            if (!candidates.empty()) {
                ApplyR10Candidate(candidates[0]);
                return true;
            }
            return false;
        }
        case RuleIds::Rule10AST: {
            auto candidates = GetAllR10ASTCandidates(true);
            if (!candidates.empty()) {
                ApplyR10ASTCandidate(candidates[0]);
                return true;
            }
            return false;
        }
        case RuleIds::RuleS3: {
            auto candidates = GetS3Candidates(true);
            if (!candidates.empty()) {
                ApplyS3Candidate(candidates[0]);
                return true;
            }
            return false;
        }
        case RuleIds::RuleS4: {
            auto candidates = GetAllS4Candidates(true);
            if (!candidates.empty()) {
                ApplyS4Candidate(candidates[0]);
                return true;
            }
            return false;
        }
        case RuleIds::RuleS5: {
            auto candidates = GetAllS5Candidates(true);
            if (!candidates.empty()) {
                ApplyS5Candidate(candidates[0]);
                return true;
            }
            return false;
        }
        case RuleIds::RuleS6: {
            // maybe i realized this doesnt make sense? verify and delete if so with implication in commit comment.
        }
        
        

        case RuleIds::SpecialRule1:
        case RuleIds::SpecialRule2:
        case RuleIds::RevSpecialRule1:
        case RuleIds::RevSpecialRule2:
            break;
    }

    return false;
}

void MaxCutGraph::MakeUnweighted() {
    while (true) {
        const auto &r1_candidates = GetAllRevSpecialRule1Candidates();
        for (auto candidate : r1_candidates)
            ApplyRevSpecialRule1(candidate);

        const auto &r2_candidates = GetAllRevSpecialRule2Candidates();
        for (auto candidate : r2_candidates)
            ApplyRevSpecialRule2(candidate);

        if (r1_candidates.empty() && r2_candidates.empty())
            break;
    }
}

void MaxCutGraph::MakeWeighted() {

    while (true) {
        const auto &r1_candidates = GetAllSpecialRule1Candidates();
        for (auto candidate : r1_candidates)
            ApplySpecialRule1(candidate);

        const auto &r2_candidates = GetAllSpecialRule2Candidates();
        for (auto candidate : r2_candidates)
            ApplySpecialRule2(candidate);

        if (r1_candidates.empty() && r2_candidates.empty())
            break;
    }
}

double MaxCutGraph::ExecuteLinearKernelization() {

    while (true) {
        // <- possibly relabel graph here first before using R8.
        auto res_r8 = GetAllR8Candidates(); // not yet fully linear! See implementation.
        if (!res_r8.empty()) {
            ApplyR8Candidate(res_r8[0]);
            continue;
        }
        
        auto res_r9 = GetAllR9Candidates();
        if (!res_r9.empty()) {
            ApplyR9Candidate(res_r9[0]);
            continue;
        }

        break;
    }

    return GetInflictedCutChangeToKernelized();
}

void MaxCutGraph::ExecuteExhaustiveKernelization() {
    while (true) {
        auto res_rs2 = GetS2Candidates(true);
        if (!res_rs2.empty()) {
            ApplyS2Candidate(res_rs2[0]);
            continue;
        }

        auto res_r9x = GetAllR9XCandidates();
        if (!res_r9x.empty()) {
            ApplyR9XCandidate(res_r9x[0]);
            continue;
        }
        
        auto res_r8 = GetAllR8Candidates();
        if (!res_r8.empty()) {
            ApplyR8Candidate(res_r8[0]);
            continue;
        }
        
        auto res_r10 = GetAllR10Candidates();
        if (!res_r10.empty()) {
            ApplyR10Candidate(res_r10[0]);
            continue;
        }
        
        auto res_r9 = GetAllR9Candidates();
        if (!res_r9.empty()) {
            ApplyR9Candidate(res_r9[0]);
            continue;
        }

        auto res_r10ast = GetAllR10ASTCandidates();
        if (!res_r10ast.empty()) {
            ApplyR10ASTCandidate(res_r10ast[0]);
            continue;
        }
        
        break;
    }
}

void MaxCutGraph::ExecuteExhaustiveKernelizationExternalsSupport(const unordered_map<int,bool> &preset_is_external) {
    while (true) {
        /*
        auto res_rs2 = GetS2Candidates(true, preset_is_external);
        if (!res_rs2.empty()) {
            ApplyS2Candidate(res_rs2[0], k_change, preset_is_external);
            continue;
        }*/

        
        auto res_s3 = GetS3Candidates(true, preset_is_external);
        if (!res_s3.empty()) {
            ApplyS3Candidate(res_s3[0], preset_is_external);
            continue;
        }

        auto res_r10ast = GetAllR10ASTCandidates(true, preset_is_external);
        if (!res_r10ast.empty()) {
            ApplyR10ASTCandidate(res_r10ast[0]);
            continue;
        }

        auto res_r8 = GetAllR8Candidates(true, preset_is_external);
        if (!res_r8.empty()) {
            ApplyR8Candidate(res_r8[0]);
            continue;
        }

        
        auto res_s4 = GetAllS4Candidates(true, preset_is_external);
        if (!res_s4.empty()) {
            ApplyS4Candidate(res_s4[0]);
            continue;
        }

      
        
        auto res_s5 = GetAllS5Candidates(true, preset_is_external);
        if (!res_s5.empty()) {
            ApplyS5Candidate(res_s5[0]);
            continue;
        }
        
        
        auto res_s6 = GetAllS6Candidates(true, preset_is_external);
        if (!res_s6.empty()) {
            ApplyS6Candidate(res_s6[0], preset_is_external);
            continue;
        }

        break;
    }
}

void MaxCutGraph::PrintGraph(std::ostream& out) const {
    out << num_nodes << " " << GetRealNumEdges() << endl;

    for (int i = 0; i < num_nodes; ++i) {
        const auto &adj = GetAdjacency(i);
        for (auto node : adj) {
            if (i < node)
                out << i + 1<< " " << node + 1<< " " << 1 << endl;
        }
    }
}

void MaxCutGraph::PrintReductionsUsage() const {
    for (auto key : kAllRuleIds) {
        cout << "//  Rule with id " << static_cast<int>(key) << endl;
        cout << "//  Description: " << kRuleDescriptions.at(key) << endl;
        cout << "//  Number of times used: " << (rules_usage_count.find(key) != rules_usage_count.end() ? rules_usage_count.at(key) : 0) << endl;
        cout << endl;
    }
}

int MaxCutGraph::GetRuleUsage(RuleIds rule) const {
    if (rules_usage_count.find(rule) != rules_usage_count.end())
        return rules_usage_count.at(rule);
    else
        return 0;
}

int MaxCutGraph::GetRuleChecks(RuleIds rule) const {
    if (rules_check_count.find(rule) != rules_check_count.end())
        return rules_check_count.at(rule);
    else
        return 0;
}

vector<int> MaxCutGraph::GetUsageVector() const {
    vector<int> ret;
    for (auto key : kAllRuleIds) {
        if (rules_usage_count.find(key) != rules_usage_count.end())
            ret.push_back(rules_usage_count.at(key));
        else
            ret.push_back(0);
    }

    return ret;
}

string MaxCutGraph::PrintDegrees(const unordered_map<int,bool>& preset_is_external) const {
    //auto current_v = GetAllExistingNodes();
    vector<int> current_v;
    for (int i = 0; i < num_nodes; ++i) current_v.push_back(i);

    string ret = "[[deg(";
    int tottot = 0;
    for (auto node : current_v) {
        tottot += Degree(node);
        ret += to_string(Degree(node)) + ",";
    }

    ret += "=" + to_string(tottot) + ")  ->(";
    tottot = 0;
    for (auto node : current_v) {
        if (KeyExists(node, preset_is_external) == false) continue;
        const auto &adj = GetAdjacency(node);
        int to_R = 0;
        for (auto w : adj)
            to_R += KeyExists(w, preset_is_external) == false;
        tottot += to_R;
        ret += to_string(to_R) + ",";
    }

    ret += "=" + to_string(tottot) + ")  <-(";
    tottot = 0;
    for (auto node : current_v) {
        if (KeyExists(node, preset_is_external)) continue;
        const auto &adj = GetAdjacency(node);
        int to_L = 0;
        for (auto w : adj)
            to_L += KeyExists(w, preset_is_external) == true;
        tottot += to_L;
        ret += to_string(to_L) + ",";
    }

    ret += "=" + to_string(tottot) + ")  *(";
    tottot = 0;
    for (auto node : current_v) {
        const auto &adj = GetAdjacency(node);
        int tot = 0;
        for (auto w : adj)
            tot += KeyExists(w, preset_is_external) == KeyExists(node, preset_is_external);
        tottot += tot;
        ret += to_string(tot) + ",";
    }
    
    ret += "=" + to_string(tottot) + ")]]";
    return ret;
}

int MaxCutGraph::GetCutSize(const vector<int> &grouping) const {
    int ret = 0;
    for (int i = 0; i < num_nodes; ++i) {
        if (MapEqualCheck(removed_node, i, true)) continue;

        int adj0 = 0, adj1 = 0;
        auto adj = GetAdjacency(i);
        for (auto x : adj) {
            if (MapEqualCheck(removed_node, x, true) || i >= x) continue;

            const auto ekey = MakeEdgeKey(i, x);
            adj0 += (grouping[x] == 0) * edge_weight.at(ekey);
            adj1 += (grouping[x] == 1) * edge_weight.at(ekey);
        }

        ret += (grouping[i] == 0) * adj1;
        ret += (grouping[i] == 1) * adj0;
    }

    return ret;
}

// http://pages.cs.wisc.edu/~shuchi/courses/880-S07/scribe-notes/lecture07.pdf
pair<int, vector<int>> MaxCutGraph::ComputeLocalSearchCut(const vector<int> pregroup) const {
    custom_assert((int)pregroup.size() == num_nodes || pregroup.empty());
    
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
            if (MapEqualCheck(removed_node, i, true)) continue;

            int adj0 = 0, adj1 = 0;
            auto adj = GetAdjacency(i);
            for (auto x : adj) {
                if (MapEqualCheck(removed_node, x, true)) continue;

                const auto ekey = MakeEdgeKey(i, x);
                adj0 += (grouping[x] == 0) * edge_weight.at(ekey);
                adj1 += (grouping[x] == 1) * edge_weight.at(ekey);
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
pair<int, vector<int>> MaxCutGraph::ComputeMaxCutWithMQLib(const double max_exec_time) const {
    std::vector<Instance::InstanceTuple> edgeList;
    for (int i = 0; i < num_nodes; ++i) {
        if (MapEqualCheck(removed_node, i, true)) continue;

        auto adj = GetAdjacency(i);
        for (auto w : adj) {
            if (MapEqualCheck(removed_node, w, true) || i >= w) continue;
            auto edge_key = MakeEdgeKey(w, i);
            custom_assert(MapEqualCheck(edge_exists_lookup, edge_key, true));
            edgeList.push_back(Instance::InstanceTuple(std::make_pair(i+1, w+1), edge_weight.at(edge_key)));
        }
    }

    MaxCutInstance mi(edgeList, num_nodes + 1);
    Burer2002 heur(mi, max_exec_time, false, NULL);
    const MaxCutSimpleSolution& mcSol = heur.get_best_solution();

    return make_pair(mcSol.get_weight(), mcSol.get_assignments());
}


// O(|V| + |E|)
vector<vector<int>> MaxCutGraph::GetCliquesWithAtLeastOneInternal() const {
    vector<vector<int>> ret;
    vector<bool> visited(num_nodes, false);

    auto cmp = [&](int a, int b) {
        return g_adj_list.at(a).size() < g_adj_list.at(b).size();
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