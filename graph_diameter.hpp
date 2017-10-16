#ifndef GRAPH_DIAMETER_HPP_INCLUDED
#define GRAPH_DIAMETER_HPP_INCLUDED

#include "common.h"
#include <utility>

#include "tbb/tbb.h"

using namespace boost;

//tbb task class for multithreaded implementation of approx_graph_diameter
struct approx_graph_diameter_task{
    const Graph &g;
    vertices_size_type &lowerdiam;
    vertices_size_type &diameter;
public:
    void operator()(const tbb::blocked_range<size_t>& r) const;

    //constructor
    approx_graph_diameter_task(const Graph &g, vertices_size_type &ld, vertices_size_type &d);
};

//idea: use BFS starting at a random vertex v0, find a vertex u0 that is furthest away from v0
//use BFS at vertex u0, find vertex w0 that is furthest away from u0, with distance d0
//repeat with another random vertex v1 != v0
//possible TODO: disallow repeated vertices as v
vertices_size_type approx_graph_diameter(const Graph &g);

//multithreaded version
vertices_size_type approx_graph_diameter_multithread(const Graph &g);

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
