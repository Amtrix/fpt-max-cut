#include <bits/stdc++.h>
using namespace std;

int n1,n2,nk;

int main() {
    cin >> n1 >> n2 >> nk;

    for (int taken_in_nk = 0; taken_in_nk <= nk; ++taken_in_nk) {
        int cut_size = 0;
        for (int i = 0; i < taken_in_nk; ++i)
            cut_size += /* new connections to remaining vertices */ nk - i - 1 /* substraction of destroyed connections to previous i */ - i;
        
        int nk_1 = taken_in_nk, nk_0 = nk - nk_1;

        cout << "Marked 1: " << nk_1 << ", 0: " << nk_0 << ", cut: " << cut_size << endl;

        for (int nsub : {n1 - nk, n2 - nk}) {
            int subcut_size = nsub * nk_1; // initially all marked as 0
            int mx_sub = subcut_size;
            for (int i = 0; i < nsub; ++i) {
                //int nsub_1 = i + 1;
                subcut_size = /* adjusting edges to nk */ (-nk_1 + nk_0) /* new edges to remainder */ + nsub - i - 1 /* substraction of destroyed connections to previous i */ - i;
                mx_sub = max(mx_sub, subcut_size);
            }
            cout << "  mx-sub: " << mx_sub << endl;
            cut_size += mx_sub;
        }

        cout << "CUT: " << cut_size << endl << endl;
    }
}