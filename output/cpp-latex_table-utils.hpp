#pragma once

#include <bits/stdc++.h>
using namespace std;

typedef vector<vector<string>> TableData;
typedef vector<string> Captions;
typedef pair<Captions, TableData> Table;

const int GetRowCount(const Table& table);
const int GetColumnIndex(const Table& table, const string colname);
vector<string> ReadTableLine(ifstream &in, bool allow_empty = true);
Table ReadTable(ifstream &in);
Table Aggregate(const Table table, const string colname);

vector<string> GetColumnVals(Table table, int coldx);
vector<string> GetColumnVals(Table table, string colname);
Table GetSubset(const Table source, const string colname, const string colval);

void PrintTable(Table table);

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

double GetAverage(const vector<double> val) {
    double res = 0;
    for (auto e : val) res += e;
    return res / (val.size());
}

double GetValInRow(Table table, vector<string> row, string colname) {
    int coldx = GetColumnIndex(table, colname);
    return stod(row[coldx]);
}
string GetStrInRow(Table table, vector<string> row, string colname) {
    int coldx = GetColumnIndex(table, colname);
    return row[coldx];
}

vector<double> ToDVec(const vector<string> val) {
    vector<double> dval;
    for (int i = 0; i < (int)val.size(); ++i)
        dval.push_back(stod(val[i]));
    return dval;
}

double GetStandardDeviation(const vector<double>& val) {
    double mu = GetAverage(val);
    double ret = 0;
    for (auto e : val)
        ret += ((e - mu) * (e - mu)) / val.size();;

    return sqrt(ret);
}

string GetColType(const vector<string> vals) {
    int double_err = 0;
    for (int i = 0; i < (int)vals.size(); ++i) {
        string val = vals[i];
        size_t pos;
        try {
            stod(val, &pos);
        }
        catch(std::invalid_argument& e){
            double_err++;
            continue;
        }

        if (pos != val.size()) double_err++;
    }

    if (double_err > 0) {
        for (int i = 1; i < (int)vals.size(); ++i)
            if (vals[i] != vals[0])
                throw std::logic_error("Tried to aggregate string column with different vlaues.");

        return "STRING";
    }

    return "DOUBLE";
}

const int GetRowCount(const Table& table) {
    return table.second.size();
}
const int GetColumnIndex(const Table& table, const string colname) {
    for (int i = 0; i < (int)table.first.size(); ++i)
        if (table.first[i] == colname)
            return i;
    
    throw std::logic_error("Requested column-name does not exist.");
    return -1;
}

vector<string> ReadTableLine(ifstream &in, bool allow_empty) {
    string strline;
    getline(in, strline);

    stringstream strin(strline);
    vector<string> ret;
    while (strin.eof() == false) {
        string elem;
        strin >> elem;
        if (elem.size() == 0 && !allow_empty) continue;
        ret.push_back(elem);
    }

    return ret;
}

Table ReadTable(ifstream &in) {
    Captions captions = ReadTableLine(in, false);
    TableData table_data;
    while(in.eof() == false) {
        table_data.push_back(ReadTableLine(in));

        while(table_data.back().size() > captions.size()) table_data.back().pop_back();

        if (table_data.back().size() != captions.size()) table_data.pop_back();
    }

    return make_pair(captions, table_data);
}

Table Aggregate(const Table table, const string colname) {
    vector<string> colvals = GetColumnVals(table, colname);
    sort(colvals.begin(), colvals.end(), [&](string a, string b) { return stoi(a) < stoi(b);});
    colvals.erase(unique(colvals.begin(), colvals.end()), colvals.end());

    vector<bool> has_sd(table.first.size(), false);
    vector<bool> is_double(table.first.size(), false);

    TableData resdata;
    for (int i = 0; i < (int)colvals.size(); ++i) {
        Table sub = GetSubset(table, colname, colvals[i]);

        vector<string> resrow;
        for (int c = 0; c < (int)sub.first.size(); ++c) {
            vector<string> vals = GetColumnVals(sub, c);

            if (GetColType(vals) == "STRING") {
                resrow.push_back(vals[0]);
            } else {
                vector<double> dvals = ToDVec(vals);
                resrow.push_back(to_string_with_precision(GetAverage(dvals), 2) + " (" + to_string_with_precision(GetStandardDeviation(dvals), 2) + ")");

                if (fabs(GetStandardDeviation(dvals)) > 1e-3) {
                    has_sd[c] = true;
                }
                is_double[c] = true;
            }
        }

        resdata.push_back(resrow);
    }

    Table restable = make_pair(table.first, resdata);

    for (int c = 0; c < (int)restable.first.size(); ++c) {
        vector<string> vals = GetColumnVals(restable, c);

        if (has_sd.at(c)) continue;
        if (!is_double.at(c)) continue;

        for (int r = 0; r < restable.second.size(); ++r) {
            string entry = restable.second[r][c];
            restable.second[r][c] = entry.substr(0, entry.size() - string(" (0.00)").size());
        }
    }

    return restable;
}


Table GetSubset(const Table source, const string colname, const string colval) {
    const int coldx = GetColumnIndex(source, colname);
    const int rowcnt = GetRowCount(source);
    TableData retdata;
    for (int i = 0; i < rowcnt; ++i) {
        if (source.second[i][coldx] == colval)
            retdata.push_back(source.second[i]);
    }

    return make_pair(source.first, retdata);
}

vector<string> GetColumnVals(Table table, int coldx) {
    vector<string> ret;
    const int rowcnt = GetRowCount(table);
    for (int i = 0; i < rowcnt; ++i) {
        ret.push_back(table.second[i][coldx]);
    }

    return ret;
}

vector<string> GetColumnVals(Table table, string colname) {
    int coldx = GetColumnIndex(table, colname);
    return GetColumnVals(table, coldx);;
}

void PrintTable(Table table) {
    vector<int> colsz;
    for (int c = 0; c < table.first.size(); ++c) colsz.push_back(table.first[c].size());

    for (int c = 0; c < table.first.size(); ++c) {
        vector<string> vals = GetColumnVals(table, c);
        for (int i = 0; i < vals.size(); ++i) {
            colsz[c] = max((int)colsz[c], (int)vals[i].size());
        }
    }
    

    for (int c = 0; c < table.first.size(); ++c) cout << setw(colsz[c] + 3) << table.first[c];
    cout << endl;

    int rowcnt = GetRowCount(table);
    for (int r = 0; r < rowcnt; ++r) {
        for (int c = 0; c < table.first.size(); ++c) cout << setw(colsz[c] + 3) << table.second[r][c];
        cout << endl;
    }
}

Table GetColumnSubsetTable(Table source, vector<string> cols) {
    Captions captions;
    for (int i = 0; i < cols.size(); ++i) captions.push_back(source.first.at(GetColumnIndex(source, cols[i])));

    TableData rotated;
    for (int i = 0; i < cols.size(); ++i) {
        rotated.push_back(GetColumnVals(source, cols[i]));
    }

    if (rotated.size() == 0) throw std::logic_error("You do not want this, yo!");

    TableData actual;
    for (int r = 0; r < rotated[0].size(); ++r) {
        vector<string> row;
        for (int c = 0; c < rotated.size(); ++c)
            row.push_back(rotated[c][r]);
        actual.push_back(row);
    }

    return make_pair(captions, actual);
}

void CreateNewColumn(Table& table, string type, std::function<string(vector<string> row)> build_func = {}) {
    table.first.push_back(type);
    if (type == "GRAPH_DENSITY") {
        vector<string> valA = GetColumnVals(table, "#num_nodes");
        vector<string> valB = GetColumnVals(table, "#num_edges");

        vector<double> dvalA = ToDVec(valA);
        vector<double> dvalB = ToDVec(valB);

        if (dvalA.size() != dvalB.size()) throw std::logic_error("Columns can't have different sizes!");

        for (int i = 0; i < dvalA.size(); ++i) {
            table.second[i].push_back(to_string(dvalB[i] / dvalA[i]));
        }
    } else if (type == "GRAPH_NAME") {
        vector<string> vals = GetColumnVals(table, "#file");
        
        for (int i = 0; i < vals.size(); ++i) {
            string entry = vals[i];
            int lastslash = 0;
            int lastperiod = entry.size();

            for (int dx = 0; dx < entry.size(); ++dx)
                if(entry[dx] == '/' || entry[dx] == '\\')
                    lastslash = dx + 1;
            
            for (int dx = lastslash; dx < entry.size(); ++dx)
                if (entry[dx] == '.')
                    lastperiod = dx;
            
            string res = entry.substr(lastslash, lastperiod - lastslash);
            table.second[i].push_back(res);
        }
    } else {
        for (int i = 0; i < table.second.size(); ++i) {
            table.second[i].push_back(build_func(table.second[i]));
        }
    }
}