#ifndef SHORTEST_PATH_HPP_INCLUDED
#define SHORTEST_PATH_HPP_INCLUDED

#include "common.h"
#include <boost/graph/visitors.hpp>
#include <vector>

//performs BFS, recording distances
//distance left at 0 if unreachable
void bfs_with_distance(const Graph &g, const Vertex &v, std::vector<unsigned>& distanceMap);

//computes distance w/o boost's bfs
unsigned non_boost_distance(const Graph &g, const Vertex v, const Vertex u);

//returns 0 if not reachable
//computes distance of a graph using boost's bfs
unsigned graph_distance(const Graph &g, const Vertex v, const Vertex u);

#endif // SHORTEST_PATH_HPP_INCLUDED
