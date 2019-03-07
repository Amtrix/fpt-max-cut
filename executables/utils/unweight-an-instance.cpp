#include <bits/stdc++.h>
#include "src/input-parser.hpp"
using namespace std;


int main(int argc, char **argv){
    ios_base::sync_with_stdio(false);
    InputParser input(argc, argv);

    int n,m;
    cin >> n >> m;
    cout << n << " " << m << endl;
    for (int i = 0; i < m; ++i) {
        int a,b,c;
        cin >> a >> b >> c;
        cout << a << " " << b << " " << 1 << endl;
    }
}