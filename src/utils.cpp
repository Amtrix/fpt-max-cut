#include "./utils.hpp"
#include <experimental/filesystem>
using namespace std;
using namespace std::experimental;

void OutputDebugLog_(const string file, const int line_num, const string str, const int flags) {
    (void)str;
    (void)file;
    (void)line_num;
    (void)flags;

#ifdef DEBUG
    string rfile = "";
    if (file != "") {
        for (unsigned int i = 0; i < file.size(); ++i)
            if (file[i] == '/' || file[i] == '\\') rfile = "";
            else rfile += file[i];
    }
    cout << "LOG: " << (rfile.size() > 0 ? rfile + ":" + to_string(line_num) + ":   " : "") << str << (flags & 1 ? "" : "\n");
#endif
}

void OutputDebugLogX_(const string str) {
    (void)str;
#ifdef DEBUG
    cout << str;
#endif
}

void OutputDebugVector_(const string file, const int line_num, const string name, const vector<int> vec) {
    (void)file;
    (void)line_num;
    (void)name;
    (void)vec;

#ifdef DEBUG
    OutputDebugLog_(file, line_num, name + ": ", NO_LINE_BREAK);
    for (unsigned int i = 0; i < vec.size(); ++i)
        OutputDebugLogRaw(to_string(vec[i]) + " ");
    OutputDebugLogRaw("\n");
#endif
}

void OutputDebugVector_(const string file, const int line_num, const string name, const vector<pair<int,int>> vec) {
    (void)file;
    (void)line_num;
    (void)name;
    (void)vec;

#ifdef DEBUG
    OutputDebugLog_(file, line_num, name + ": ", NO_LINE_BREAK);
    for (unsigned int i = 0; i < vec.size(); ++i)
        OutputDebugLogRaw("(" + to_string(vec[i].first) + "," + to_string(vec[i].second) + ") ");
    OutputDebugLogRaw("\n");
#endif
}


// source - target
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

vector<string> GetAllDatasets(const string path) {
    vector<string> ret;
    for (auto & p : filesystem::directory_iterator(path))
        if (filesystem::is_regular_file(p)) {
            string fname = filesystem::path(p.path().u8string()).filename();
            if (fname == "README.md") continue;
            ret.push_back(p.path().u8string());
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

    return ret;
}