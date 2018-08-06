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

void print_row(std::ostream& out, vector<int> column_size, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    int col_dx = 0;
    while (*format != '\0') {
        if (*format == 'd') {
            int i = va_arg(args, int);
            print(out, i, column_size[col_dx++]);
        } else if (*format == 'c') {
            int c = va_arg(args, int);
            print(out, static_cast<char>(c), column_size[col_dx++]);
        } else if (*format == 'f') {
            print(out, va_arg(args, double), column_size[col_dx++]);
        } else if (*format == 's') {
            print(out, va_arg(args, char*), column_size[col_dx++]);
        } else if (*format == 'l') {
            long long i = va_arg(args, long long);
            print(out, i, column_size[col_dx++]);
        }
        ++format;
    }
 
    va_end(args);

    out       << std::endl;
    //std::cout << std::endl;
}


vector<int> kMarkedSizeColumnDescriptor =   {10, 10, 15, 22, 15, 50};
vector<int> cliqueDecompositionDescriptor = {10, 10, 22, 50};
vector<int> kernelizationCountDescriptor =  {10, 10, 10, 10, 10, 10, 60};
vector<int> kernelizationDescriptor =       {10, 10, 10, 10, 10, 10, 15, 15, 15, 15, 20, 10, 10, 10, 60};

void InitOutputFiles(const InputParser& input) {
    if (input.cmdOptionExists("-oneway-reduce-marked-size")) {
        const string output_path = input.getCmdOption("-oneway-reduce-marked-size");
        ofstream out(output_path);
        print_row(out, kMarkedSizeColumnDescriptor, "ssssss", "#|V|", "#|E|", "#oneway-|S|", "#oneway-reduced-|S|", "#adhoc-|S|", "#file");
    }

    if (input.cmdOptionExists("-clique-decomposition-intersection")) {
        const string output_path = input.getCmdOption("-clique-decomposition-intersection");
        ofstream out(output_path);
        print_row(out, kMarkedSizeColumnDescriptor, "ssss", "#|V|", "#|E|", "#computation-steps", "#file");
    }

    if (input.cmdOptionExists("-benchmark-output")) {
        const string action = input.getCmdOption("-action");

        if (action == "kernelization-applicability-count") {
            const string output_path = input.getCmdOption("-benchmark-output");
            ofstream out(output_path);
            print_row(out, kernelizationCountDescriptor, "sssssss", "#|V|", "#|E|", "#|R8|", "#|R9|", "#|R9x|", "#|R10|", "#file");
        }

        if (action == "kernelization") {
            const string output_path = input.getCmdOption("-benchmark-output");
            ofstream out(output_path);
            print_row(out, kernelizationDescriptor, "sssssssssssssss", "#|V(G)|", "#|E(G)|", "#|V(Gk)|", "#|E(Gk)|", "#LOW_B(k)","#LOW_B(kk)", "#MQLIB(G)", "#MQLIB(Gk)", "#locsearch(G)", "#locsearch(Gk)", "#locsearch_ext(Gk)", "EE(G)", "EE(Gk)", "+kk", "#file");
        }
    }
}

void OutputFilterMarkedVertices(const InputParser& input,
                                const string dataset,
                                const int num_nodes,
                                const int num_edges,
                                const int marked_size_oneway,
                                const int marked_size_oneway_reduce,
                                const int marked_size_adhoc) {

    if (input.cmdOptionExists("-oneway-reduce-marked-size")) {
        const string output_path = input.getCmdOption("-oneway-reduce-marked-size");
        ofstream out(output_path, fstream::app);
        print_row(out, kMarkedSizeColumnDescriptor, "ddddds", num_nodes, num_edges, marked_size_oneway, marked_size_oneway_reduce, marked_size_adhoc, dataset.c_str());
    }
}

void OutputCliqueDecompositionIntersection(
                                const InputParser& input,
                                const string dataset,
                                const int num_nodes,
                                const int num_edges,
                                const long long csteps) {

    if (input.cmdOptionExists("-clique-decomposition-intersection")) {
        const string output_path = input.getCmdOption("-clique-decomposition-intersection");
        ofstream out(output_path, fstream::app);
        print_row(out, cliqueDecompositionDescriptor, "ddls", num_nodes, num_edges, csteps, dataset.c_str());
    }
}

void OutputKernelizationApplicabilityCount(
                                const InputParser& input,
                                const string dataset,
                                const int num_nodes,
                                const int num_edges,
                                const int r8_cnt,
                                const int r9_cnt,
                                const int r9x_cnt,
                                const int r10_cnt) {
    if (input.cmdOptionExists("-benchmark-output")) {
        const string output_path = input.getCmdOption("-benchmark-output");
        ofstream out(output_path, fstream::app);
        print_row(out, kernelizationCountDescriptor, "dddddds", num_nodes, num_edges, r8_cnt, r9_cnt, r9x_cnt, r10_cnt, dataset.c_str());
    }
}

void OutputKernelization(
                                const InputParser& input,
                                const string dataset,
                                const int num_nodes,
                                const int num_edges,
                                const int num_nodes_k,
                                const int num_edges_k,
                                const double k,
                                const double k_k,
                                const int mqlib_sol,
                                const int mqlib_sol_k,
                                const int locsearch,
                                const int locsearch_k,
                                const double localsearch_k_extended,
                                const double EE,
                                const double EE_k,
                                const double add_kk) {
    if (input.cmdOptionExists("-benchmark-output")) {
        const string output_path = input.getCmdOption("-benchmark-output");
        ofstream out(output_path, fstream::app);
        print_row(out, kernelizationDescriptor, "ddddffddddffffs", num_nodes, num_edges, num_nodes_k, num_edges_k, k, k_k, mqlib_sol, mqlib_sol_k, locsearch, locsearch_k, localsearch_k_extended, EE, EE_k, add_kk, dataset.c_str());
    }
}