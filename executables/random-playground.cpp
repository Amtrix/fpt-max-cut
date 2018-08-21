#include <bits/stdc++.h>
#include <functional>
#include "src/mc-graph.hpp"
using namespace std;

const bool kSkipSingletons = false;
const bool kStopAtSame = false;

const int n = 5;
const int nc = 5;
map<pair<int,int>, bool> preadd = {
};

map<pair<int,int>, bool> subset_in_result = {
    //{make_pair(0,2), true},
    //{make_pair(0,3), true},
    //{make_pair(0,4), true},
   // {make_pair(0,5), true},
    //{make_pair(0,6), true},
    

   // {make_pair(1,2), true},
   // {make_pair(1,3), true},
    ////{make_pair(1,4), true},
    //{make_pair(1,5), true},
    //{make_pair(1,6), true},
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

    int subset_in_result_cnt_start = subset_in_result.size();
    for (auto entry : equiv_cls) {
        if (entry.second.size() <= 1 && kSkipSingletons) continue;

        cout << "Class " << entry.first << " = " << entry.second.size() << endl;

        bool found_exact = false;
        for (auto e : entry.second) {
            cout << "     ";
            cout << "[sz: " << e.second.size() << ", mx(0): " << e.first << "] = ";

            // Print edges and check if they contain subset_in_result as a subset.
            map<pair<int,int>,bool> visi;
            int subset_in_result_cnt = subset_in_result_cnt_start;
            for (int i = 0; i < (int)e.second.size(); ++i) {
                auto edge = e.second[i];
                subset_in_result_cnt -= (visi[edge] == false) && (subset_in_result[edge] || subset_in_result[RevPair(edge)]);
                
                cout << "(" << edge.first << ", " << edge.second << ") ";
                visi[edge] = true;
            }

            if (subset_in_result_cnt_start != 0 && subset_in_result_cnt == 0) {
                if ((int)e.second.size() != subset_in_result_cnt_start)
                    cout << " *********** ";
                else {
                    cout << " ########### ";
                    found_exact = true;
                }
            }

            cout << endl;
        }
        cout << endl << endl;
    
        if (found_exact && kStopAtSame) {
            cout << "Break because exact found." << endl;
            break;
        }
    }
}