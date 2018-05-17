#include "./utils.hpp"

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

bool IsASubsetOfB(const vector<int>& A, const vector<int>& B) {
    return SetSubstract(B, A).size() == (B.size() - A.size());
}