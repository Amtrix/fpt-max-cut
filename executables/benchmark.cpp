
#include "src/mc-graph.hpp"
#include "src/one-way-reducers.hpp"
#include "src/two-way-reducers.hpp"
#include "src/input-parser.hpp"
#include "src/utils.hpp"
#include "src/output-filter.hpp"
#include "src/graph-database.hpp"
#include "src/colormod.hpp"

//#include "src/benchmarks/benchmark-marked-set.hpp"
#include "src/benchmarks/benchmark-kernelization.hpp"
#include "src/benchmarks/benchmark-count-clique-with-internal.hpp"
#include "src/benchmarks/benchmark-playground.hpp"
#include "src/benchmarks/benchmark-linear-kernel-paper.hpp"

#include <thread>
#include <iostream>
using namespace std;

const bool kMultipleEdgesAreOk = true;

vector<int> tot_used_rules(10, 0);
int main(int argc, char **argv){
    // Init MPI
    MPI_Init(&argc, &argv);
    PEID rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //srand((unsigned)time(0));
    ios_base::sync_with_stdio(false);

    Color::Modifier green(Color::FG_GREEN);
    Color::Modifier red  (Color::FG_RED);
    Color::Modifier defcol(Color::FG_DEFAULT);

    #ifdef DEBUG
        cout << red << "DEBUG is set to true." << defcol << endl;
    #endif
    
    #ifdef NDEBUG
        cout << red << "NDEBUG is set to true." << defcol << endl;
    #endif

    bool local_solver_exists = false;
    #ifdef LOCALSOLVER_EXISTS
        local_solver_exists = true;
    #endif

    std::cout << "Available number of threads: " << std::thread::hardware_concurrency() << endl;
    std::cout << std::fixed;
    std::cout << std::setprecision(5);
    InputParser input(argc, argv);
    InitOutputFiles(input);

    GraphDatabase::KagenGraphCollectionDescriptor::InitializeParamBounds(input);
    GraphDatabase graph_db(input);
    

    const string action = input.getCmdOption("-action");
    std::unique_ptr<BenchmarkAction> benchmark_action;
        
    if (action == "kernelization") {
        benchmark_action.reset(new Benchmark_Kernelization());
    } else if (action == "count-clique-internal") {
        benchmark_action.reset(new Benchmark_CountCliquesWithInternal());
    } else if (action == "playground") {
        benchmark_action.reset(new Benchmark_Playground());
    } else if (action == "linear-kernel") {
        benchmark_action.reset(new Benchmark_LinearKernelPaper());
    } else if (action == "make-unweighted") {
        auto files = GetAllDatasets(input.getCmdOption("-f"));
        for (auto file : files) {
            ifstream in(file.c_str());
            ofstream out((file + ".out").c_str());
            while (in.eof() == false) {
                string strline;
                auto elems = ReadLine(in, &strline);
                if (elems.size() != 3) out << strline << endl;
                else out << elems[0] << " " << elems[1] << endl;
            }
            in.close();
            out.close();
        }
    } else if (action == "scale-weights") {
        auto files = GetAllDatasets(input.getCmdOption("-f"));
        for (auto file : files) {
            ifstream in(file.c_str());
            ofstream out((file + ".out").c_str());
            vector<tuple<int,int,double>> edges;
            while (in.eof() == false) {
                string strline;
                auto elems = ReadLine(in, &strline);
                if (elems.size() != 3 || !isdigit(strline[0])) out << strline << endl;
                else edges.push_back(make_tuple(stoi(elems[0]), stoi(elems[1]), stod(elems[2])));
            }

            for (auto edge : edges)
                out << (get<0>(edge)) << " " << (get<1>(edge)) << " " << (long long)(get<2>(edge)*100000LL) << endl;

            in.close();
            out.close();
        }
    }
    else throw std::logic_error("Action flag not defined.");


    if (benchmark_action) {
        cout << BENCHMARK_NUMBER_OF_THREADS << endl;
        int number_of_threads = (BENCHMARK_NUMBER_OF_THREADS);
        const int number_of_instances = graph_db.GetNumberOfInstances();

        
        if (input.cmdOptionExists("-number-of-threads")) {
            number_of_threads = stoi(input.getCmdOption("-number-of-threads"));    
        } 

        cout << "TOTAL NUMBER OF THREADS FOR INSTANCES: " << number_of_threads << endl;
        cout << "TOTAL NUMBER OF INSTANCES: " << number_of_instances << endl;

        std::mutex mtx_aggregation;
        vector<thread> threads(number_of_threads);
        for (int threadid = 0; threadid < number_of_threads; ++threadid) {
            threads[threadid] = std::thread(std::bind([&](int threadid){
                int lo = (number_of_instances / (double)number_of_threads) * threadid;
                int hi = (number_of_instances / (double)number_of_threads) * (threadid + 1);
                if (threadid == number_of_threads - 1) hi = number_of_instances;

                mtx_aggregation.lock();
                cout << "Thread " << threadid << " assigned range: " << lo << " to " << hi << endl;
                mtx_aggregation.unlock();

                for (int i = lo; i < hi; ++i) {
                    auto graph = graph_db.GetGraphById(i);;

                    if (graph.GraphIsValid()) {
                        cout << "================ RUNNING BENCHMARK ON " + graph.GetGraphNaming() + " ================ " << endl;
                        cout << green << "   |V|:                           " << graph.GetRealNumNodes() << endl;
                        cout << green << "   |E|:                           " << graph.GetRealNumEdges() << endl;
                        cout << green << "   graph contains multiple edges: ";
                        if (graph.info_mult_edge > 0) cout << red;
                        cout << graph.info_mult_edge << defcol << endl;

                        cout << green << "   graph contains self-loops: ";
                        if (graph.info_self_loop_edge > 0) cout << red;
                        cout << graph.info_self_loop_edge << defcol << endl;
                        
                        cout << green << "   Localsolver lib is provided: ";
                        if (local_solver_exists) cout << "yes." << defcol << endl;
                        else cout << red << "no." << defcol << endl;
                    
                        custom_assert(kMultipleEdgesAreOk || graph.info_mult_edge == 0);
                        
                        benchmark_action->Evaluate(input, graph);

                        mtx_aggregation.lock();
                        tot_used_rules = VectorsAdd(tot_used_rules, benchmark_action->tot_used_rules, true);
                        mtx_aggregation.unlock();
                    } else {
                        cout << "not supported." << endl;
                    }
                    cout << "================================= END " + graph.GetGraphNaming() + " ================ " << endl << endl << endl;
                }
            }, threadid));
        }
        std::for_each(threads.begin(), threads.end(), [](std::thread& x){x.join();});

        benchmark_action->PostProcess(input);
    }

    cout << endl;
    cout << " ================ Analysis over all rules commulative ================ " << endl;
    for (int r = 1; r <= 7; ++r)
        cout << "Rule " << r << " was used " << tot_used_rules[r] << " times." << endl;
    cout << " ===================================================================== " << endl << endl;

    MPI_Finalize();
}