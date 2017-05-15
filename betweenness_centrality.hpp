#ifndef BETWEENNESS_CENTRALITY_HPP_INCLUDED
#define BETWEENNESS_CENTRALITY_HPP_INCLUDED

#include "common.h"
#include <utility>
#include <vector>
#include "centrality_bfs_visitor.hpp"

typedef std::pair<Vertex, std::vector< std::vector<float> > > CentralityMap;
void betweenness_centrality(const Graph &g, CentralityMap &centralityMap)
{

}

#endif // BETWEENNESS_CENTRALITY_HPP_INCLUDED
