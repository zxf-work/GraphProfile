#ifndef GRAPH_DIAMETER_HPP_INCLUDED
#define GRAPH_DIAMETER_HPP_INCLUDED

#include "common.h"
#include <utility>

using namespace boost;

//idea: use BFS starting at a random vertex v0, find a vertex u0 that is furthest away from v0
//use BFS at vertex u0, find vertex w0 that is furthest away from u0, with distance d0
//repeat with another random vertex v1 != v0
//possible TODO: disallow repeated vertices as v
#ifdef SEQ
vertices_size_type approx_graph_diameter(const Graph &g);
#endif

//multithreaded version
#ifdef TBB
vertices_size_type approx_graph_diameter_multithread(const Graph &g);
#endif

//if bfs_count = 2, iterations = 1000, same as above.
unsigned approx_graph_diameter_bfs(const Graph &g, int bfs_count, int iterations);

//finds an upper bound and a lower bound to the diameter
//upper bound: diameter of a spanning tree
//this diameter is measured by 2BFS (exact for trees)
//lower bound: 2BFS
//forces 10 iterations
std::pair<vertices_size_type, vertices_size_type> khaled_approx_diameter(const Graph &g);

//computes all pairs shortest path problem
//uses either johnson's method or floyd's method
//space complexity O(V^2)
//time complextity O(E V logV) for johnson or O(V^3) for floyd
//uses johnson if dense is set to true
int simple_graph_diameter(const Graph &g, bool dense = false);

//comptue all pairs shortest path, one vertex at a time
//space complextiy O(V)
//time complexity O(V^2 * EV)
//returns longest shortest path aka diameter
unsigned memory_graph_diamter(const Graph &g);

#endif // GRAPH_DIAMETER_HPP_INCLUDED
