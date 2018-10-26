#pragma once

#include <experimental/filesystem>
#include <unordered_map>
#include <iostream>

using namespace std;
using namespace std::experimental;

#ifndef __FUNCTION_NAME__
    #ifdef WIN32   //WINDOWS
        #define __FUNCTION_NAME__   __FUNCTION__  
    #else          //*NIX
        #define __FUNCTION_NAME__   __func__ 
    #endif
#endif

const int NO_LINE_BREAK = 1;

#define OutputDebugLogNoNewLine(str) OutputDebugLog_(__FILE__,__LINE__,str,NO_LINE_BREAK)
#define OutputDebugLog(str) OutputDebugLog_(__FILE__,__LINE__,str,0)
#define OutputDebugLogRaw(str) OutputDebugLogX_(str)
#define OutputDebugVector(name,vec) OutputDebugVector_(__FILE__,__LINE__,name,vec)

void OutputDebugLog_(const string file, const int line_num, const string str, const int flags = 0);
void OutputDebugLogX_(const string str);


void OutputDebugVector_(const string file, const int line_num, const string name, const vector<int> vec);
void OutputDebugVector_(const string file, const int line_num, const string name, const vector<pair<int,int>> vec);

vector<int> SetSubstract(const vector<int>& source, const vector<int>& target);
vector<int> SetUnion(const vector<int>& s1, const vector<int>& s2);
vector<int> SetIntersection(const vector<int>& s1, const vector<int>& s2);
bool IsASubsetOfB(const vector<int>& A, const vector<int>& B);

vector<pair<int, string>> ZipVec(const vector<int> A, const vector<string> B);
vector<int> VectorsAdd(const vector<int> A, const vector<int> B, bool cut_at_smaller = false);

// ignores README.md !
vector<string> GetAllDatasets(const string path);

vector<string> ReadLine(std::istream& in);

inline bool KeyExists(int key, const unordered_map<int, bool> &m) {
    return m.find(key) != m.end();
}


// BEWARE: KeyExistsAndEquals(<key,bool>, key, false) returns false when key does not exist. Same for KeyExistsAndEquals(<key,int>, key, 0). You cannot rely on default type!
// See MapEqualCheck for alternative.
template <class KeyType, class ValType>
inline bool KeyExistsAndEquals(const unordered_map<KeyType, ValType> &m, const KeyType key, const ValType value) {
    return m.find(key) != m.cend() && m.at(key) == value;
}

template <class KeyType, class ValType>
inline bool MapEqualCheck(const unordered_map<KeyType, ValType> &m, const KeyType key, const ValType value) {
    return m.find(key) != m.cend() ? m.at(key) == value : value == ValType();
}