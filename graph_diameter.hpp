#ifndef GRAPH_DIAMETER_HPP_INCLUDED
#define GRAPH_DIAMETER_HPP_INCLUDED

#include "common.h"
#include <utility>
#include <boost/graph/graph_traits.hpp>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <algorithm>
#include "bfs_furthest_vertex.hpp"
#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>


//testing includes
#include <iostream>
#include <boost/array.hpp>
#include <array>



using namespace boost;

//idea: use BFS starting at a random vertex v0, find a vertex u0 that is furthest away from v0
//use BFS at vertex u0, find vertex w0 that is furthest away from u0, with distance d0
//repeat with another random vertex v1 != v0
//possible TODO: disallow repeated vertices as v
int approx_graph_diameter(const Graph &g)
{
    srand(time(NULL));


    vertices_size_type diameter = 0;
    //main loop
    for (int i = 0; i < 1000; i++)
    {
        if (i % 50 == 0) { std::cout << "2BFS #" << i+1 << std::endl; }
        //start at random vertex
        Vertex v = vertex(rand() % num_vertices(g), g);

        //find next vertex u
        std::pair<Vertex, vertices_size_type> uPair = bfs_furthest_vertex(g, v);
        if (uPair.second > diameter) diameter = uPair.second;

        //find next vertex w
        std::pair<Vertex, vertices_size_type> wPair = bfs_furthest_vertex(g, v);
        if (wPair.second > diameter) diameter = wPair.second;

    }


    return diameter;


}

//computes all pairs shortest path problem
//uses either johnson's method or floyd's method
//space complexity O(V^2)
//time complextity O(E V logV) for johnson or O(V^3) for floyd
//uses johnson if dense is set to true
int simple_graph_diameter(const Graph &g, bool dense = false)
{
    int V = num_vertices(g);
    std::vector< std::vector<int> > D(V, std::vector<int>(V, -1));
    if (num_edges(g) == 0) return -1;
    //std::vector<int> weights(num_edges(g), 1);


    if(dense) floyd_warshall_all_pairs_shortest_paths(g, D);
    else johnson_all_pairs_shortest_paths(g, D);
    std::cout<< "Finished all pairs shortest paths." << std::endl;
    int shortestPath[V];
    for (int i = 0; i < V; ++i)
    {
        shortestPath[i] = *std::max_element(&D[i][0], &D[i][V] + 1);
    }
    return *std::max_element(shortestPath, shortestPath+V);
}


#endif // GRAPH_DIAMETER_HPP_INCLUDED
