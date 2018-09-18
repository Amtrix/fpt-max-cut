#include <bits/stdc++.h>
#include <functional>
#include "src/mc-graph.hpp"
using namespace std;

const bool kSkipSingletons = false;
const bool kStopAtSame = false;
const bool kBreakWhenSmaller = false;
const int runs = 1000000;

int n = 4;
int nc = 3;

map<pair<int,int>, bool> preadd = {
};

map<pair<int,int>, bool> subset_in_result = {
};



void TryAllEdgeSets(int n, std::function<void(vector<pair<int,int>>&)> callback) {
    //int mx_edges = (n * (n - 1)) / 2;
    for (int i = 0; i <= runs; ++i) {
        int mask = rand();
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
    ios_base::sync_with_stdio(false);
    cin >> n >> nc;

    unordered_map<string, vector<pair<int, vector<pair<int,int>>>> > equiv_cls;
    unordered_map<string, bool> visited;
    unordered_map<string, bool> visited_key;
    int num_of_classes = 0;

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
        num_of_classes += visited_key[key] == false;
        visited_key[key] = true;

    });

    cout << "Number of classes: " << num_of_classes << endl;
}