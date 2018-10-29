#include <string>

#include "graph-database.hpp"
#include "mc-graph.hpp"
#include "input-parser.hpp"
using namespace std;


GraphDatabase::GraphDatabase(InputParser& input) {
    if (input.cmdOptionExists("-sample-kagen")) {
        cout << "Sample kagen mode activated" << endl;
        graphs_per_type = stoi(input.getCmdOption("-sample-kagen"));
        graph_generation_mode = GraphGenerationMode::KagenSampling;

        const auto is_smaller = [](const KagenGraphCollectionDescriptor& x1, const KagenGraphCollectionDescriptor& x2) {
            int p1 = distance(kKagenTypeListing.begin(), find(kKagenTypeListing.begin(), kKagenTypeListing.end(), x1.graph_type));
            int p2 = distance(kKagenTypeListing.begin(), find(kKagenTypeListing.begin(), kKagenTypeListing.end(), x2.graph_type));
            if (p1 != p2) return p1 < p2;

            auto type = x1.graph_type;
            if (type == KagenGraphCollectionDescriptor::Type::GNM)   return x1.iparam < x2.iparam;
            if (type == KagenGraphCollectionDescriptor::Type::RGG2D) return x1.rparam < x2.rparam;
            if (type == KagenGraphCollectionDescriptor::Type::RGG3D) return x1.rparam < x2.rparam;
            if (type == KagenGraphCollectionDescriptor::Type::BA)    return x1.iparam < x2.iparam;
            if (type == KagenGraphCollectionDescriptor::Type::RHG)   return x1.iparam < x2.iparam;
            return false;
        };

        for (int i = 0; i < graphs_per_type * (int)kKagenTypeListing.size(); ++i) {
            KagenGraphCollectionDescriptor::Type type =  kKagenTypeListing[i / graphs_per_type];
            int it = i % graphs_per_type;
            KagenGraphCollectionDescriptor descr(type, 123, it);
            all_kagen_sets_to_evaluate.push_back(descr);
        }

        sort(all_kagen_sets_to_evaluate.begin(), all_kagen_sets_to_evaluate.end(), is_smaller);
        all_sets_to_evaluate.resize(graphs_per_type * kKagenTypeListing.size());
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
    if (graph_generation_mode == GraphGenerationMode::KagenSampling) {
        auto elist = all_kagen_sets_to_evaluate.at(id).GenerateEdgeList();
        MaxCutGraph ret(elist);
        ret.SetGraphNaming(all_kagen_sets_to_evaluate.at(id).Serialize());
        return ret;
    } else {
        return MaxCutGraph(all_sets_to_evaluate[id]);
    }
}


const vector<GraphDatabase::KagenGraphCollectionDescriptor::Type> GraphDatabase::kKagenTypeListing {
    KagenGraphCollectionDescriptor::Type::GNM,
    KagenGraphCollectionDescriptor::Type::RGG2D,
    KagenGraphCollectionDescriptor::Type::RGG3D,
    KagenGraphCollectionDescriptor::Type::BA,
    KagenGraphCollectionDescriptor::Type::RHG
};

const map<GraphDatabase::KagenGraphCollectionDescriptor::Type, string> GraphDatabase::KagenGraphCollectionDescriptor::kKagenNaming {
    { KagenGraphCollectionDescriptor::Type::GNM, "GNM" },
    { KagenGraphCollectionDescriptor::Type::RGG2D, "RGG2D" },
    { KagenGraphCollectionDescriptor::Type::RGG3D, "RGG3D" },
    { KagenGraphCollectionDescriptor::Type::BA, "BA" },
    { KagenGraphCollectionDescriptor::Type::RHG, "RHG" }
};

