#pragma once

#include "../input-parser.hpp"

class BenchmarkAction {
public:
    virtual void Evaluate(InputParser& input, const string data_filepath) = 0;

    vector<int> tot_used_rules;
};