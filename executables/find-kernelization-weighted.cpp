#include <bits/stdc++.h>
#include <functional>
#include "src/mc-graph.hpp"
using namespace std;

const bool kSignedCliqueMode = true;

typedef vector<pair<int,int>> graph_edges;

const bool kRequireLClique = false;

const bool kRoughAnalaysis = false;
const bool kHandleAnyProperty = false;
const bool kSkipSingletons = false;
const bool kStopAtSame = false;
const bool kBreakWhenSmaller = false;
const bool kKernelizeAndVisit = true;
const bool kRemoveIsomorphisms = true;

const int kSampleMode = -1; // -1 for normal mode, -2 for specific sampling, -3 for bfs with <=2-removal(take first entry from specific_sampling_set as start)

int n = 7; 
int nc = 3;

// when nc = 2:
// 2=>2 classes, 3=>3 classes, 4=>7 classes, 5=> 5 classes, 6=>11 classes, 7=>7 classes, 

// when nc = 3:
// 3=>5 classes, 4=>34 classes, 5=> 55 classes, 6=>311 classes, 7=>153 classes, 

map<pair<int,int>, bool> preadd = {
};

map<pair<int,int>, bool> subset_in_result = {
};

map<pair<int,int>, bool> any_in_result = {
};

vector<vector<pair<int,int>>> specific_sampling_set = {
   /* {{0,1}, {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {1,2}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7},
     {2,3}, {2,4}, {2,5}, {2,6}, {2,7}, {3,4}, {3,5}, {3,6},
     {4,5}, {4,6}, {4,7}, {5,6}, {5,7}, {6,7}},*/

     {{0,1}, {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,9},     {1,2}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8}, {1,9},      {2,3}, {2,4}, {2,5}, {2,6}, {2,7}, {2,8}, {2,9},            {3,4}, {3,5}, {3,6}, {3,7}, {3,8}, {3,9},    {4,5}, {4,6}, {4,7}, {4,8}, 
      {5,6}, {5,7}, {5,8}, {5,9}, {6,7}, {6,8}, {6,9}, {7,8}, {7,9}, {8,9}}


      /*{{0,1}, {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,9}, {0,10}, {0,11},     {1,2}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8}, {1,9}, {1,10}, {1,11},      {2,3}, {2,4}, {2,5}, {2,6}, {2,7}, {2,8}, {2,9}, {2,10}, {2,11},            
                        {3,4}, {3,5}, {3,6}, {3,7}, {3,8}, {3,9}, {3,10}, {3,11},    {4,5}, {4,6}, {4,7}, {4,8}, {4,10}, {4,11}, {5,6}, {5,7}, {5,8}, {5,10},
       {6,7}, {6,8}, {6,9}, {6,10}, {6,11}, {7,8}, {7,9}, {7,10}, {7,11}, {8,9}, {8,10}, {8,11}, {9,10}, {9,11}, {10,11}}*/
};

vector<int> L_vertex, R_vertex;
unordered_map<int, bool> preset_is_external;

vector<tuple<int,int,int>> MakeUnweighted(const vector<pair<int,int>> &edges) {
    vector<tuple<int,int,int>> ret;
    for (auto e : edges)
        ret.push_back(make_tuple(e.first, e.second, 1));
    return ret;
}

string GetGraphKey(const vector<tuple<int,int,int>>& edges) {
    string ret = "";
    for (auto e : edges)
        ret += "(" + to_string(get<0>(e)) + "," + to_string(get<1>(e)) + + "," + to_string(get<2>(e)) + ")";
    return ret;
}
string GetGraphKey(const vector<pair<int,int>>& edges) {
    return GetGraphKey(MakeUnweighted(edges));
}

string EncodeDiff(const vector<int> &cuts) {
    string ret = "";
    for (int i = 0; i + 1 < (int)cuts.size(); ++i)
        ret += (to_string(cuts[i] - cuts[i+1])) + ".";
    return ret;
}

vector<int> GetWeightedMaxcutDependentOnNc(const vector<tuple<int,int,int>> &edges) {
    for (auto edge : edges)
        cout << "(" << (get<0>(edge)) << ", " << (get<1>(edge)) << ", " << (get<2>(edge)) << ") ";
    cout << endl;

    vector<int> maxcut_dependent_on_nc; // sorted according to lex bitmask of nc
    for (int nc_mask = 0; nc_mask * 2 < (1 << nc); ++nc_mask) { // simetry!
        int mx_cut = -99999999;
        vector<int> color_nc;
        for (int i = 0; i < nc; ++i) color_nc.push_back((nc_mask & (1 << i)) != 0);

        for (int nrem_mask = 0; nrem_mask < (1 << (n-nc)); ++nrem_mask) {
            vector<int> color = color_nc;
            for (int i = 0; i < n - nc; ++i) color.push_back((nrem_mask & (1 << i)) != 0);

            int cut = 0;
            for (auto e : edges) cut += (color[get<0>(e)] != color[get<1>(e)]) * (get<2>(e));
            mx_cut = max(mx_cut, cut);
        }
        maxcut_dependent_on_nc.push_back(mx_cut);

        
       cout << "  ";
        for (int i = 0; i < (int)color_nc.size(); ++i)
            cout << color_nc[i];
        cout << " = " << mx_cut << endl;
    }
    exit(0);
    return maxcut_dependent_on_nc;
}

vector<int> GetMaxcutDependentOnNc(const vector<pair<int,int>> &edges) {
    return GetWeightedMaxcutDependentOnNc(MakeUnweighted(edges));
}




void TryAllWeightedCliqueEdgeSets(int n, std::function<void(const vector<tuple<int,int,int>>&)> callback) {
    int mx_edges = (n * (n - 1)) / 2;

    int bound = pow(2, mx_edges);
    cout << "NUM OF EDGE SETS: " << bound << endl;
    if (kSampleMode != -1) bound = kSampleMode;
    for (int i = 0; i < bound; ++i) {
        int mask = i;
        if (kSampleMode != -1) mask = rand();

        vector<int> c;
        for (int i = 0; i < mx_edges; ++i) {
            c.push_back(mask % 2);
            mask /= 2;
        }

        vector<tuple<int,int,int>> cumm;
        int dx = 0;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (c[dx] == 0)
                    cumm.push_back(make_tuple(i,j,-1));
              //  else if (c[dx] == 1)
                //    cumm.push_back(make_tuple(i,j,-1));
                dx++;
            }
        }

        callback(cumm);
    }
}

inline bool IsExternal(int id) {
    return id < nc;
}

void GetAllIsomorphisms(vector<pair<int,int>> elist, std::function<void(vector<pair<int,int>>&)> callback) {
    vector<int> vorder;
    for (int i = 0; i < n; ++i) vorder.push_back(i);

    do {
        vector<pair<int,int>> new_elist = elist;
        bool ok = true;
        for (int i = 0; i < (int)new_elist.size(); ++i) {
            if (IsExternal(new_elist[i].first) != IsExternal(vorder[new_elist[i].first])) { ok = false; break; }
            if (IsExternal(new_elist[i].second) != IsExternal(vorder[new_elist[i].second])) { ok = false; break; }
            
            new_elist[i].first = vorder[new_elist[i].first];
            new_elist[i].second = vorder[new_elist[i].second];

            if (new_elist[i].first > new_elist[i].second)
                swap(new_elist[i].first, new_elist[i].second);
        }

        if (ok) {
            sort(new_elist.begin(), new_elist.end());
            callback(new_elist);
        }
    } while (next_permutation(vorder.begin(), vorder.end()));
}

vector<pair<int,int>> GetLexicographicallyLowestIso(vector<pair<int,int>> elist) {
    auto sel = elist;
    GetAllIsomorphisms(elist,  [&](vector<pair<int,int>>& edges){
        if (sel > edges) sel = edges;
    });
    return sel;
}

vector<pair<int,int>> IncreaseBy1(const vector<pair<int,int>> &v) {
    vector<pair<int,int>> ret;
    for (int i = 0; i < (int)v.size(); ++i)
        ret.push_back(make_pair(v[i].first + 1, v[i].second + 1));
    return ret;
}

//void GetAllIsomorphismKeys(MaxCutGraph& G, std::function<void(vector<pair<int,int>>&)> callback) {

pair<int,int> RevPair(pair<int,int> p) { return make_pair(p.second, p.first); }


int main() {
    ios_base::sync_with_stdio(false);
    cin >> n >> nc;

    for (int i = 0; i < nc; ++i) preset_is_external[i] = true;
    for (int i = 0; i < n; ++i)
        if (i < nc) L_vertex.push_back(i);
        else R_vertex.push_back(i);



    /* Compute all graphs and remove isomorphic ones */
    vector<vector<tuple<int,int,int>>> all_graphs_edges;
    unordered_map<string, bool> class_count;
    unordered_map<string, bool> tot_class_count;

    if (kSignedCliqueMode) {
        TryAllWeightedCliqueEdgeSets(n, [&](const vector<tuple<int,int,int>>& init_edges){
            auto mxcutnc = GetWeightedMaxcutDependentOnNc(init_edges);
            class_count[EncodeDiff(mxcutnc)] = true;
            all_graphs_edges.push_back(init_edges);
        });
    } else {
     
    }

    

    if (kRoughAnalaysis) {
        cout << "Number of classes: " << class_count.size() << endl; // beware, using = false increases this too.
        cout << "Total number of classes -- including visited isos (with visited isomorphisms) [kRemoveIso has to be active]: " << tot_class_count.size() << endl; // beware, using = false increases this too.
        cout << "Graph set computation complete." << endl;
        return 0;
    }

    /* Calculate equivalence classes */
    unordered_map<string, vector<pair<int, vector<tuple<int,int,int>>>> > equiv_cls;
    unordered_map<string, bool> visited;
    for (auto edges : all_graphs_edges) {
        string edge_key = GetGraphKey(edges);
        if (visited[edge_key]) continue;
        visited[edge_key] = true;

        const auto maxcut_dependent_on_nc = GetWeightedMaxcutDependentOnNc(edges);
        const auto key = EncodeDiff(maxcut_dependent_on_nc);
        equiv_cls[key].push_back(make_pair(maxcut_dependent_on_nc[0], edges));
    }


    vector<pair<int,string>> ordered_classes;
    for (auto entry : equiv_cls) {
        int sz = entry.second.size();
        ordered_classes.push_back(make_pair(sz, entry.first));
    }
    sort(ordered_classes.rbegin(), ordered_classes.rend());

    auto cmpentries = [&](pair<int,vector<tuple<int,int,int>>> &entry1, pair<int,vector<tuple<int,int,int>>> &entry2) {
        return entry1.second.size() < entry2.second.size();
    };

    int num_of_classes = 0;
    double sum_coverage = 0;
    double sum_denum = 0;
    for (int i = 0; i < (int)ordered_classes.size(); ++i) {
        const string key = ordered_classes[i].second;
        vector<pair<int, vector<tuple<int,int,int>>>> entries = equiv_cls[key];
        if (entries.size() <= 1 && kSkipSingletons) continue;

        sort(entries.begin(), entries.end(), cmpentries);

        cout << "Class " << key << " = " << entries.size() << " (total!)  or  " << ordered_classes[i].first << " (filtered!)" << endl;
        num_of_classes++;

        bool found_exact = false;
        unordered_map<string, bool> visited_graph_key;
        int kernelized_count = 0;
        for (auto e : entries) { // iterate over all edge-sets in the class.
            auto graph_edges = e.second;

            cout << "     ";
            cout << "[sz: " << graph_edges.size() << ", mx(0): " << e.first << ", clsid: " << num_of_classes << "] = ";

            // Print edges and check if they contain subset_in_result as a subset.
            map<tuple<int,int,int>,bool> visi;
            int cnt_minus_one = 0, cnt_one = 0;
            for (int i = 0; i < (int)graph_edges.size(); ++i) {
                auto edge = graph_edges[i];
                
                cout << "(" << (get<0>(edge)) << ", " << (get<1>(edge)) << ", " << (get<2>(edge)) << ") ";

                if (get<2>(edge) == -1) cnt_minus_one++;
                if (get<2>(edge) == 1)  cnt_one++;
                
                visi[edge] = true;
            }

            cout << "      [";
            cout << cnt_minus_one << " " << cnt_one << " = " << -cnt_minus_one + cnt_one <<"     ";

            auto G = MaxCutGraph(graph_edges, n);
            cout << "clique(L,R)=(" << G.IsClique(L_vertex) << "," << G.IsClique(R_vertex) << "      ";

            
            cout << G.PrintDegrees(preset_is_external);
            cout << "  CONNECTED=" << (G.GetAllConnectedComponents().size() == 1u);
            cout << "]";

            cout << endl;
        }
        if (kKernelizeAndVisit) {
            double coverage = 1;
            if (entries.size() >= 2) coverage = (kernelized_count) / (double)(entries.size() - 1);
            sum_coverage += kernelized_count;
            sum_denum += entries.size() - 1;
            cout << "kernelized count: " << kernelized_count << " (coverage: " << coverage << ")" << endl;
        }
        cout << endl << endl;
    
        if (found_exact && kStopAtSame) {
            cout << "Break because exact found." << endl;
            break;
        }
    }

    cout << "Number of classes: " << num_of_classes << endl;
    cout << "Total kernelization coverage: " << (sum_denum > 1e-9 ? (sum_coverage / sum_denum) : 1) << endl;

    ofstream out("find-kernelization-general-stats", std::ios_base::app);
    out << "(" << n << " " << nc << ")" << endl;
    out << "Number of classes: " << num_of_classes << endl;
    out << "Total kernelization coverage: " << (sum_denum > 1e-9 ? (sum_coverage / sum_denum) : 1) << endl;
    out << endl;
}