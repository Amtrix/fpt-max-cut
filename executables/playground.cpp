#include <bits/stdc++.h>
#include <functional>
#include "src/mc-graph.hpp"
#include "src/input-parser.hpp"
using namespace std;

vector<int> GetMaxcutDependentOnNc(const vector<pair<int,int>> &edges, int n, int nc) {
    vector<int> maxcut_dependent_on_nc; // sorted according to lex bitmask of nc
    for (int nc_mask = 0; nc_mask * 2 < (1 << nc); ++nc_mask) { // simetry!
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
    return maxcut_dependent_on_nc;
}

void ConnectNCToAll(int ncid, vector<pair<int,int>>& dest_edges, int n, int nc) {
    for (int i = nc; i < n; ++i) {
        dest_edges.push_back(make_pair(ncid, i));
    }
}

void ConnectNonNCToClique(vector<pair<int,int>>& dest_edges, int n, int nc) {
    for (int i = nc; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            dest_edges.push_back(make_pair(i, j));
        }
    }
}


string EncodeDiff(const vector<int> &cuts) {
    string ret = "";
    for (int i = 0; i + 1 < (int)cuts.size(); ++i)
        ret += (to_string(cuts[i] - cuts[i+1])) + ".";
    return ret;
}

void PrintVec(const vector<int> cuts) {
    for (int i = 0; i < (int)cuts.size(); ++i)
        cout << cuts[i] << " ";
    cout << endl;
}

int binom(int n, int k) 
{ 
    int res = 1; 
  
    // Since C(n, k) = C(n, n-k) 
    if ( k > n - k ) 
        k = n - k; 
  
    // Calculate value of [n * (n-1) *---* (n-k+1)] / [k * (k-1) *----* 1] 
    for (int i = 0; i < k; ++i) 
    { 
        res *= (n - i); 
        res /= (i + 1); 
    } 
  
    return res; 
} 

int sumdouble(int from, int rem) {
    int ret = 0;
    for (int i = from; i <= (from-1) + (rem / 2); ++i)
        ret += i + i;
    return ret + (rem % 2) * (rem / 2 + from);
}

int seq(int n) {
    return (n*n + 6*n + 1)/4;
}

int seq2(int n, int m) {
    int tmp = n * n - 3 * m * m + 2 * n * m - 6 * n + 10 * m - 7;
    return  tmp / 4;
}

int main() {
    int nc = 5;
    int n1 = 5, n2 = 5;

    cout << "Input number of vertices outside of the clique" << endl;
//    cin >> nc;

    while (cin >> nc >> n1 >> n2) {

        vector<pair<int,int>> edges1, edges2;
        for (int i = 0; i < nc; ++i) {
            ConnectNCToAll(i, edges1, n1, nc);
            ConnectNCToAll(i, edges2, n2, nc);
        }

        ConnectNonNCToClique(edges1, n1, nc);
        ConnectNonNCToClique(edges2, n2, nc);

        edges1.push_back(make_pair(0, 1));
        edges2.push_back(make_pair(0, 1));
        

        int n = n1 - nc;
        int m = nc;
        int rem = n - nc + 1;
        cout << "    " << "|S| = " << n1 - nc << ", nc = " << nc << endl;
        cout << "    " << "Remove: " << rem <<  " ( = " << sumdouble(nc, rem) << " = " << seq(rem + nc*2 - 2*2) - seq(nc*2 - 2*2) << " = " << seq(n + m - 3) - seq(2 * m - 4) << endl;
        cout << "    " << EncodeDiff(GetMaxcutDependentOnNc(edges1, n1, nc)) << endl;
        cout << "    " << EncodeDiff(GetMaxcutDependentOnNc(edges2, n2, nc)) << endl;
        cout << " ----------------------- " << endl;
        cout << "    "; PrintVec(GetMaxcutDependentOnNc(edges1, n1, nc));
        cout << "    "; PrintVec(GetMaxcutDependentOnNc(edges1, n1, nc));
    }
}