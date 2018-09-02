#include <bits/stdc++.h>
#include "src/utils.hpp"
using namespace std;

// Rule1: If clique of odd size n and has n - 2 external, remove edge between non-external


int n = 9;
int nc = 7;

string EncodeDiff(vector<int> &cuts) {
    string ret = "";
    for (int i = 0; i + 1 < (int)cuts.size(); ++i)
        ret += (to_string(cuts[i] - cuts[i+1])) + ".";
    return ret;
}

int GetMaxCut(const vector<bool> color_base, const vector<pair<int,int>> &edges) {
    int ret = 0;
    for (int rem_mask = 0; rem_mask < (1 << (n - nc)); ++rem_mask) {
        vector<bool> color = color_base;
        for (int i = 0; i < n-nc; ++i) color.push_back(rem_mask & (1 << i));

        int curr = 0;
        for (auto e : edges)
            curr += color[e.first] != color[e.second];

        ret = max(ret, curr);
    }

    return ret;
}

string GetMaxCutClass(const vector<pair<int,int>> &edges) {
    vector<int> sol;
    for (int nc_mask = 0; nc_mask < (1 << nc); ++nc_mask) {
        vector<bool> color;
        for (int i = 0; i < nc; ++i) color.push_back(nc_mask & (1 << i));
        int subsol = GetMaxCut(color, edges);
        sol.push_back(subsol);
    }

    return EncodeDiff(sol);
}

vector<pair<int,int>> GetCliqueEdges(const int n) {
    vector<pair<int,int>> ret;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            ret.push_back(make_pair(i,j));
    return ret;
}

void printedges(vector<pair<int,int>> edges) {
    for (auto e : edges)
        cout << "(" << e.first << ", " << e.second << ") ";
    cout << endl;
}

bool TryRemoveEdge(const vector<pair<int,int>> &edges, std::function<bool(pair<int,int>, vector<pair<int,int>>&)> callback) {
    for (auto e : edges) {
        auto cpye = edges;
        auto it = std::find(cpye.cbegin(), cpye.cend(), e);
        cpye.erase(it);
        if (callback(e, cpye)) return true;
    }
    return false;
}

vector<pair<int,int>> EdgeConcat(const vector<pair<int,int>> &A, const vector<pair<int,int>> &B) {
    vector<pair<int,int>> ret = A;
    ret.insert( ret.end(), B.begin(), B.end());
    return ret;
}

bool TryRemoveNEdge(const int N, const vector<pair<int,int>> erased, const vector<pair<int,int>> &edges, std::function<bool(vector<pair<int,int>>, vector<pair<int,int>>&)> callback) {
    return TryRemoveEdge(edges, [&](pair<int,int> erased_edge, vector<pair<int,int>>& edges){
        if (erased.empty() == false && erased.back() > erased_edge) return false;

        if (N > 1) return TryRemoveNEdge(N - 1, EdgeConcat(erased, {erased_edge}), edges, callback);
        else return callback(EdgeConcat(erased, {erased_edge}), edges);

        return false;
    });
}

void TryTripleRemove(const string basesol, const vector<pair<int,int>> &edges) {
    TryRemoveNEdge(5, {}, edges, [&](vector<pair<int,int>> erased, vector<pair<int,int>>& edges){
        string cmpsol = GetMaxCutClass(edges);
        if (basesol == cmpsol) {
            cout << "Reduction: " << endl;
            cout << "ERASE-TRIPLE: " << "X "<< erased.size() << endl;
            cout << "mx(0) = " << GetMaxCut(vector<bool>(nc, 0), edges) << endl;
            printedges(edges);
            cout << endl << endl;
        }
        return false;
    });
}

void TryHowManyEdgesRemovable(const string basesol, const vector<pair<int,int>> &edges) {
    for (int i = 1; i <= n * (n-1) / 2; ++i) {
        bool possible = false;
        cout << "Can remove " << i << " edges?" << endl;
        TryRemoveNEdge(i, {}, edges, [&](vector<pair<int,int>> erased, vector<pair<int,int>>& edges){
            string cmpsol = GetMaxCutClass(edges);
            if (basesol == cmpsol) {
                for (auto e : erased) cout << e.first << " " << e.second << " ,  ";
                cout << endl;
                possible = true;
                return true;
            }
            return false;
        });
        cout << (possible ? "yes" : "no") << endl;
    }
}

int main() {
    string basesol = GetMaxCutClass(GetCliqueEdges(n));
    
    vector<pair<int,int>> cmp_edges;
    //for (int i = 0; i < nc; ++i) // look into this too, there is something to this +-2... for n = 7,nc=5
    //    for (int j = i + 1; j < nc; ++j)
    //        cmp_edges.push_back(make_pair(i, j));
    
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (i < nc || j < nc)
                cmp_edges.push_back(make_pair(i, j));

    string cmpsol = GetMaxCutClass(cmp_edges);

    cout << "Test #1: " << (basesol == cmpsol) << endl;

    auto edges = GetCliqueEdges(n);
    cout << "Base edge list: " << endl;
    printedges(edges);
    cout << "Base mx(0) = " << GetMaxCut(vector<bool>(nc, 0), edges) << endl;
    //cout << "Base key = " << basesol << endl;
    cout << endl;

    TryHowManyEdgesRemovable(basesol, edges);

    /*
    TryRemoveEdge(edges, [&](pair<int,int> erased, vector<pair<int,int>>& edges){
        string cmpsol = GetMaxCutClass(edges);
        if (basesol == cmpsol) {
            cout << "Reduction: " << endl;
            cout << "ERASE: " << erased.first << " " << erased.second << endl;
            cout << "mx(0) = " << GetMaxCut(vector<bool>(nc, 0), edges) << endl;
            printedges(edges);
            cout << endl << endl;

            TryTripleRemove(basesol, edges);
        }
        return false;
    });*/

    
    
    return 0;
}