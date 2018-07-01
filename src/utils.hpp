#pragma once

#include <bits/stdc++.h>
#include <experimental/filesystem>
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
vector<int> VectorsAdd(const vector<int> A, const vector<int> B);

// ignores README.md !
vector<string> GetAllDatasets(const string path);