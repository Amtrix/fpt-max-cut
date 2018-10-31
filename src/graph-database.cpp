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

            double c1 = x1.num_edges != 0 ? x1.num_nodes / (double)x1.num_edges : 1e9;
            double c2 = x2.num_edges != 0 ? x2.num_nodes / (double)x2.num_edges : 1e9;
            return c1 > c2;
        };

        for (int i = 0; i < graphs_per_type * (int)kKagenTypeListing.size(); ++i) {
            KagenGraphCollectionDescriptor::Type type =  kKagenTypeListing[i / graphs_per_type];
            int it = i % graphs_per_type;
            KagenGraphCollectionDescriptor descr(i / graphs_per_type, type, 1000 + rand()%5000, 1111, it);
            all_kagen_sets_to_evaluate.push_back(descr);
        }

        sort(all_kagen_sets_to_evaluate.begin(), all_kagen_sets_to_evaluate.end(), is_smaller);
        all_sets_to_evaluate.resize(all_kagen_sets_to_evaluate.size());

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

MaxCutGraph GraphDatabase::GetGraphByFile(const string& key) const {
    return MaxCutGraph(key);
}

MaxCutGraph GraphDatabase::GetGraphById(const long id) const {
    if (graph_generation_mode == GraphGenerationMode::KagenSampling) {
        auto elist = all_kagen_sets_to_evaluate.at(id).GenerateEdgeList();
        MaxCutGraph ret(elist);
        ret.SetGraphNaming(all_kagen_sets_to_evaluate.at(id).Serialize());
        ret.SetMixingId(all_kagen_sets_to_evaluate.at(id).id);
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

int GraphDatabase::KagenGraphCollectionDescriptor::num_edges_lo = -1;
int GraphDatabase::KagenGraphCollectionDescriptor::num_edges_hi = -1;


int GraphDatabase::KagenGraphCollectionDescriptor::ba_lo_minimum_vertex_deg = 1;
int GraphDatabase::KagenGraphCollectionDescriptor::ba_hi_minimum_vertex_deg = 16;
int GraphDatabase::KagenGraphCollectionDescriptor::gnm_lo_num_edges = 0;
int GraphDatabase::KagenGraphCollectionDescriptor::gnm_hi_num_edges = 8;
double GraphDatabase::KagenGraphCollectionDescriptor::rgg_2d_lo_rad = 0.001;
double GraphDatabase::KagenGraphCollectionDescriptor::rgg_2d_hi_rad = 0.04;
double GraphDatabase::KagenGraphCollectionDescriptor::rgg_3d_lo_rad = 0.001;
double GraphDatabase::KagenGraphCollectionDescriptor::rgg_3d_hi_rad = 0.11;
double GraphDatabase::KagenGraphCollectionDescriptor::rhg_lo_e = 2.1;
double GraphDatabase::KagenGraphCollectionDescriptor::rhg_hi_e = 6.5;
int GraphDatabase::KagenGraphCollectionDescriptor::rhg_lo_avg_vertex_deg = 2;
int GraphDatabase::KagenGraphCollectionDescriptor::rhg_hi_avg_vertex_deg = 32;