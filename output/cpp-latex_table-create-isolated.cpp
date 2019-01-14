#include <bits/stdc++.h>
#include "./cpp-latex_table-utils.hpp"
using namespace std;




vector<string> paths = {
    "/kernelization/n2048/out",
    "/kernelization/n2048/missing-r8-with-signed/out", //                5+
    "/kernelization/n2048/missing-s2/out",             // Reduction Rule 23 
    "/kernelization/n2048/missing-s3/out",             //                25     (missing edge rule)
    "/kernelization/n2048/missing-s5/out",             //                24
  //  "/kernelization/n2048/missing-s6/out"
};

Table table;
vector<Table> subtables;



int main(int argc, char **argv){
    if (argc < 3) {
        throw std::logic_error("Path to experiment required. Type of KaGen graphs required (BA, RGG2D, RGG3D, RHG, or GNM).");
        exit(-1);
    }

    const string root_dir(argv[1]);
    const string selected_type(argv[2]);
    
    for (int i = 0; i < paths.size(); ++i) {
        ifstream in((root_dir + paths[i]).c_str());

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