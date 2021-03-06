#pragma once

#include <bits/stdc++.h>

#define VERIFY_RETURN_ON_FAIL(x,y) if(!COMPARE_(x,y,__LINE__,__FILE__))return;
#define VERIFY(x,y) COMPARE_(x,y,__LINE__,__FILE__)

bool HAS_FAILED_ONCE = false;


bool COMPARE_(int x, int y, int line_number, const char* file) {
    if (x != y) {
        cout << "FAIL @ " << file << ":" << line_number << endl;
        cout << "       First value: " << x << ", second: " << y << endl;
        HAS_FAILED_ONCE = true;
        return false;
    }
    return true;
}

bool COMPARE_(vector<int> x, vector<int> y, int line_number, const char* file) {
    if (x.size() != y.size()) {
        cout << "FAIL @ " << file << ":" << line_number << endl;
        cout << "       Different size. First: " << x.size() << ", second: " << y.size() << endl;
        HAS_FAILED_ONCE = true;
        return false;
    }

    if (x != y) {
        cout << "FAIL @ " << file << ":" << line_number << endl;
        cout << "       Vectors contain different values." << endl;
        HAS_FAILED_ONCE = true;
        return false;
    }
    return true;
}

void InitializeTest() {
    HAS_FAILED_ONCE = false;
}