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
#define OutputDebugLogX(str) OutputDebugLogX_(str)

vector<int> SetSubstract(const vector<int>& source, const vector<int>& target);

void OutputDebugLog_(const string file, const int line_num, const string str, const int flags = 0);
void OutputDebugLogX_(const string str);