#include <bits/stdc++.h>
#include "./cpp-latex_table-utils.hpp"
using namespace std;




const string outpath = "/solvers/real-world-small/out";

Table table;

int main(int argc, char **argv){
    if (argc < 2) {
        throw std::logic_error("Path to experiment required.");
        exit(-1);
    }

    const string root_dir(argv[1]);

    ifstream in((root_dir + outpath).c_str());
    if (!in.is_open()) {
        cout << "Couldn't open: " << outpath << endl;
        exit(-1);
    }

    table = ReadTable(in);
    RenameColumn(table, "#|V(G)|", "#num_nodes");
    RenameColumn(table, "#|E(G)|", "#num_edges");
    RenameColumn(table, "#|V(Gk)|", "#num_nodes_k");
    RenameColumn(table, "#|E(Gk)|", "#num_edges_k");
    PrintTable(table);
    table = Aggregate(table, "#sec");
    

    CreateNewColumn(table, "GRAPH_DENSITY");
    CreateNewColumn(table, "VEFFICIENCY");
    CreateNewColumn(table, "GRAPH_NAME");
    table = SortMethodA(table);

    CreateNewColumn(table, "speedup_localsolver", [&](vector<string> row) { 
        double loct = GetValInRow(table, row, "#LOCSOLVER_T(G)");
        if (loct < 0) return string("-");
        return to_string_with_precision(loct/1000.0, 2);
    });
    CreateNewColumn(table, "speedup_localsolver_k", [&](vector<string> row) {
            
        double loct = GetValInRow(table, row, "#LOCSOLVER_T(G)");
        double loct_k = GetValInRow(table, row, "#LOCSOLVER_T(Gk)");
        
        
        if (loct < 0 && loct_k < 0) return string("-");

        string scnd = " [" + to_string_with_precision(loct/loct_k, 2) + "]";
        if (loct < 0 && loct_k >= 0) scnd = " [$" + string("\\infty") + "$]";
        
        return to_string_with_precision(loct_k/1000.0, 2) + scnd;
    });
    CreateNewColumn(table, "biqmac_localsolver", [&](vector<string> row) { 
        double loct = GetValInRow(table, row, "#BIQMAC_T(G)");
        if (loct < 0) return string("-");
        return to_string_with_precision(loct/1000.0, 2);
    });
    CreateNewColumn(table, "biqmac_localsolver_k", [&](vector<string> row) {
            
        double loct = GetValInRow(table, row, "#BIQMAC_T(G)");
        double loct_k = GetValInRow(table, row, "#BIQMAC_T(Gk)");
        
        
        if (loct < 0 && loct_k < 0) return string("-");

        string scnd = " [" + to_string_with_precision(loct/loct_k, 2) + "]";
        if (loct < 0 && loct_k >= 0) scnd = " [$" + string("\\infty") + "$]";
        
        return to_string_with_precision(loct_k/1000.0, 2) + scnd;
    });

    vector<bool> remove_decimals = { false,       false,          true,         false      ,  false,                false,                false,             false};
    vector<string> interesting = {"GRAPH_NAME", "GRAPH_DENSITY",  "#num_nodes", "VEFFICIENCY", "speedup_localsolver", "speedup_localsolver_k", "biqmac_localsolver", "biqmac_localsolver_k"};

    table = GetColumnSubsetTable(table, interesting);

    PrintTable(table);

    for (int r = 0; r < table.second.size(); ++r) {
        vector<string> row = table.second[r];

        for (int i = 0; i + 1 < row.size(); ++i)
            cout << latexify(row[i], remove_decimals[i]) << " & ";
        cout << latexify(row.back(), remove_decimals.back()) << " \\\\ \\hline" << endl;
    }
}