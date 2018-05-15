#pragma once

#include <bits/stdc++.h>

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