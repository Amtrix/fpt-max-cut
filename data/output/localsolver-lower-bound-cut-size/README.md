The last part of the filename represents the running time of the solver to achieve the result.

A short description of each column:
#file: filename of the dataset that the row is representing
#|V|: number of vertices in that specific dataset
#|E|: number of edges
#|EE|: the edwards-erdos bound for the graph
#|CUT-LO|: a lower-bound for the cut-size as computed by localsolver (in most cases optimal)
#k: #|CUT-LO| - #|EE|


#2.0:
Added #k': represents the amount k is decreased from the one way reductions. And #k-#k' for a better look at the implications.