#pragma once

#include <vector>
#include <cassert>

#include "mc-graph.hpp"

using namespace std;

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
};