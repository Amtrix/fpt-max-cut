#pragma once

#include <vector>
#include <cassert>
#include <random>

#include <test-generators/KaGen/interface/kagen_interface.h>

#include "mc-graph.hpp"
#include "utils.hpp"
#include "input-parser.hpp"

using namespace std;
using namespace kagen;

class InputParser;
class MaxCutGraph;

const map<string, vector<string>> disk_suites = {
    {"randomness-large", {
        "../data/random/web-uk-2002-all.mtx"
    }}
};


// TOUCHING ANYTHING HERE INVALIDATES CONSISTENCY OF SEED->GRAPH SETS
class GraphDatabase{
public:
    enum class GraphGenerationMode {
        KagenSampling,
        SelectedThesisTests
    };
    
    struct KagenGraphCollectionDescriptor{
        enum class Type {
            BA, GNM, RGG2D, RGG3D, RHG
        };

        static const map<KagenGraphCollectionDescriptor::Type, string> kKagenNaming;

        ///////////////////////////////////////////////////////// EVERYTHING FROM HERE CONTRIBUTES SET OF GENERATED GRAPHS ///////////////
        // All ranges inclusive ([a,b])
        // BA collection generation param range:
        static int ba_lo_minimum_vertex_deg, ba_hi_minimum_vertex_deg;
        // GNM collection generation param range:
        static int gnm_lo_num_edges, gnm_hi_num_edges; // num_nodes * x
        // RGG 2D collection generation param range:
        static double rgg_2d_lo_rad, rgg_2d_hi_rad;
        // RGG 3D collection generation param range:
        static double rgg_3d_lo_rad, rgg_3d_hi_rad;
        // RHG collection generation param range:
        static double rhg_lo_e, rhg_hi_e;
        static int rhg_lo_avg_vertex_deg, rhg_hi_avg_vertex_deg;

        static int num_edges_lo, num_edges_hi;

        int num_nodes;
        ///////////////////////////////////////////////////////// TO HERE ////////////////////////////////////////////////////////////////////////

        int num_edges;
        int id = -1;
        Type graph_type;
        int sel_seed;
        double rparam;
        int iparam;
        mt19937 gen;
        uniform_int_distribution<>  idist = std::uniform_int_distribution<>(-1, -1);
        uniform_real_distribution<> rdist = std::uniform_real_distribution<>(-1, -1);

        static void InitializeParamBounds(InputParser& input) {
            ((void) input);
            num_edges_lo = 0, num_edges_hi = 8192 * 8;
            ba_lo_minimum_vertex_deg = 1, ba_hi_minimum_vertex_deg = 16;
            gnm_lo_num_edges = 0, gnm_hi_num_edges = 8;
            rgg_2d_lo_rad = 0.001, rgg_2d_hi_rad = 0.04;
            rgg_3d_lo_rad = 0.001, rgg_3d_hi_rad = 0.11;
            rhg_lo_e = 2.1, rhg_hi_e = 6.5;
            rhg_lo_avg_vertex_deg = 2, rhg_hi_avg_vertex_deg = 32;

            if (input.cmdOptionExists("-num-edges-hi")) {
                num_edges_hi = stoi(input.getCmdOption("-num-edges-hi"));
            }

            if (input.cmdOptionExists("-num-edges-lo")) {
                num_edges_lo = stoi(input.getCmdOption("-num-edges-lo"));
            }
        }

        void GenerateParams() {
            while(1) {
                iparam = idist(gen);
                rparam = rdist(gen);
                num_edges = (int)GenerateEdgeList().size();

                if (num_edges_lo < num_edges && num_edges < num_edges_hi) break;

                // Adjust bounds to exclude overreach for future speed up.
                if (num_edges >= num_edges_hi) {
                    if (graph_type == Type::GNM)   gnm_hi_num_edges = iparam / num_nodes;
                    if (graph_type == Type::RGG2D) rgg_2d_hi_rad = rparam;
                    if (graph_type == Type::RGG3D) rgg_3d_hi_rad = rparam;
                    if (graph_type == Type::BA)    ba_hi_minimum_vertex_deg = iparam;
                    if (graph_type == Type::RHG)   rhg_hi_avg_vertex_deg = iparam;
                } else {
                    if (graph_type == Type::GNM)   gnm_lo_num_edges = iparam / num_nodes;
                    if (graph_type == Type::RGG2D) rgg_2d_lo_rad = rparam;
                    if (graph_type == Type::RGG3D) rgg_3d_lo_rad = rparam;
                    if (graph_type == Type::BA)    ba_lo_minimum_vertex_deg = iparam;
                    if (graph_type == Type::RHG)   rhg_lo_avg_vertex_deg = iparam;
                }
            }
        }
        
        // Generating fixed ranges for params with possibility of specifying nax num edges is stupid, but sufficient for most use cases.
        // (seed,it) used together for final seed.
        KagenGraphCollectionDescriptor(int _id, Type type, int _num_nodes, int seed, int it) {
            id = _id;
            num_nodes = _num_nodes;
            num_edges = -1;
            graph_type = type;
            sel_seed = (seed + it * 13333337) % 100019;

            std::random_device rd;  // Will be used to obtain a seed for the random number engine
            gen = mt19937(sel_seed); // Standard mersenne_twister_engine seeded with rd()

            if (type == Type::GNM)
                idist = std::uniform_int_distribution<>(gnm_lo_num_edges * num_nodes, gnm_hi_num_edges * num_nodes);
            if (type == Type::RGG2D) 
                rdist = std::uniform_real_distribution<>(rgg_2d_lo_rad, rgg_2d_hi_rad);
            if (type == Type::RGG3D)
                rdist = std::uniform_real_distribution<>(rgg_3d_lo_rad, rgg_3d_hi_rad);
            if (type == Type::BA)
                idist = std::uniform_int_distribution<>(ba_lo_minimum_vertex_deg, ba_hi_minimum_vertex_deg);
            if (type == Type::RHG) {
                idist = std::uniform_int_distribution<>(rhg_lo_avg_vertex_deg, rhg_hi_avg_vertex_deg);
                rdist = std::uniform_real_distribution<>(rhg_lo_e, rhg_hi_e);
            }

            GenerateParams();
        }

        const vector<tuple<int,int,int>> GenerateEdgeList() const {
            vector<tuple<int,int,int>> ret;
            KaGen gen(0, 1);

            OutputDebugLog("Generating graph: " + kKagenNaming.at(graph_type) + "(iparam: " + to_string(iparam) + ", rparam: " + to_string(rparam) + ", seed: " + to_string(sel_seed));
            EdgeList edge_list_undirected;
            
            if (graph_type == KagenGraphCollectionDescriptor::Type::GNM)
                edge_list_undirected = gen.GenerateUndirectedGNM(num_nodes, iparam, 0, sel_seed);
            if (graph_type == KagenGraphCollectionDescriptor::Type::RGG2D)
                edge_list_undirected = gen.Generate2DRGG(num_nodes, rparam, 0, sel_seed);
            if (graph_type == KagenGraphCollectionDescriptor::Type::RGG3D)
                edge_list_undirected = gen.Generate2DRGG(num_nodes, rparam, 0, sel_seed);
            if (graph_type == KagenGraphCollectionDescriptor::Type::BA)
                edge_list_undirected = gen.GenerateBA(num_nodes, iparam, 0, sel_seed);
            if (graph_type == KagenGraphCollectionDescriptor::Type::RHG)
                edge_list_undirected = gen.GenerateRHG(num_nodes, rparam, iparam, 0, sel_seed);
            
            edge_list_undirected = RemoveAnyMultipleEdgesAndSelfLoops(edge_list_undirected); // warning! if the input is directed, it will remove one direction.

            OutputDebugLog("Result: |V| = " + to_string(num_nodes) + ", |E| = " + to_string(edge_list_undirected.size()));
            
            // COMPRESS GRAPH?

            for (auto e : edge_list_undirected) {
                ret.push_back(make_tuple(e.first, e.second, 1));
            }

            return ret;
        }

        string Serialize() const {
            std::ostringstream out;
            out.precision(4);
            out << "i" << iparam << "_r" << std::fixed << rparam;

            return kKagenNaming.at(graph_type) + "-" + out.str() + "." + to_string(sel_seed);
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

    MaxCutGraph GetGraphByFile(const string& key) const;

    iterator begin() { return iterator(*this, 0); }
    iterator end() { return iterator(*this, all_sets_to_evaluate.size()); }

private:
    vector<string> all_sets_to_evaluate;
    vector<KagenGraphCollectionDescriptor> all_kagen_sets_to_evaluate;

    GraphGenerationMode graph_generation_mode = GraphGenerationMode::SelectedThesisTests;

    int graphs_per_type = 1;

    int main_seed = 0;

    static const vector<KagenGraphCollectionDescriptor::Type> kKagenTypeListing;
};