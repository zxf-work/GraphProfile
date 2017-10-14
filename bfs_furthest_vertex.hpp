#ifndef BFS_FURTHEST_VERTEX_HPP_INCLUDED
#define BFS_FURTHEST_VERTEX_HPP_INCLUDED

#include "common.h"
#include <vector>
#include <utility>

using namespace boost;

// use BFS to get a vertex furthest away from v
// selects a random vertex u of furthest distance from v
// returns u and the distance of u to v
// note: distance is 0 if there are no reachabile vertices from v
std::pair<Vertex, vertices_size_type> bfs_furthest_vertex(const Graph& g, const Vertex& v);

#endif // BFS_FURTHEST_VERTEX_HPP_INCLUDED
