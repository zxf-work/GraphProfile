#ifndef BETWEENNESS_CENTRALITY_HPP_INCLUDED
#define BETWEENNESS_CENTRALITY_HPP_INCLUDED

#include "common.h"
#include <utility>
#include <vector>
#include <boost/graph/betweenness_centrality.hpp>

using namespace boost;


//alg: use BFS to find all shortests paths from s to t, for all s t
//then count the number of shortest paths from s to t that contain v, compute this count for all v not s or t
//then compute BC(v) with this data. BC(v) = sum of (# of st paths through v / # of st paths) for all s t pairs, s, t not v
//O(n) BFS ops = O(n^2 + mn). O(n^3) summations. O(n^3) space.
void betweenness_centrality(const Graph &g, std::vector<float> &centralityVector);

void betweenness_centrality_multithread(const Graph &g, std::vector<float> &centralityVector);

void betweenness_centrality_multithread_3parallel(const Graph &g, std::vector<float> &centralityVector);

//approximate BC for a single vertex
//alg: repeatedly select vertex vi
//perform SSSP(BFS) and compute dependency score of vi wrt v - delta_vi*(v)
//add this dependency score to a sum S
//stop once S >= n * c, for some constant c.
//then BC(v) = nS / k, where k random vertices were chosen
float approx_betweenness_centrality(const Graph &g, const Vertex &v);

//expects graph g, reduced graph h with same # of vertices
//computers approx BC for each vertex in g and h
//returns # of vertices of h in top 0.15% of BC that are in top 0.15% of g,
//and # of vertices of h in top 1% and not 0.15% of BC that are in h
std::pair<unsigned, unsigned>betweenness_centrality_test(const Graph &g, const Graph &h);

#endif // BETWEENNESS_CENTRALITY_HPP_INCLUDED
