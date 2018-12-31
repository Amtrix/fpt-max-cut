#include <bits/stdc++.h>
#include "./cpp-latex_table-utils.hpp"
using namespace std;

int main() {
    ifstream in("experiments/linear-kernel/real-world-small/out");

    Table table = ReadTable(in);

    for (int i = 0; i < table.first.size(); ++i) {
        cout << i << " " << table.first[i] << endl;
    }

    PrintTable(table);

    cout << endl << endl;

    PrintTable(Aggregate(table, "#sec"));
}