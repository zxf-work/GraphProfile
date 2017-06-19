#ifndef SHORTEST_PATH_HPP_INCLUDED
#define SHORTEST_PATH_HPP_INCLUDED

#include "common.h"
#include <boost/graph/breadth_first_search.hpp>

unsigned graph_distance(const Graph &g, const Vertex v, const Vertex u)
{
    std::vector<vertices_size_type> distances;
    distances.resize(num_vertices(g), 0);

    //perform BFS, filling the distances vector
    breadth_first_search(g, v, visitor(make_bfs_visitor(record_distances(&distances[0], on_tree_edge()))));

    return distances.at(u);
}

#endif // SHORTEST_PATH_HPP_INCLUDED
