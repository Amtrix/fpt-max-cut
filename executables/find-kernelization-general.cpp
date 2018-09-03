#include <bits/stdc++.h>
#include <functional>
#include "src/mc-graph.hpp"
using namespace std;

const bool kSkipSingletons = false;
const bool kStopAtSame = false;
const bool kBreakWhenSmaller = false;
const bool kKernelizeAndVisit = true;

int n = 4;
int nc = 3;

map<pair<int,int>, bool> preadd = {
};

map<pair<int,int>, bool> subset_in_result = {
};

map<pair<int,int>, bool> any_in_result = {
    {make_pair(0, 1), true},
    {make_pair(0, 2), true},
    {make_pair(1, 2), true},
};

void TryAllEdgeSets(int n, std::function<void(vector<pair<int,int>>&)> callback) {
    int mx_edges = (n * (n - 1)) / 2;
    for (int mask = 0; mask < (1 << mx_edges); ++mask) {
        vector<pair<int,int>> cumm;
        int dx = 0;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if ((mask & (1 << dx)) || preadd[make_pair(i,j)] || preadd[make_pair(j,i)])
                    cumm.push_back(make_pair(i,j));
                dx++;
            }
        }

        callback(cumm);
    }
}

string GetGraphKey(MaxCutGraph &G) {
    string ret = "";
    auto edges = G.GetAllExistingEdges();
    for (auto e : edges)
        ret += "(" + to_string(e.first) + "," + to_string(e.second) +")";
    return ret;
}

//void GetAllIsomorphismKeys(MaxCutGraph& G, std::function<void(vector<pair<int,int>>&)> callback) {

pair<int,int> RevPair(pair<int,int> p) { return make_pair(p.second, p.first); }

string EncodeDiff(vector<int> &cuts) {
    string ret = "";
    for (int i = 0; i + 1 < (int)cuts.size(); ++i)
        ret += (to_string(cuts[i] - cuts[i+1])) + ".";
    return ret;
}

string EncodeEdgeSet(const vector<pair<int,int>>& w) {
    string ret = "";
    for (auto e : w)
        ret += "(" + to_string(e.first) + "," + to_string(e.second) + "):";
    return ret;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin >> n >> nc;

    unordered_map<string, vector<pair<int, vector<pair<int,int>>>> > equiv_cls;
    unordered_map<string, bool> visited;

    TryAllEdgeSets(n, [&](vector<pair<int,int>>& edges){
        string edge_key = EncodeEdgeSet(edges);
        if (visited[edge_key]) return;
        visited[edge_key] = true;

        vector<int> maxcut_dependent_on_nc; // sorted according to lex bitmask of nc
        for (int nc_mask = 0; nc_mask < (1 << nc); ++nc_mask) {
            int mx_cut = 0;
            for (int nrem_mask = 0; nrem_mask < (1 << (n-nc)); ++nrem_mask) {
                vector<int> color;
                for (int i = 0; i < nc; ++i) color.push_back((nc_mask & (1 << i)) != 0);
                for (int i = 0; i < n - nc; ++i) color.push_back((nrem_mask & (1 << i)) != 0);

                int cut = 0;
                for (auto e : edges) cut += color[e.first] != color[e.second];
                mx_cut = max(mx_cut, cut);
            }
            maxcut_dependent_on_nc.push_back(mx_cut);
        }

        const auto key = EncodeDiff(maxcut_dependent_on_nc);
        
        equiv_cls[key].push_back(make_pair(maxcut_dependent_on_nc[0], edges));
    });

    vector<pair<int,string>> ordered_classes;
    for (auto entry : equiv_cls) {
        int sz = entry.second.size();
        if (kKernelizeAndVisit) {
            map<string, bool> visited_graph_key;
            for (auto e : entry.second) {
                MaxCutGraph G(e.second);
                G.ExecuteExhaustiveKernelization();
                const string key = GetGraphKey(G);
                if (visited_graph_key[key]) { sz--; continue; }
                visited_graph_key[key] = true;
            }
        }
        ordered_classes.push_back(make_pair(sz, entry.first));
    }
    sort(ordered_classes.rbegin(), ordered_classes.rend());

    auto cmpentries = [&](pair<int,vector<pair<int,int>>> &entry1, pair<int,vector<pair<int,int>>> &entry2) {
        return entry1.second.size() < entry2.second.size();
    };

    int num_of_classes = 0;
    double sum_coverage = 0;
    int subset_in_result_cnt_start = subset_in_result.size();
    for (int i = 0; i < (int)ordered_classes.size(); ++i) {
        const string key = ordered_classes[i].second;
        vector<pair<int, vector<pair<int,int>>>> entries = equiv_cls[key];
        if (entries.size() <= 1 && kSkipSingletons) continue;

        sort(entries.begin(), entries.end(), cmpentries);

        cout << "Class " << key << " = " << entries.size() << " (total!)" << endl;
        num_of_classes++;

        bool found_exact = false;
        map<string, bool> visited_graph_key;
        int kernelized_count = 0;
        for (auto e : entries) {
            if (kKernelizeAndVisit) {
                MaxCutGraph G(e.second);
                G.ExecuteExhaustiveKernelization();
                const string key = GetGraphKey(G);
                if (visited_graph_key[key]) { kernelized_count++; continue; }
                visited_graph_key[key] = true;
            }

            cout << "     ";
            cout << "[sz: " << e.second.size() << ", mx(0): " << e.first << ", clsid: " << num_of_classes << "] = ";

            // Print edges and check if they contain subset_in_result as a subset.
            bool any_property = false;
            map<pair<int,int>,bool> visi;
            int subset_in_result_cnt = subset_in_result_cnt_start;
            for (int i = 0; i < (int)e.second.size(); ++i) {
                auto edge = e.second[i];
                subset_in_result_cnt -= (visi[edge] == false) && (subset_in_result[edge] || subset_in_result[RevPair(edge)]);
                if (any_in_result[edge]) any_property = true;
                
                cout << "(" << edge.first << ", " << edge.second << ") ";
                visi[edge] = true;
            }

            cout << "  [";
            if (subset_in_result_cnt_start != 0 && subset_in_result_cnt == 0) {
                if ((int)e.second.size() != subset_in_result_cnt_start)
                    cout << " sub:1 ";
                else {
                    cout << " eq:1 ";
                    found_exact = true;
                }
            }

            if (any_property)
                cout << " any:1 ";
            else
                cout << " any:0 ";
            
            cout << "]";

            if (subset_in_result_cnt_start != 0 && subset_in_result_cnt == 0) {
                if (subset_in_result_cnt_start > (int)e.second.size() && kBreakWhenSmaller) {
                    cout << "Break because reduction of given subset found" << endl;
                    break;
                }
            }

            cout << endl;
        }
        if (kKernelizeAndVisit) {
            double coverage = 1;
            if (entries.size() >= 2) coverage = (kernelized_count) / (double)(entries.size() - 1);
            sum_coverage += coverage;
            cout << "kernelized count: " << kernelized_count << " (coverage: " << coverage << ")" << endl;
        }
        cout << endl << endl;
    
        if (found_exact && kStopAtSame) {
            cout << "Break because exact found." << endl;
            break;
        }
    }

    cout << "Number of classes: " << num_of_classes << endl;
    cout << "Total kernelization coverage: " << (sum_coverage / num_of_classes) << endl;
}