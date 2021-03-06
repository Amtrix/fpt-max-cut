#pragma once

#include <experimental/filesystem>
#include <unordered_map>
#include <iostream>
#include <cmath>
#include <cassert>
#include <map>
#include <functional>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace std;
using namespace std::experimental;

#ifndef __FUNCTION_NAME__
    #ifdef WIN32   //WINDOWS
        #define __FUNCTION_NAME__   __FUNCTION__  
    #else          //*NIX
        #define __FUNCTION_NAME__   __func__ 
    #endif
#endif

#ifdef DEBUG
    #define OutputDebugLogNoNewLine(str) OutputDebugLog_(__FILE__,__LINE__,str,NO_LINE_BREAK)
    #define OutputDebugLog(str) OutputDebugLog_(__FILE__,__LINE__,str,0)
    #define OutputDebugLogRaw(str) OutputDebugLogX_(str)
    #define OutputDebugVector(name,vec) OutputDebugVector_(__FILE__,__LINE__,name,vec)
    #define custom_assert(val) CustomAssertImpl_(__FILE__,__LINE__,val)
    #define custom_assert(val) CustomAssertImpl_(__FILE__,__LINE__,val)
    #define custom_assert_with_msg(val,msg) CustomAssertImpl_(__FILE__,__LINE__,val,msg)
#else
    #define custom_assert(val) ((void)0)
    #define custom_assert_with_msg(val,msg) ((void)0)
    #define OutputDebugLogNoNewLine(str) ((void)0)
    #define OutputDebugLog(str) ((void)0)
    #define OutputDebugLogRaw(str) ((void)0)
    #define OutputDebugVector(name,vec) ((void)0)
#endif


void CustomAssertImpl_(const string file, const int line_num, bool res, const string msg = "");

const int NO_LINE_BREAK = 1;

void OutputDebugLog_(const string file, const int line_num, const string str, const int flags = 0);
void OutputDebugLogX_(const string str);
void OutputDebugVector_(const string file, const int line_num, const string name, const vector<int> vec);
void OutputDebugVector_(const string file, const int line_num, const string name, const vector<pair<int,int>> vec);

vector<int> SetSubstract(const vector<int>& source, const vector<int>& target);
vector<int> SetUnion(const vector<int>& s1, const vector<int>& s2);
vector<int> SetIntersection(const vector<int>& s1, const vector<int>& s2);
bool IsASubsetOfB(const vector<int>& A, const vector<int>& B);
bool SameSets(const vector<int>& A, const vector<int>& B);

vector<pair<int, string>> ZipVec(const vector<int> A, const vector<string> B);
vector<int> VectorsAdd(const vector<int> A, const vector<int> B, bool cut_at_smaller = false);

//warning! if the input is directed, it will remove one direction.
vector<pair<unsigned long long,unsigned long long>> RemoveAnyMultipleEdgesAndSelfLoops(const vector<pair<unsigned long long,unsigned long long>>& elems);

std::function<long long(void)> TakeFirstFromPairFunction(std::function<pair<long long,vector<int>>(void)> func, long long addval = 0);
tuple<double, double, double, double, long long> ComputeAverageAndDeviation(vector<double> Ares, vector<double> Bres);
tuple<double, double, double, double, long long> ComputeAverageAndDeviation(std::function<long long(void)> Agen, std::function<long long(void)> Bgen, int iter = 1);

// ignores README.md !
vector<string> GetAllDatasets(const string path);

const vector<string> ReadLine(std::istream& in, string *read_line = nullptr);

template <class Type>
string SerializeVector(vector<Type> vec) {
    string ret = "";
    for (int i = 0; i < (int)vec.size(); ++i)
        ret += to_string(vec[i]) + (i + 1 < (int)vec.size() ? ", " : "");
    return ret;
}

template <class KeyType, class ValType>
inline bool KeyExists(KeyType key, const unordered_map<KeyType, ValType> &m) {
    return m.find(key) != m.end();
}
template <class KeyType, class ValType>
inline bool KeyExists(KeyType key, const map<KeyType, ValType> &m) {
    return m.find(key) != m.end();
}

template <class Type>
double GetAverage(const vector<Type>& val) {
    double res = 0;
    for (auto e : val) res += e;
    return res / (val.size());
}

template <class Type>
vector<Type> SubVectorVal(const vector<Type>& valA, const vector<Type>& valB) {
    assert(valA.size() == valB.size());
    vector<Type> ret;
    for (int i = 0; i < (int)valA.size(); ++i)
        ret.push_back(valA[i] - valB[i]);
    return ret;
}

template <class Type>
double GetStandardDeviation(const vector<Type>& val) {
    double mu = GetAverage(val);
    double ret = 0;
    for (auto e : val)
        ret += ((e - mu) * (e - mu)) / val.size();;

    return sqrt(ret);
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

int GetANonZeroWeight(int edge_weight_lo, int edge_weight_hi);



// Returns output and execution time in milliseconds.
tuple<std::string, double> exec_custom(const string binfile, const string rfilepath, int timelimit_seconds = 5);