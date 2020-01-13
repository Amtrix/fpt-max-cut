<h1>Max-Cut Kernelization Benchmark Suite</h1>

<h2>Setup:</h2>
<pre><code>git submodule update --init --recursive
cd solvers/MQLIb
make
cd ../../
mkdir build
cd build
cmake ../
make benchmark</code></pre>

<h2>Solvers</h2>
Per default, localsearch (simple implementation by us) and mqlib are run. These are included with our project. This enables our project to perform a comparison between the non-kernelized and kernelized graph's maximum cut computation.

<h3>Linking BiqMac and Localsolver</h3>
Two other famous maximum cut solvers are the BiqMac solver and Localsolver. These are not open source and, therefore, not included with our project. You can manually retrieve those binaries and use build-config.json to specify the paths to their executable. Do this before running cmake in the previous step.

<h2>Run Example</h2>
We have also provided the script <code>run-example.sh</code> to showcase how to use our project. More execution scripts are available in scripts/experiments. They should be only used for education purposes, though. We used them to compute the results for our experiments.

<h2>Execution flags:</h2>

    -action [action-name]
    Which action the suite should perform. Supported values: 'kernelization' or 'linear-kernel'.

    -seed
    Some code areas use randomization. This value seets the initial seed.

    -sample-kagen n
    How many graph instances to generate for each KaGen type (BA,GNM,RGG2D,RGG3D,RHG).

    -f [input-file-path]
    Sets the used graph as a single graph -- retrieved from [input-file-path]. Has no effect if -sample-kagen is also in use.

    -disk-suite [name]
    Utilizes a predefined set of directories/files when searching for datasets. See src/graph-database.hpp.

    -iterations [number]
    Number of times to perform the provided benchmark on each graph.

    -int-weight-lo [number]
    -int-weight-hi [number]
    If one is given, both are required. Specifies the edge weights. Sampled at random. If graph has already given edges weights, this is ignored.

    -output-graphs-dir [path]
    Output all generated graphs into the given directoriy -- kernelized and non-kernelized ones.

    -no-biqmac
    Never execute BiqMac solver.

    -no-localsolver
    Never execute Localsolver

    -do-signed-reduction
    Utilize signed reductions.

    -do-mc-extension-algo
    If a clique forest should be computed and max-cut extension algorithm run. Only works for linear-kernel right now.


Flags when kagen graph sampling is used for graph instance generations:

    -num-nodes [number]
    The number of vertices for the graph generation. Default: 8192.

    -num-edges-lo [number]
    -num-edges-hi [number]
    Respectively, the lower and upper bound on the number of edges for the generated graphs. Should be between 0 and 16*(num_nodes)

    -rhgfix [number]
    Turn of random selection between 2 and 6 and use the given fixed one by this parameter.


Flags for benchmarking performance of kernelization:

    -support-weighted-result
    If the resulting graph is allowed to be weighted. This allows a significant additional reduciton of vertices and edges.

    -benchmark-output [output-file-path]
    Where to output the benchmark data.

    -total-allowed-solver-time [time]
    In seconds! Limits the amount of time the kernelization plus solver computation is allowed to run. If not specified, the allowed time for kernelization is infinite and the solver's time is limited to that of 10 times the amount that was spend on the kernelization, per instance. Set -1 if you want to turn of solvers.

    -locsearch-iterations   [number]
    Indicates the number of times localsearch is to be executed.


Only relevant for benchmarking the linear kernel performance:

    -oneway-reduce-marked-size [output-file-path]
    Specifies where to output the table.

    -do-reduce
    Indicates if the heuristics should be applied to reduce the computed marked vertex set.
    

Tips:
<ul>
    <li>To input a graph through adjacency list format: Append ".graph" suffix to file. Our parser automatically treats it then as such.</li>
    <li>To aggregate the results for a set of graphs {graph0,...,graph1}, name them graph0.0,...,graphn.n.</li>
</ul>
