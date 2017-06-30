#ifndef SHORTEST_PATH_HPP_INCLUDED
#define SHORTEST_PATH_HPP_INCLUDED

#include "common.h"
#include <boost/graph/breadth_first_search.hpp>

//performs BFS, recording distances
//distance left at 0 if unreachable
void bfs_with_distance(const Graph &g, const Vertex &v, std::vector<unsigned>& distanceMap)
{
    distanceMap = std::vector<unsigned>(num_vertices(g), 0);
    std::vector<Vertex> predecessor_map(num_vertices(g));

    std::queue<Vertex> mainQ;
    std::map<Vertex, bool> visitedMap;
    mainQ.push(v);
    visitedMap.emplace(v, true);
    while(!mainQ.empty())
    {
        Vertex current = mainQ.front();

        std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIt = adjacent_vertices(current, g);
        //add neighbours to the Q
        for(auto ni = neighbourIt.first; ni != neighbourIt.second; ++ni)
        {
            //only add if not visited before
            if(visitedMap.find(*ni) == visitedMap.end())
            {
                mainQ.push(*ni);
                visitedMap.emplace(*ni, true);

                predecessor_map.at(*ni) = predecessor_map.at(current);
                distanceMap.at(*ni) = distanceMap.at(current) + 1;
            }
        }

        mainQ.pop();
    }
}

//computes distance w/o boost's bfs
unsigned non_boost_distance(const Graph &g, const Vertex v, const Vertex u)
{
    std::vector<unsigned> distanceMap;
    bfs_with_distance(g, v, distanceMap);
    return distanceMap.at(u);
}

//returns 0 if not reachable
//computes distance of a graph using boost's bfs
unsigned graph_distance(const Graph &g, const Vertex v, const Vertex u)
{
    std::vector<vertices_size_type> distances;
    distances.resize(num_vertices(g), 0);

    //perform BFS, filling the distances vector
    breadth_first_search(g, v, visitor(make_bfs_visitor(record_distances(&distances[0], on_tree_edge()))));

    return distances.at(u);
}

#endif // SHORTEST_PATH_HPP_INCLUDED
