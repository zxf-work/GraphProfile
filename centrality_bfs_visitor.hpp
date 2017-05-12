#ifndef CENTRALITY_BFS_VISITOR_HPP_INCLUDED
#define CENTRALITY_BFS_VISITOR_HPP_INCLUDED

#include "common.h"
#include <boost/graph/breadth_first_search.hpp>

#include <utility>
#include <vector>
#include <map>
#include <queue>

typedef std::vector< std::vector< unsigned> > ShortestPathMap;


//when performing BFS with source s
//need to record all shortest paths (the paths themselves) from s to v, for all v != s

struct centrality_bfs_visitor : boost::bfs_visitor
{
    typedef std::vector<Vertex> Path;
    std::map<Vertex, unsigned> shortestDistanceMap; //length of shortest path from s to Vertex

    std::map<Vertex, Path> ShortestPathMap; //shortest paths from s to Vertex

};


void bfs(const Graph &g, const Vertex &v)
{
    typedef std::vector<Vertex> Path;
    typedef std::pair<Vertex, Path> VertexPathPair;

    std::queue<VertexPathPair> bfsQueue;


}


/* pseudocode
    start at vertex s
    q = {}. will contain 2 element pairs
    pair [v, p]. the vertex v along wiht the path p used to traverse
    enqueue( [s, s], q)


*\

#endif // CENTRALITY_BFS_VISITOR_HPP_INCLUDED
