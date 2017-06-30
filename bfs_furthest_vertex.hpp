#ifndef BFS_FURTHEST_VERTEX_HPP_INCLUDED
#define BFS_FURTHEST_VERTEX_HPP_INCLUDED


#include <boost/graph/adjacency_list.hpp>
#include "common.h"
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <vector>
#include <algorithm> //for max element
#include <utility>

using namespace boost;

// use BFS to get a vertex furthest away from v
// selects a random vertex u of furthest distance from v
// returns u and the distance of u to v
// note: distance is 0 if there are no reachabile vertices from v
std::pair<Vertex, vertices_size_type> bfs_furthest_vertex(const Graph& g, const Vertex& v)
{
        std::vector<vertices_size_type> distances;
        distances.resize(num_vertices(g), 0);

        //perform BFS, filling the distances vector
        breadth_first_search(g, v, visitor(make_bfs_visitor(record_distances(&distances[0], on_tree_edge()))));

        //pick u
        vertices_size_type maxDistance = *std::max_element(distances.begin(), distances.end());
        std::vector<vertices_size_type> maxDistanceVertices;

        for(size_t i = 0; i < distances.size(); ++i)
        {
            if(distances[i] ==  maxDistance) maxDistanceVertices.push_back(i);
        }
        Vertex u = maxDistanceVertices[rand() % maxDistanceVertices.size()];

        std::pair<Vertex, vertices_size_type> retVal(u, maxDistance);

        return retVal;
}

#endif // BFS_FURTHEST_VERTEX_HPP_INCLUDED
