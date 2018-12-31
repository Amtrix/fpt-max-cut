#include <bits/stdc++.h>
#include "./cpp-latex_table-utils.hpp"
using namespace std;

string RemoveSDIfPresent(string entry) {
    if (entry.back() == ')')
        return entry.substr(0, entry.size() - string(" (0.00)").size());
    return entry;
}

Table SortMethodA(Table source) {
    sort(source.second.begin(), source.second.end(), [&](auto rowA, auto rowB) {
        int coldx = GetColumnIndex(source, "GRAPH_DENSITY");
        string strA = rowA[coldx], strB = rowB[coldx];

        double valA = stod(RemoveSDIfPresent(strA)), valB = stod(RemoveSDIfPresent(strB));
        return valA < valB;
    });

    return source;
}

Table SortMethodB(Table source) {
    sort(source.second.begin(), source.second.end(), [&](auto rowA, auto rowB) {
        int coldx = GetColumnIndex(source, "#num_edges");
        string strA = rowA[coldx], strB = rowB[coldx];

        double valA = stod(RemoveSDIfPresent(strA)), valB = stod(RemoveSDIfPresent(strB));
        return valA < valB;
    });

    return source;
}

string latexify(string w) {
    string ret = "";
    for (int i = 0; i < w.size(); ++i)  {
        if (w[i] == '_') ret += "\\_";
        else ret += w[i];
    }
    return ret;
}

void HandleLinearKernel(Table table) {
    CreateNewColumn(table, "GRAPH_DENSITY");
    CreateNewColumn(table, "GRAPH_NAME");
    table = Aggregate(table, "#sec");
    table = SortMethodB(table);

    vector<string> interesting = {"GRAPH_NAME", "#num_nodes", "#num_edges", "#marked_time", "#marked_cnt", "#marked_reduc_time", "#marked_reduc_cnt"};

    table = GetColumnSubsetTable(table, interesting);

    PrintTable(table);
    cout << endl << endl;

    for (int r = 0; r < table.second.size(); ++r) {
        vector<string> row = table.second[r];

        for (int i = 0; i + 1 < row.size(); ++i)
            cout << latexify(row[i]) << " & ";
        cout << latexify(row.back()) << " \\\\ \\hline" << endl;
    }
}

int main() {
    ifstream in("experiments/linear-kernel/real-world-small/out");
    Table table = ReadTable(in);
    
    PrintTable(table);
    cout << endl << endl << endl << endl;

    HandleLinearKernel(table);
}