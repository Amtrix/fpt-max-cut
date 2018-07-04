#include <bits/stdc++.h>
#include <functional>
#include "src/mc-graph.hpp"
using namespace std;

int C_sz = 9;
vector<int> partition_a = {2};
vector<int> partition_b = {3};
vector<pair<int,int>> add_sx = {
  //  {2, 0}
};

vector<int> Sx = {1};


bool SatisfiesRule8(vector<int> partition, int dx) {
    int csz = 0;
    for (auto v : partition) csz += v;

    double rval = (csz + Sx[dx]) / 2.0;
    double lval = partition[dx];
    cout << lval << " > " << rval << endl;
    return lval > rval;
}

int GetMaxCut(vector<int> partition, vector<int>& w_diff, vector<pair<int,int>> add_sx) {
    vector<int> w_diff_new = w_diff;
    for (int i = 0; i < (int)add_sx.size(); ++i)
        w_diff_new[add_sx[i].second] += w_diff[add_sx[i].first];

    vector<int> w_diff_tot;
    int curr = 0;
    for (int i = 0; i < (int)partition.size(); ++i)
        for (int j = 0; j < partition[i]; ++j) {
            w_diff_tot.push_back(w_diff_new[i]);
            curr += -w_diff_new[i]; // all set to 0
        }

    sort(w_diff_tot.rbegin(), w_diff_tot.rend());

    int res = curr;
    int csz = w_diff_tot.size();
 //   cout << "  " << csz << endl;
    for (int i = 0; i < csz; ++i) {
        int val = w_diff_tot[i];
        curr += 2 * val + (csz - i - 1) - i;
        res = max(res, curr);
    }

    return res;
}

vector<int> w_diff;
void GetSxColoring(int dx, std::function<void(vector<int>&)> callback) {
    if (dx == (int)Sx.size()) {
        callback(w_diff);
        return;
    }

    for (int i = -Sx[dx]; i <= Sx[dx]; ++i) {
        w_diff.push_back(i);
        GetSxColoring(dx + 1, callback);
        w_diff.pop_back();
    }
}

vector<pair<int,int>> aggregate;
bool match(vector<int> p1, vector<int> p2, vector<int> Sx) {
    (void) Sx;

    GetSxColoring(0, [&](vector<int>& color){
        int c1 = GetMaxCut(p1, color, {});
        int c2 = GetMaxCut(p2, color, add_sx);
       // for (auto c : color)
        //    cout << c << " ";
        //cout << " = " << c1 << " " << c2 << " [ " << c1 - c2 << " ]" << endl;
        aggregate.push_back(make_pair(c1, c2));
    });
    return false;
}

int main() {
    cout << "NOTE: These are absolute differences in cut size. Papers use above edwards erdos value." << endl;
    assert(partition_b.size() == partition_a.size() && partition_a.size() == Sx.size());
    cout << "Satisfies? " << SatisfiesRule8(partition_a, 0) << endl;

    unordered_map<string, bool> visited;
    while(1) {
        aggregate.clear();

        int mx = 0;
        for (int e : partition_a) mx = max(mx, e);
        for (int i = 0; i < (int)partition_b.size(); ++i) partition_b[i] = rand()%(mx + 1);

        string key = "";
        for (auto node : partition_a) key += to_string(node);
        for (auto node : partition_b) key += to_string(node);

        if (visited[key]) continue;
        visited[key] = true;

        match(partition_a, partition_b, Sx);

        bool ok = true;
        int cdiff = aggregate[0].first - aggregate[0].second;
        for (auto entry : aggregate)
            if (cdiff != entry.first - entry.second)
                ok = false;


        if (ok) {
            cout << "OK: ";
            for (int i = 0; i < (int)partition_b.size(); ++i)
                cout << partition_b[i] << " ";
            cout << endl;
        }
    }


  //  vector<int> col = {0, 0, 1};
  //  cout << GetMaxCut(partition_a, col) << endl;

   // vector<int> col = {1, 1, 1};
   // cout << GetMaxCut(partition_a, col) << endl;

  //  cout << GetMaxCut(partition_b, col) << endl;
}