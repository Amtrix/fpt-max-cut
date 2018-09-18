#pragma once

#include "../input-parser.hpp"

class BenchmarkAction {
public:
    BenchmarkAction() {
        tot_used_rules.resize(20);
    }

    // same for same prefix in prefix.x
    string GetKey(const string data_filepath) {
        string key = "";
        int dx = ((int)data_filepath.size()) - 1;
        while (data_filepath[dx] != '/' && data_filepath[dx] != '\\')
            key += data_filepath[dx--];
        key = key.substr(key.find('.'));
        return key;
    }

    virtual void PostProcess(InputParser& /* input */) {

    }
    
    virtual void Evaluate(InputParser& /*input*/, const string data_filepath) {
        string key = GetKey(data_filepath);    
        if (mixingid[key] == 0) mixingid[key] = mixingid_giver++;
    }

    vector<int> tot_used_rules;

protected:
    int mixingid_giver = 1;
    map<string,int> mixingid;
};