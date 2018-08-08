make && ./benchmark -f ../data/custom/input0
make && ./benchmark -f ../data/biqmac/rudy/g05_60.0
make benchmark-debug && ./benchmark-debug -f ../data/biqmac/rudy/pm1d_80.7 -print-kernalized-graph ../data/output/pm1d_80.7-kernalized


FLAGS:
    -action [action-name]
    Which action the suite should perform.

    -f [input-file-path]
    Runs the benchmark on a single file instead of all in data/{biqmac,custom}.

    -oneway-reduce-marked-size [output-file-path]
    If EvaluateMarkedVertices is being executed, this specifies where to output the table.

    -cc-brute
    Execute brute force algorithm over marked vertices set right after one-way reduction rules are done.

    -print-kernalized-graph [output-file-path]

    -iterations [number]
    Number of iterations to execute for randomized benchmark actions.

    -benchmark-output [output-file-path]
    Where to output the benchmark data.

Helpful stuff:
    To get 1-indexed in KaGen: +1 within generator.io.h