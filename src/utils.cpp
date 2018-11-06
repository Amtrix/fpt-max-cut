#include "./utils.hpp"
#include "./colormod.hpp"
#include <sstream>
#include <string>
#include <map>
#include <experimental/filesystem>
using namespace std;
using namespace std::experimental;

void CustomAssertImpl_(const string file, const int line_num, bool res) {
    if (!res) {
        cout << file << ":" << line_num << ": failed assertion." << endl;
        exit(0);
    }
}

void OutputDebugLog_(const string file, const int line_num, const string str, const int flags) {
    string rfile = "";
    if (file != "") {
        for (unsigned int i = 0; i < file.size(); ++i)
            if (file[i] == '/' || file[i] == '\\') rfile = "";
            else rfile += file[i];
    }
    cout << "LOG: " << (rfile.size() > 0 ? rfile + ":" + to_string(line_num) + ":   " : "") << str << (flags & 1 ? "" : "\n");
}

void OutputDebugLogX_(const string str) {
    cout << str;
}

void OutputDebugVector_(const string file, const int line_num, const string name, const vector<int> vec) {
    OutputDebugLog_(file, line_num, name + ": ", NO_LINE_BREAK);
    for (unsigned int i = 0; i < vec.size(); ++i)
        OutputDebugLogRaw(to_string(vec[i]) + " ");
    OutputDebugLogRaw("\n");
}

void OutputDebugVector_(const string file, const int line_num, const string name, const vector<pair<int,int>> vec) {
    OutputDebugLog_(file, line_num, name + ": ", NO_LINE_BREAK);
    for (unsigned int i = 0; i < vec.size(); ++i)
        OutputDebugLogRaw("(" + to_string(vec[i].first) + "," + to_string(vec[i].second) + ") ");
    OutputDebugLogRaw("\n");
}


// source - target
// order in source preserved
vector<int> SetSubstract(const vector<int>& source, const vector<int>& target) {
    vector<int> ret;

    unordered_map<int,int> exist;
    for (unsigned int i = 0; i < target.size(); ++i)
        exist[target[i]] = true;
    
    for (unsigned int i = 0; i < source.size(); ++i)
        if (!exist[source[i]])
            ret.push_back(source[i]);

    return ret;
}

vector<int> SetUnion(const vector<int>& s1, const vector<int>& s2) {
    unordered_map<int,int> exist;
    for (auto e : s1) exist[e] = true;
    for (auto e : s2) exist[e] = true;

    vector<int> ret;
    for (auto e : s1) if (exist[e]) { ret.push_back(e); exist[e] = false; }
    for (auto e : s2) if (exist[e]) { ret.push_back(e); exist[e] = false; }
    return ret;
}

vector<int> SetIntersection(const vector<int>& s1, const vector<int>& s2) {
    unordered_map<int,int> exist;
    for (auto e : s1) exist[e]++;
    for (auto e : s2) exist[e]++;

    vector<int> ret;
    for (auto e : s1) if (exist[e] == 2) { ret.push_back(e); exist[e] = 0; }
    for (auto e : s2) if (exist[e] == 2) { ret.push_back(e); exist[e] = 0; }
    return ret;
}

bool IsASubsetOfB(const vector<int>& A, const vector<int>& B) {
    return SetSubstract(B, A).size() == (B.size() - A.size());
}

vector<int> VectorsAdd(const vector<int> A, const vector<int> B, bool cut_at_smaller) {
    if (!cut_at_smaller && A.size() != B.size())
        throw std::logic_error("Can't add vectors of different sizes!");
    
    int sz = min(A.size(), B.size());
    vector<int> ret;
    for (int i = 0; i < sz; ++i)
        ret.push_back(A[i] + B[i]);
    return ret;
}

vector<pair<unsigned long long,unsigned long long>> RemoveAnyMultipleEdgesAndSelfLoops(const vector<pair<unsigned long long,unsigned long long>>& elems) {
    vector<pair<unsigned long long,unsigned long long>> ret;
    map<pair<unsigned long long,unsigned long long>, bool> visi;

    for (auto e : elems) {
        if (visi[e] || e.first == e.second) continue;
        ret.push_back(e);
        visi[e] = true;
        visi[make_pair(e.second, e.first)] = true;
    }
    return ret;
}

std::function<int(void)> TakeFirstFromPairFunction(std::function<pair<int,vector<int>>(void)> func, int addval) {
    return [func,addval]() {
        return func().first  + addval;
    };
}

tuple<double, double, double, int> ComputeAverageAndDeviation(std::function<int(void)> Agen, std::function<int(void)> Bgen, int iter) {
    vector<double> Ares, Bres;
    double mxval = 0;
    while (iter--) {
        Ares.push_back(Agen());
        Bres.push_back(Bgen());
        mxval = max(mxval, Ares.back());
        mxval = max(mxval, Bres.back());
    }

    double Aavg = GetAverage(Ares);
    double Bavg = GetAverage(Bres);
    double SDres = GetStandardDeviation(SubVectorVal(Ares, Bres));
    return make_tuple(Aavg, Bavg, SDres, mxval);
}

vector<string> GetAllDatasets(const string path) {
    filesystem::path cpppath = path.c_str();

    vector<string> ret;
    if (filesystem::is_directory(cpppath)) {
        for (auto & p : filesystem::directory_iterator(path))
            if (filesystem::is_regular_file(p)) {
                string fname = filesystem::path(p.path().u8string()).filename();
                if (fname == "README.md") continue;
                ret.push_back(p.path().u8string());
            }
    } else {
        ret.push_back(cpppath.u8string());
    }
    return ret;
}

vector<string> ReadLine(std::istream& in) {
    vector<string> ret;

    string w;
    getline(in, w);

    stringstream line_in(w);
    while (line_in.eof() == false) {
        string val; line_in >> val;
        ret.push_back(val);
    }

    while (ret.empty() == false && ret.back() == "") ret.pop_back();
    return ret;
}