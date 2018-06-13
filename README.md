make && ./benchmark -f ../data/custom/input0
make && ./benchmark -f ../data/biqmac/rudy/g05_60.0



FLAGS:
    -f [input-file-path]
    Runs the benchmark on a single file instead of all in data/{biqmac,custom}.

    -oneway-reduce-marked-size [output-file-path]
    If EvaluateMarkedVertices is being executed, this specifies where to output the table.

    -cc-brute
    Execute brute force algorithm over marked vertices set right after one-way reduction rules are done.