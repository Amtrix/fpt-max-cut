#include <string>

#include "graph-database.hpp"
#include "mc-graph.hpp"
#include "input-parser.hpp"
using namespace std;


GraphDatabase::GraphDatabase(InputParser& input) {
    if (input.cmdOptionExists("-sample-kagen")) {
        graph_generation_mode = GraphGenerationMode::KagenSample;
        return;
    }

    if (input.cmdOptionExists("-f")) {
        const string data_filepath = input.getCmdOption("-f");
        all_sets_to_evaluate.push_back(data_filepath);
    } else {
        // get all datasets from data/
        for (unsigned int i = 0; i < kDataSetCount; ++i) {
            auto sets = GetAllDatasets(paths[i]);
            for (unsigned int i = 0; i < sets.size(); ++i)
                all_sets_to_evaluate.push_back(sets[i]);
        }
    }
}

MaxCutGraph GraphDatabase::GetGraph(const string& key) const {
    return MaxCutGraph(key);
}

MaxCutGraph GraphDatabase::GetGraphById(const long id) const {
    return MaxCutGraph(all_sets_to_evaluate[id]);
}