#include <bits/stdc++.h>
#include "./cpp-latex_table-utils.hpp"
using namespace std;




vector<string> paths = {
    "experiments/kernelization/n2048/out",
    "experiments/kernelization/n2048/missing-r8-only-signed/out",
    "experiments/kernelization/n2048/missing-r8-with-signed/out",
    "experiments/kernelization/n2048/missing-s2/out",
    "experiments/kernelization/n2048/missing-s3/out",
    "experiments/kernelization/n2048/missing-s5/out",
    "experiments/kernelization/n2048/missing-s6/out"
};

Table table;
vector<Table> subtables;

const string selected_type = "RHG";

int main() {
    
    for (int i = 0; i < paths.size(); ++i) {
        ifstream in(paths[i].c_str());

        if (!in.is_open()) {
            cout << "Couldn't open: " << paths[i] << endl;
            exit(-1);
        }

        subtables.push_back(ReadTable(in));

        CreateNewColumn(subtables[i], "kagentype", [&](vector<string> row) {
            int dx = GetColumnIndex(subtables[i], "#file");
            string f = row[dx];
            int cnt = 0;
            while (cnt < f.size() && f[cnt] != '-') cnt++;
            return f.substr(0, cnt);
        });

        subtables[i] = GetSubset(subtables[i], "kagentype", selected_type);

        SetColumnValue(subtables[i], "#sec", [&](vector<string>row) { return to_string(i); });
        
        AppendTable(table, subtables[i]);
    }

    ofstream out(("experiments/kernelization/n2048/isolated-aggregate/isolated-rules-" + selected_type).c_str());
    PrintTable(table, out);
}