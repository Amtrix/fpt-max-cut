#include <bits/stdc++.h>
#include <functional>
#include "src/mc-graph.hpp"
#include "src/input-parser.hpp"
using namespace std;

bool stripmode = false;

int GetWeight() {
    return -1 + (rand()%3);
}

int GetFullWeight() {
    return -1 + (rand()%2 == 0 ? 2 : 0);
}

int GetGridNodeId(int h, int w, int r, int c) {
    (void) h;
    return r * w + c;
}

const int dir[][2] = {0,1,1,0,  0,-1,-1,0};

vector<tuple<int,int,int>> MakeGridGraph(const int h, const int w, bool full = false) {
    vector<tuple<int,int,int>> res;
    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            for (int d = 0; d < 2; ++d) {
                int nr = r + dir[d][0];
                int nc = c + dir[d][1];
                if (nr < 0 || nc < 0 || nr >= h || nc >= w) continue;

                int weight = GetWeight();
                if (full) weight = GetFullWeight();
                if (w != 0) {
                  //  cout << GetGridNodeId(h, w, r, c) << " " << GetGridNodeId(h, w, nr, nc) << " " << weight << endl;
                    res.push_back(make_tuple(GetGridNodeId(h, w, r, c), GetGridNodeId(h, w, nr, nc), weight));
                }
            }
        }
    }
    cout << endl;

    return res;
}

bool IsNc(int h, int w, int r, int c) {
    if (!stripmode) if (r == 0 || r == h - 1) return true;
    return c == 0 || c == w - 1;
}

vector<int> GetNC(int h, int w) {
    vector<int> ret;
    for (int r = 0; r < h; ++r) {
        if ((r == 0 || r == h - 1) && !stripmode) {
            for (int c = 0; c < w; ++c) {
                ret.push_back(GetGridNodeId(h, w, r, c));
            }
        } else {
            ret.push_back(GetGridNodeId(h, w, r, 0));
            ret.push_back(GetGridNodeId(h, w, r, w - 1));
        }
    }
    return ret;
}

MaxCutGraph BuildColoredGraph(int h, int w, vector<tuple<int,int,int>> data, int mask) {
    vector<int> ncs = GetNC(h, w);
    int designatedv = h * w;
    for (int i = 0; i < (int)ncs.size(); ++i) {
        if (mask&(1<<i)) {
            data.push_back(make_tuple(designatedv, ncs[i], 1e6));
        } else {
            data.push_back(make_tuple(designatedv, ncs[i], -1e6));
        }
    }

    return MaxCutGraph(data);
}

string EncodeDiff(const vector<int> &cuts) {
    string ret = "";
    for (int i = 0; i + 1 < (int)cuts.size(); ++i)
        ret += (to_string(cuts[i] - cuts[i+1])) + ".";
    return ret;
}

int GetMaxcutDependentOnNc(int h, int w, vector<tuple<int,int,int>> data, int mask) {
    vector<int> ncs = GetNC(h, w);
    vector<int> non_nc;
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j)
            if (IsNc(h, w, i, j) == false)
                non_nc.push_back(GetGridNodeId(h, w, i, j));
    
    vector<bool> colors(h * w, 0);
    for (int i = 0; i < (int)ncs.size(); ++i)
        colors[ncs[i]] = mask & (1 << i);

  //  cout << "SZ: " << non_nc.size() << " | " << mask << endl;

    int sol = -1e9;
    for (int mask2 = 0; mask2 < (1 << non_nc.size()); ++mask2) {
        for (int i = 0; i < (int)non_nc.size(); ++i)
            colors[non_nc[i]] = mask2 & (1 << i);
        
        int tmp = 0;
        for (auto e : data) {
            int a = get<0>(e);
            int b = get<1>(e);
            int weight = get<2>(e);
          //  cout << a << " " << b << " | " << colors[a] << " " << colors[b] << endl;
            tmp += weight * (colors[a] != colors[b]);
        }
        //for (int i = 0; i < colors.size(); ++i) cout << colors[i]; cout << " = " << tmp << endl;
        sol = max(sol, tmp);
    }
   // cout << "SOL: " << sol << endl;
    return sol;
}

map<string, vector<vector<tuple<int,int,int>>>> classes;

int main(int argc, char **argv){
    ios_base::sync_with_stdio(false);
    InputParser input(argc, argv);

    const int h = 3, w = 5;
    const vector<int> ncs = GetNC(h, w);

    vector<int> masks;
    masks.push_back(0);
  //  for (int i = 0; i < 20; ++i) {
  //      masks.push_back(rand()%(1 << ncs.size()));
  //  }
    for (int i = 0; i < (1 << ncs.size()); ++i) {
        masks.push_back(i);
    }

    for (int i = 0; i < 200000; ++i) {
        auto graph_data = MakeGridGraph(h, w);
        
        vector<int> mxcuts;
        for (int k = 0; k < (int)masks.size(); ++k) {
            int mask = masks[k];
           // auto G = BuildColoredGraph(h, w, graph_data, mask);
           // auto mc = G.ComputeMaxCutWithLocalsolver(2);
           // int mcsz = mc.first;
            int mcsz = GetMaxcutDependentOnNc(h, w, graph_data, mask);
           // cout << mcsz << " cmp " << GetMaxcutDependentOnNc(h, w, graph_data, mask) << endl;
            mxcuts.push_back(mcsz);
        //    exit(0);
        }

        string key = EncodeDiff(mxcuts);
        classes[key].push_back(graph_data);

        for (auto entry : classes) {
            if (entry.second.size() == 1) continue;
            cout << entry.second.size() << " : " << endl;// << entry.first << endl;
            for (auto elist : entry.second) {
                cout << "  ";
                for (auto e : elist)
                    if ((get<2>(e)) != 0) cout << "(" << (get<0>(e)) << " " << (get<1>(e)) << " " << (get<2>(e)) << ") ";
                cout << " = " << elist.size() << endl;
            }
        }
        cout << "=================================" << endl << endl;
    }

    return 0;
}