#pragma once

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdarg>
using namespace std;

#include "./output-filter.hpp"
#include "./utils.hpp"
#include "./input-parser.hpp"

template <class T>
void print(std::ostream& out, const T& t, size_t w);

void print_row(std::ostream& out, vector<int> column_size, const char* format, ...);



void InitOutputFiles(const InputParser& input);

void OutputFilterMarkedVertices(const InputParser& input,
                                const string dataset,
                                const int num_nodes,
                                const int num_edges,
                                const int marked_size_oneway,
                                const int marked_size_oneway_reduce,
                                const int marked_size_adhoc);

void OutputCliqueDecompositionIntersection(
                                const InputParser& input,
                                const string dataset,
                                const int num_nodes,
                                const int num_edges,
                                const long long csteps);

void OutputKernelizationApplicabilityCount(
                                const InputParser& input,
                                const string dataset,
                                const int num_nodes,
                                const int num_edges,
                                const int r8_cnt,
                                const int r9_cnt,
                                const int r9x_cnt,
                                const int r10_cnt);
void OutputKernelization(
                                const InputParser& input,
                                const string dataset,
                                const int sec,
                                const int it,
                                const int num_nodes,
                                const int num_edges,
                                const int num_nodes_k,
                                const int num_edges_k,
                                const double k,
                                const double mqlib_sol,
                                const double mqlib_sol_k,
                                const double mqlib_avg_rate,
                                const double mqlib_sddiff,
                                
                                const double localsolver_cut_size,
                                const double localsolver_cut_size_k,
                                const double localsolver_avg_rate,
                                const double localsolver_sddiff,

                                const double locsearch,
                                const double locsearch_k,
                                const double locsearch_avg_rate,
                                const double locsearch_sddiff,

                                const double mqlib_time,
                                const double mqlib_time_k,

                                const double localsolver_time,
                                const double localsolver_time_k,

                                const double biqmac_time,
                                const double biqmac_time_k,

                                const double EE,
                                const double EE_k,
                                const int MAXCUT_best,
                                const double ktime,
                                const string subtyping_output = "");


void OutputLinearKernelAnalysis(const InputParser& input,
                                const string dataset,
                                const int sec,
                                const int it,
                                const int num_nodes,
                                const int num_edges,
                                const int num_nodes_k,
                                const int num_edges_k,
                                const int marked_cnt,
                                const int marked_cnt_reduced,
                                const int rand_marked_cnt,
                                const double oneway_time,
                                const double twoway_time,
                                const double oneway_reduc_time,
                                const double rand_marked_time,
                                const int mcpre, const double mcpre_time,
                                const int mcpost, const double mcpost_time);

void OutputLiveMaxcut(const InputParser& input,
                      const string dataset,
                      const int sec,
                      const int num_nodes,
                      const int num_edges,
                      const double xtime,
                      const int maxcutsz,
                      const string sfx);

void OutputMarkedSetAnalysisMeta(const InputParser& input, const string str);