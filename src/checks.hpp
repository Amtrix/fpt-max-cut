#pragma once

#include <bits/stdc++.h>

#define VERIFY_RETURN_ON_FAIL(x,y) if(!COMPARE_(x,y,__LINE__,__FILE__))return;
#define VERIFY(x,y) COMPARE_(x,y,__LINE__,__FILE__)


bool COMPARE_(int x, int y, int line_number, const char* file) {
    if (x != y) {
        cout << "FAIL @ " << file << ":" << line_number << endl;
        cout << "       First value: " << x << ", second: " << y << endl;
        return false;
    }
    return true;
}