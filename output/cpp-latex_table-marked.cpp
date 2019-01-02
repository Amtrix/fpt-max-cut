#include <bits/stdc++.h>
#include "./cpp-latex_table-utils.hpp"
using namespace std;


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

    vector<bool> remove_decimals = { false,          true,      false,          false,             false,            false,               false };
    vector<string> interesting = {"GRAPH_NAME","#num_edges", "GRAPH_DENSITY", "custom_S_part_nice", "#marked_time", "custom_Sx_part_nice", "#marked_reduc_time"};

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

    vector<bool> remove_decimals = { false,       true,          true,      false,            false,               false,          false,    false};
    vector<string> interesting = {"GRAPH_NAME", "#num_nodes", "#num_edges", "GRAPH_DENSITY", "custom_Sx_part_nice", "VEFFICIENCY", "Tker",  "mcpost_time"};

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