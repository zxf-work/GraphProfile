#ifndef CENTRALITY_BFS_VISITOR_HPP_INCLUDED
#define CENTRALITY_BFS_VISITOR_HPP_INCLUDED

#include "common.h"
#include <boost/graph/breadth_first_search.hpp>

#include <utility>
#include <vector>
#include <queue>

using namespace boost;

typedef std::vector<Vertex> Path;
typedef std::vector<Path> PathList;
//when performing BFS with source s
//need to record all shortest paths (the paths themselves) from s to v, for all v != s

struct centrality_bfs_visitor : boost::default_bfs_visitor
{
private:

    std::vector<unsigned>* shortestDistanceMap; //length of shortest path from s to Vertex
    std::vector<PathList>* shortestPathsMap; //shortest paths from s to Vertex. shortestPath[v] contains a vector of all shortest paths from s to v.

    std::queue<Path> pathQueue;

public:

    ///visitor fuctions start
    void initialize_vertex(const Vertex &u, const Graph &g);

    //when discovering a new vertex, following is done:
    //1. add its path to the Q
    //2. create new path vector in shortestPathsMap
    //3. update shortest distance
    void discover_vertex(const Vertex &u, const Graph &g);

    //when finding a vertex v that has already been discovered,
    //check if the path leading to v is another shortest path
    //if so, add the path to the path map
    void gray_target(const Edge &e, const Graph &g);

    //when finished examining all the adjacent vertices of v, pop the path from the Q. (this is when BFS goes to next vertex)
    void finish_vertex(const Vertex &v, const Graph &g);
    ///visitor fuctions end

    ///getters start
    std::vector< std::vector<Path> >* getShortestPathsMap();

    std::vector<unsigned>* getShortestDistanceMap();

    ///ctors start
    centrality_bfs_visitor(std::vector<unsigned>* ShortestDistanceMap, std::vector<PathList>* ShortestPathsMap);

    ///ctors end
    ///misc function for dev use
    void print_shortest_paths();

    void print_shortest_distances();

};


#endif // CENTRALITY_BFS_VISITOR_HPP_INCLUDED
