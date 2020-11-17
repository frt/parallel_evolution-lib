# How Parallel Evolution works

Each node could run a different algorithm, while its candidate solutions (individuals) share the same representation.

## Nodes execution and communication

All nodes do these steps:

1. Read configuration files and builds its topology representation;
2. Receive a migrant individual if there's one;
3. Run a number of iterations;
4. Selects one individual from its population and send to all adjacent nodes, according to the topology representation built before;
5. Repeats steps 2-4 until the algorithm ends, by convergence, maximum number of iterations, or another condition;
6. Sends its population and algorithm stats to node 0.

## Node 0

*Node 0* is special, after all nodes ended it receives and sumarizes the results of all nodes at the end.
