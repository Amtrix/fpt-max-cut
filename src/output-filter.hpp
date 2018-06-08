#pragma once

#include "./output-filter.hpp"
#include "./utils.hpp"

template <class T>
void print(std::ostream& out, const T& t, size_t w)
{
    out.width(w);
    out       << t << " " << std::flush;
  //  std::cout.width(w);
  //  std::cout << t << " " << std::flush;
}

void print_row(std::ostream& out, vector<pair<int, string>> columns)
{
    for (auto entry : columns)
        print(out, entry.second.c_str(), entry.first);
    out       << std::endl;
    std::cout << std::endl;
}


vector<int> kMarkedSizeColumnDescriptor = {10, 10, 15, 15, 15};

void InitOutputFiles(const InputParser& input) {
    if (input.cmdOptionExists("-oneway-reduce-marked-size")) {
        const string output_path = input.getCmdOption("-oneway-reduce-marked-size");
        ofstream out(output_path);
        print_row(out, ZipVec(kMarkedSizeColumnDescriptor, {"#|V|", "#|E|", "#oneway-|S|", "#oneway-reduced-|S|", "#adhoc-|S|"}));
    }
}

void OutputFilterMarkedVertices(const InputParser& input,
                                const int num_nodes,
                                const int num_edges,
                                const int marked_size_oneway,
                                const int marked_size_oneway_reduce,
                                const int marked_size_adhoc) {
    
    if (input.cmdOptionExists("-oneway-reduce-marked-size")) {
        const string output_path = input.getCmdOption("-oneway-reduce-marked-size");
        ofstream out(output_path, fstream::app);
        print_row(out, ZipVec(kMarkedSizeColumnDescriptor, {
            to_string(num_nodes), to_string(num_edges), to_string(marked_size_oneway), to_string(marked_size_oneway_reduce), to_string(marked_size_adhoc)
        }));
    }
}