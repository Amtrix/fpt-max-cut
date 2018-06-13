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
        }
        ++format;
    }
 
    va_end(args);

    out       << std::endl;
    //std::cout << std::endl;
}


vector<int> kMarkedSizeColumnDescriptor = {10, 10, 15, 22, 15, 50};

void InitOutputFiles(const InputParser& input) {
    if (input.cmdOptionExists("-oneway-reduce-marked-size")) {
        const string output_path = input.getCmdOption("-oneway-reduce-marked-size");
        ofstream out(output_path);
        print_row(out, kMarkedSizeColumnDescriptor, "ssssss", "#|V|", "#|E|", "#oneway-|S|", "#oneway-reduced-|S|", "#adhoc-|S|", "#file");
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