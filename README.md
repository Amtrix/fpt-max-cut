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
    To input a graph through adjacency list format, append ".graph" suffix to file.
    To execute a set of graphs {graph0,...,graph1} which are the same set of graphs but with different randomization seed, one can name them graph0.0,...,graphn.n to make the suite aware of it.


     make benchmark-debug -j4 && ./benchmark-debug -action "kernelization" -disk-suite randomness-large -benchmark-output ../data/output/kernelization/out

      make benchmark-debug -j4 && ./benchmark-debug -action "kernelization" -disk-suite randomness-large -benchmark-output ../data/output/kernelization/out

      [err] event.c:1278: Assertion is_same_common_timeout(&ev->ev_timeout, &ev->ev_io_timeout) failed in event_persist_closure
[xtitan:07444] *** Process received signal ***
[xtitan:07444] Signal: Aborted (6)
[xtitan:07444] Signal code:  (-6)
[xtitan:07444] [ 0] /lib/x86_64-linux-gnu/libpthread.so.0(+0x12890)[0x7f420ddf2890]
[xtitan:07444] [ 1] /lib/x86_64-linux-gnu/libc.so.6(gsignal+0xc7)[0x7f420cbaee97]
[xtitan:07444] [ 2] /lib/x86_64-linux-gnu/libc.so.6(abort+0x141)[0x7f420cbb0801]
[xtitan:07444] [ 3] /usr/lib/x86_64-linux-gnu/libopen-pal.so.20(+0x6dcda)[0x7f420c68dcda]
[xtitan:07444] [ 4] /usr/lib/x86_64-linux-gnu/libopen-pal.so.20(+0x6e0c8)[0x7f420c68e0c8]
[xtitan:07444] [ 5] /usr/lib/x86_64-linux-gnu/libopen-pal.so.20(opal_libevent2022_event_base_loop+0x60d)[0x7f420c685a4d]
[xtitan:07444] [ 6] /usr/lib/x86_64-linux-gnu/libopen-pal.so.20(+0x2b17e)[0x7f420c64b17e]
[xtitan:07444] [ 7] /lib/x86_64-linux-gnu/libpthread.so.0(+0x76db)[0x7f420dde76db]
[xtitan:07444] [ 8] /lib/x86_64-linux-gnu/libc.so.6(clone+0x3f)[0x7f420cc9188f]
[xtitan:07444] *** End of error message ***
Aborted (core dumped)
amtrix@xtitan:/mnt/d/master-thesis/implementation/build$ cd build/^C
amtrix@xtitan:/mnt/d/master-thesis/implementation/build$ cd ..
amtrix@xtitan:/mnt/d/master-thesis/implementation$ git status
On branch master
Your branch is up to date with 'origin/master'.

Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git checkout -- <file>..." to discard changes in working directory)

        modified:   data/output/kernelization/plotting-compare.r
        modified:   data/output/kernelization/plotting-immense.r
        modified:   executables/benchmark.cpp
        modified:   src/benchmarks/benchmark-kernelization.hpp
        modified:   src/graph-database.cpp

no changes added to commit (use "git add" and/or "git commit -a")
amtrix@xtitan:/mnt/d/master-thesis/implementation$ git add --all
warning: CRLF will be replaced by LF in data/output/kernelization/plotting-compare.r.
The file will have its original line endings in your working directory.
warning: CRLF will be replaced by LF in data/output/kernelization/plotting-immense.r.
The file will have its original line endings in your working directory.
amtrix@xtitan:/mnt/d/master-thesis/implementation$ git commit -am "Update"
[master 3bc50ca] Update