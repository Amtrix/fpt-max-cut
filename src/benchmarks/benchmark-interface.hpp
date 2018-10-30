#pragma once

#include "../input-parser.hpp"

class BenchmarkAction {
public:
    BenchmarkAction() {
        tot_used_rules.resize(20);
    }

    inline void LogTime(vector<pair<double,int>> &times, auto &t0, int id = -1) {
        auto t1 = std::chrono::high_resolution_clock::now();
        times.push_back(make_pair(std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000., id));
        t0 = t1;
    }

    inline void FlushTimes(vector<pair<double,int>>& times, /*vector<double>& atimes,*/ const bool print = true) {
        if (print) {
            cout << "Times from previous run: ";
            double cumm = 0;
            for (unsigned int i = 0; i < times.size(); ++i) {
                cout << times[i].first << " ";
                cumm += times[i].first;
            }
            cout << " = " << cumm << endl;
        }
        for (auto elem : times)
            times_all[elem.second] += elem.first;
        times.clear();
    }

    // same for same prefix in prefix.x
    string GetKey(const string data_filepath) {
        string key = "";
        int dx = ((int)data_filepath.size()) - 1;
        while (dx >= 0 && data_filepath[dx] != '/' && data_filepath[dx] != '\\')
            key += data_filepath[dx--];
        if (key.find('.') == string::npos)
            return key;
        
        key = key.substr(key.find('.'));
        return key;
    }

    virtual void PostProcess(InputParser& /* input */) {

    }
    
    virtual void Evaluate(InputParser& /*input*/, const MaxCutGraph& main_graph) {
        string key = GetKey(main_graph.GetGraphNaming());    
        if (mixingid[key] == 0) mixingid[key] = mixingid_giver++;
    }

    int GetMixingId(const MaxCutGraph &G) const {
        if (G.GetMixingId() != -1) return G.GetMixingId();
        return mixingid.at(G.GetGraphNaming());
    }

    vector<int> tot_used_rules;

protected:
    int mixingid_giver = 1;
    map<string,int> mixingid;
    unordered_map<int, double> times_all;
};