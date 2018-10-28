#pragma once

#include <vector>
#include <cassert>
#include <random>

#include <test-generators/KaGen/interface/kagen_interface.h>

#include "mc-graph.hpp"

using namespace std;
using namespace kagen;

class InputParser;
class MaxCutGraph;

const int kDataSetCount = 1;
const string paths[] = {
   //"../data/biqmac/ising",
   // "../data/biqmac/rudy",
  //  "../data/custom",
  //  "../data/KaGen/ba",
  //  "../data/KaGen/gnp_undirected",
  //  "../data/KaGen/rhg",
  //  "../data/KaGen-2/rhg_1024_g2.4"//,

    "../data/thesis-tests/kagen-large-sparse/tests"
    //"../data/thesis-tests/cschulz/tests"
};

class GraphDatabase{
public:
    enum class GraphGenerationMode {
        KagenSample,
        SelectedThesisTests
    };
    
    struct KagenGraphCollectionDescriptor{
        enum class Type {
            BA, GNM, RGG2D, RGG3D, RHG
        };

        // All ranges inclusive ([a,b])
        // BA collection generation param range:
        const int ba_lo_minimum_vertex_deg = 1, ba_hi_minimum_vertex_deg = 16;
        // GNM collection generation param range:
        const int gnm_lo_num_edges = 0, gnm_hi_num_edges = 4; // num_nodes * 2^x
        // RGG 2D collection generation param range:
        const int rgg_2d_lo_rad = 0.005, rgg_2d_hi_rad = 0.04;
        // RGG 3D collection generation param range:
        const int rgg_3d_lo_rad = 0.03, rgg_3d_hi_rad = 0.105;
        // RHG collection generation param range:
        const int rhg_lo_e = 1.9, rhg_hi_e = 6.5;
        const int rhg_lo_avg_vertex_deg = 2, rhg_hi_avg_vertex_deg = 64;

        int num_nodes;
        int powerlaw_exponent;
        double rparam;
        int iparam;
        mt19937 gen;
        uniform_int_distribution<>  idist;
        uniform_real_distribution<> rdist;
        
        KagenGraphCollectionDescriptor(Type type, int seed, int it, int _num_nodes = 8192) {
            num_nodes = _num_nodes;
            std::random_device rd;  //Will be used to obtain a seed for the random number engine
            gen = mt19937(seed + it * 13333337); //Standard mersenne_twister_engine seeded with rd()

            if (type == Type::BA)
                idist = std::uniform_int_distribution<>(ba_lo_minimum_vertex_deg, ba_hi_minimum_vertex_deg);
            if (type == Type::GNM)
                idist = std::uniform_int_distribution<>(gnm_lo_num_edges * num_nodes, gnm_lo_num_edges * num_nodes);
            if (type == Type::RGG2D)
                rdist = std::uniform_real_distribution<>(rgg_2d_lo_rad, rgg_2d_hi_rad);
            if (type == Type::RGG3D)
                rdist = std::uniform_real_distribution<>(rgg_3d_lo_rad, rgg_3d_hi_rad);
            if (type == Type::RHG) {
                idist = std::uniform_int_distribution<>(rhg_lo_avg_vertex_deg, rhg_hi_avg_vertex_deg);
                rdist = std::uniform_real_distribution<>(rhg_lo_e, rhg_hi_e);

                powerlaw_exponent = rdist(gen);
            }
        
            iparam = idist(gen);
            rparam = idist(gen);
        }

        const vector<tuple<int,int,int>> GenerateEdgeList() {
            vector<tuple<int,int,int>> ret;
            KaGen gen(1, 1);

            auto edge_list_undirected = gen.GenerateUndirectedGNM(
                num_nodes, iparam);

            return ret;
        }
    };
    
    class iterator
    {
        const GraphDatabase& graph_db;
        long id;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = MaxCutGraph;
        using pointer = const MaxCutGraph*;
        using reference = const MaxCutGraph&;
        using difference_type = long;

        iterator(const GraphDatabase &_graph_db, long _id = 0) : graph_db(_graph_db), id(_id) {}
        iterator& operator++() { assert(iterator(graph_db, id + 1) != iterator(graph_db, id)); id++; return *this; }
        bool operator==(iterator other) const { return id == other.id; }
        bool operator!=(iterator other) const { return !(*this == other); }
        MaxCutGraph operator*() { return graph_db.GetGraphById(id); }
    };

    friend class iterator;

    GraphDatabase(InputParser& input);

    MaxCutGraph GetGraphById(const long id) const;

    MaxCutGraph GetGraph(const string& key) const;

    iterator begin() { return iterator(*this, 0); }
    iterator end() { return iterator(*this, all_sets_to_evaluate.size()); }

private:
    vector<string> all_sets_to_evaluate;

    GraphGenerationMode graph_generation_mode = GraphGenerationMode::SelectedThesisTests;
};