#ifndef GRAPH_DIAMETER_HPP_INCLUDED
#define GRAPH_DIAMETER_HPP_INCLUDED

#include "common.h"
#include <boost/graph/adjacency_list.hpp>
#include "diameter_bfs_visitor.hpp"
#include <boost/graph/breadth_first_search.hpp>
#include <utility>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/visitors.hpp>
#include <vector>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <algorithm>
#include "bfs_furthest_vertex.hpp"

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

#endif // GRAPH_DIAMETER_HPP_INCLUDED
