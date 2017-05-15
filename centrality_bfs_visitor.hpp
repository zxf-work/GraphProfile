#ifndef CENTRALITY_BFS_VISITOR_HPP_INCLUDED
#define CENTRALITY_BFS_VISITOR_HPP_INCLUDED

#include "common.h"
#include <boost/graph/breadth_first_search.hpp>

#include <utility>
#include <vector>
#include <map>
#include <queue>

//testing
#include <iostream>
#include <string>

using namespace boost;

//when performing BFS with source s
//need to record all shortest paths (the paths themselves) from s to v, for all v != s

struct centrality_bfs_visitor : boost::default_bfs_visitor
{
    typedef std::vector<Vertex> Path;
    typedef std::vector<Path> PathList;

private:

    std::vector<unsigned> shortestDistanceMap; //length of shortest path from s to Vertex
    std::vector<PathList> shortestPathsMap; //shortest paths from s to Vertex. shortestPath[v] contains a vector of all shortest paths from s to v.

    std::queue<Path> pathQueue;

    //centrality_bfs_visitor() {};

public:

    ///visitor fuctions start
    void initialize_vertex(const Vertex &u, const Graph &g)
    {
        shortestDistanceMap.at(u) = 0;
    }

    //when discovering a new vertex, following is done:
    //1. add its path to the Q
    //2. create new path vector in shortestPathsMap
    //3. update shortest distance
    void discover_vertex(const Vertex &u, const Graph &g)
    {
        std::cout<< u << std::endl;
        Path path;
        if(pathQueue.size() == 0) //if initial vertex
        {
            path = Path(1, u);
        }
        else
        {
            path = pathQueue.front();
            path.push_back(u);
        }

        pathQueue.emplace(path);  //1
        shortestPathsMap.emplace(shortestPathsMap.begin() + u, 1, path); //2
        shortestDistanceMap.at(u) = path.size() - 1; //3
    }

    //when finding a vertex v that has already been discovered,
    //check if the path leading to v is another shortest path
    //if so, add the path to the path map
    void gray_target(const Edge &e, const Graph &g)
    {
        Vertex v = target(e, g);

        //check if the path is same length as shortest
        //if so, then it is anotehr shortest path for u
        if(pathQueue.front().size() == shortestDistanceMap.at(v))
        {
            shortestPathsMap.at(v).emplace_back(pathQueue.front());
            shortestPathsMap.at(v).back().push_back(v);
            print_shortest_paths();
        }
    }

    //when finished examining all the adjacent vertices of v, pop the path from the Q. (this is when BFS goes to next vertex)
    void finish_vertex(const Vertex &v, const Graph &g)
    {
        pathQueue.pop();
    }
    ///visitor fuctions end

    ///getters start
    std::vector< std::vector<Path> >& getShortestPathsMap(){
        return shortestPathsMap;
    }

    std::vector<unsigned> getShortestDistanceMap(){
        return shortestDistanceMap;
    }
    ///getters end

    ///ctors start
    centrality_bfs_visitor(const Graph &g)
    {
        shortestDistanceMap.resize(num_vertices(g));
        shortestPathsMap.resize(num_vertices(g));
    }

    centrality_bfs_visitor(unsigned numVertices)
    {
        shortestDistanceMap.resize(numVertices);
        shortestPathsMap.resize(numVertices);
    }
    ///ctors end

    void print_shortest_paths()
    {
        int counter = -1;
        for(auto it = shortestPathsMap.begin(); it != shortestPathsMap.end(); ++it)
        {
            counter++;
            std::cout << "Shortest Paths for vertex " << counter << std::endl;
            int innerCounter = 0;
            for(auto it2 = it->begin(); it2 != it->end(); ++it2)
            {
                innerCounter++;
                std::cout << "Path " << innerCounter << "." << std::endl;
                for(auto pathIt = it2->begin(); pathIt != it2->end(); ++pathIt)
                {
                    std::cout<< *pathIt << " ";
                }
                std::cout << std::endl;
            }
        }
    }

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


*/

#endif // CENTRALITY_BFS_VISITOR_HPP_INCLUDED
