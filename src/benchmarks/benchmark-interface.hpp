#pragma once

#include "../input-parser.hpp"

class BenchmarkAction {
public:
    BenchmarkAction() {
        tot_used_rules.resize(20);
    }
    
    virtual void Evaluate(InputParser& input, const string data_filepath) = 0;

    vector<int> tot_used_rules;
};