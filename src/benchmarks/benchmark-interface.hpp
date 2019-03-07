#pragma once

#include "../input-parser.hpp"

class BenchmarkAction {
public:
    BenchmarkAction() {
        tot_used_rules.resize(20);
    }

    inline void LogTimeEx(unordered_map<int, double>& times_all, auto &tdiff, int id = -1) const {
        times_all[id] += tdiff;
    }

    inline void LogTime(unordered_map<int, double>& times_all, std::chrono::high_resolution_clock::time_point &t0, int id = -1) const {
        auto t1 = std::chrono::high_resolution_clock::now();
        double tdiff = std::chrono::duration_cast<std::chrono::microseconds> (t1 - t0).count()/1000.;
        times_all[id] += tdiff;
        t0 = t1;
    }

    inline std::chrono::high_resolution_clock::time_point GetCurrentTime() {
        return std::chrono::high_resolution_clock::now();
    }

    // same for same prefix in prefix.x
    string GetKey(const string data_filepath) const {
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
    
    virtual void Evaluate(InputParser& /*input*/, const MaxCutGraph& /*main_graph*/) {
    }

    void GenerateMissingMixingId(const MaxCutGraph& main_graph) {
        string key = GetKey(main_graph.GetGraphNaming());
        cout << " =====> " << key << endl;
        if (mixingid[key] == 0) mixingid[key] = mixingid_giver++;
    }

    int GetMixingId(const MaxCutGraph &G) const {
        if (G.GetMixingId() != -1) return G.GetMixingId();
        cout << "+> " << GetKey(G.GetGraphNaming()) << endl;
        return mixingid.at(GetKey(G.GetGraphNaming()));
    }

    vector<int> tot_used_rules;

protected:
    int mixingid_giver = 1;
    map<string,int> mixingid;
};