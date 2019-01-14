<h1>Max-Cut kernelization benchmark suite</h1>

General flags:

    -action [action-name]
    Which action the suite should perform.

    -seed
    A seed used at every place where randomness is used. This includes: Test generations (KaGen), greedy clique-forest subgraph computation, etc.

    -sample-kagen n
    Sets the used graphs as instances generated by KaGen -- for each type (BA,GNM,RGG2D,RGG3D,RHG) generates n graphs, randomly.

    -f [input-file-path]
    Sets the used graph as a single graph -- retrieved from [input-file-path. Has no effect if -sample-kagen is also in use.

    -disk-suite [name]
    Utilizes a predefined set of directories/files when searching for datasets. See src/graph-database.hpp.

    -iterations [number]
    Number of times to perform the provided benchmark on each graph.

    -int-weight-lo [number]
    -int-weight-hi [number]
    If one is given, both are required. Specifies the edge weights. Sampled at random. If sampler find 0, it resamples as that would influence number of edges.
    If graph has already given edges weights, this is ignored.

    -output-graphs-dir [path]
    Output all graphs into the directoriy -- kernelized and non-kernelized.

    -no-biqmac
    Never execute BiqMac solver.

    -no-localsolver
    Never execute Localsolver

    -do-signed-reduction
    Utilize signed reductions.

    -do-mc-extension-algo
    If a clique forest should be computed an max-cut extension algorithm run. Only works for linear-kernel right now!


Flags when kagen graph sampling is used for graph instance generations:

    -num-nodes [number]
    The number of vertices for the graph generation. Default: 8192.

    -num-edges-lo [number]
    -num-edges-hi [number]
    Respectively, the lower and upper bound on the number of edges for the generated graphs. Should be between 0 and 16*(num_nodes)

    -rhgfix [number]
    Turn of random selection between 2 and 6 and use the given fixed one by this parameter.


Flags for benchmark current kernelization performance:
    -support-weighted-result
    If the resulting graph is allowed to be weighted -- allows further reduciton in terms of number of vertices/edges.

    -print-kernalized-graph [output-file-path]
    Outputs kernelized graph. NOT IN USE RIGHT NOW!

    -benchmark-output [output-file-path]
    Where to output the benchmark data.

    -total-allowed-solver-time [time]
    In seconds! Gives the allocated time the kernelization, solver, etc; instead of simply using 10x kernelization time *additionally* on solver time (10s if that value is less than 10s). is allowed to utilize in TOTAL *per* graph. Set -1 if you want to turn of solvers.

    -locsearch-iterations   [number]
    -mqlib-iterations       [number]
    -localsolver-iterations [number]
    Indicates the number of times localsearch, mqlib, localsolver are to be executed.


Only relevant for benchmarking the linear kernel performance:
    -oneway-reduce-marked-size [output-file-path]
    If EvaluateMarkedVertices is being executed, this specifies where to output the table.

    -cc-brute
    Execute brute force algorithm over marked vertices set right after one-way reduction rules are done. DEPRECATED!

    -do-reduce
    Only relevant for linear kernel. Indicates if the heuristics should be applied to reduce the computed marked vertex set by the algorithm from the linear kernel paper.
    

Helpful stuff:

    To get 1-indexed in KaGen: +1 within generator.io.h
    To input a graph through adjacency list format, append ".graph" suffix to file. Our parser automatically treats it then as adjacency list input format.
    To execute a set of graphs {graph0,...,graph1} which are the same set of graphs but with different randomization seed, one can name them graph0.0,...,graphn.n to make the suite aware of it.