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

string latexify(string w, bool remove_decimals = false) {
    string ret = "";
    int period = 0;
    for (int i = 0; i < w.size(); ++i)  {
        if (w[i] == '_') ret += "\\_";
        else ret += w[i];
    }

    if (remove_decimals) return ret.substr(0, w.size() - 3);
    return ret;
}

void HandleLinearKernel(Table table) {
    CreateNewColumn(table, "GRAPH_DENSITY");
    CreateNewColumn(table, "GRAPH_NAME");
    CreateNewColumn(table, "custom_S_part", [&](vector<string> row) {
        double marked_cnt = GetValInRow(table, row, "#marked_cnt");
        double num_nodes = GetValInRow(table, row, "#num_nodes");
        return to_string_with_precision(marked_cnt / num_nodes, 2);
    });
    CreateNewColumn(table, "custom_Sx_part", [&](vector<string> row) {
        double marked_cnt = GetValInRow(table, row, "#marked_reduc_cnt");
        double num_nodes = GetValInRow(table, row, "#num_nodes");
        return to_string_with_precision(marked_cnt / num_nodes, 2);
    });

    table = Aggregate(table, "#sec");
    table = SortMethodA(table);

    CreateNewColumn(table, "custom_S_part_nice", [&](vector<string> row) {
        string spart = GetStrInRow(table, row, "custom_S_part");
        string marked_cnt = GetStrInRow(table, row, "#marked_cnt");
        return latexify(marked_cnt, true) + " [" + latexify(spart, false) + "]";
    });
    CreateNewColumn(table, "custom_Sx_part_nice", [&](vector<string> row) {
        string spart = GetStrInRow(table, row, "custom_Sx_part");
        string marked_cnt = GetStrInRow(table, row, "#marked_reduc_cnt");
        return latexify(marked_cnt, true) + " [" + latexify(spart, false) + "]";
    });

    vector<bool> remove_decimals = { false,          true,      false,    false,          false,            false,               false };
    vector<string> interesting = {"GRAPH_NAME","#num_edges", "GRAPH_DENSITY", "#marked_time", "custom_S_part_nice", "#marked_reduc_time", "custom_Sx_part_nice"};

    table = GetColumnSubsetTable(table, interesting);

    PrintTable(table);
    cout << endl << endl;

    for (int r = 0; r < table.second.size(); ++r) {
        vector<string> row = table.second[r];

        for (int i = 0; i + 1 < row.size(); ++i)
            cout << latexify(row[i], remove_decimals[i]) << " & ";
        cout << latexify(row.back(), remove_decimals.back()) << " \\\\ \\hline" << endl;
    }
}

void HandleLinearKernelEfficiency(Table table) {
    CreateNewColumn(table, "GRAPH_DENSITY");
    CreateNewColumn(table, "VEFFICIENCY");
    CreateNewColumn(table, "GRAPH_NAME");
    CreateNewColumn(table, "custom_S_part", [&](vector<string> row) {
        double marked_cnt = GetValInRow(table, row, "#marked_cnt");
        double num_nodes = GetValInRow(table, row, "#num_nodes");
        return to_string_with_precision(marked_cnt / num_nodes, 2);
    });
    CreateNewColumn(table, "custom_Sx_part", [&](vector<string> row) {
        double marked_cnt = GetValInRow(table, row, "#marked_reduc_cnt");
        double num_nodes = GetValInRow(table, row, "#num_nodes");
        return to_string_with_precision(marked_cnt / num_nodes, 2);
    });

    table = Aggregate(table, "#sec");
    table = SortMethodA(table);

    CreateNewColumn(table, "custom_S_part_nice", [&](vector<string> row) {
        string spart = GetStrInRow(table, row, "custom_S_part");
        string marked_cnt = GetStrInRow(table, row, "#marked_cnt");
        return latexify(marked_cnt, true) + " [" + latexify(spart, false) + "]";
    });
    CreateNewColumn(table, "custom_Sx_part_nice", [&](vector<string> row) {
        string spart = GetStrInRow(table, row, "custom_Sx_part");
        string marked_cnt = GetStrInRow(table, row, "#marked_reduc_cnt");
        return latexify(marked_cnt, true) + " [" + latexify(spart, false) + "]";
    });

    CreateNewColumn(table, "Tker", [&](vector<string> row) {
        string A = GetStrInRow(table, row, "#marked_reduc_time");
        string B = GetStrInRow(table, row, "twoway_time");
        return to_string_with_precision(stod(A) + stod(B), 2);
    });

    vector<bool> remove_decimals = { false,       true,          true,      false,            false,               false,          false,    true};
    vector<string> interesting = {"GRAPH_NAME", "#num_nodes", "#num_edges", "GRAPH_DENSITY", "custom_Sx_part_nice", "VEFFICIENCY", "Tker",  "#mcpost"};

    table = GetColumnSubsetTable(table, interesting);

    PrintTable(table);
    cout << endl << endl;

    for (int r = 0; r < table.second.size(); ++r) {
        vector<string> row = table.second[r];

        for (int i = 0; i + 1 < row.size(); ++i)
            cout << latexify(row[i], remove_decimals[i]) << " & ";
        cout << latexify(row.back(), remove_decimals.back()) << " \\\\ \\hline" << endl;
    }
}

int main() {
    ifstream in("experiments/linear-kernel/real-world-small/out");
    Table table = ReadTable(in);
    
    PrintTable(table);
    cout << endl << endl << endl << endl;

    HandleLinearKernel(table);
}