#pragma once

#include <bits/stdc++.h>
using namespace std;

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

template <class T>
void OutputDebugVector_(const string file, const int line_num, const string name, const vector<T> vec) {
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

vector<int> SetSubstract(const vector<int>& source, const vector<int>& target);
vector<int> SetUnion(const vector<int>& s1, const vector<int>& s2);
bool IsASubsetOfB(const vector<int>& A, const vector<int>& B);