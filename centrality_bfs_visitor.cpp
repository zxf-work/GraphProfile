#include "common.h"
#include <boost/graph/breadth_first_search.hpp>

#include <utility>
#include <vector>
#include <map>
#include <queue>

#include <iostream>
#include <string>

#include "centrality_bfs_visitor.hpp"

using namespace boost;

///visitor fuctions start
void centrality_bfs_visitor::initialize_vertex(const Vertex &u, const Graph &g)
{
    shortestDistanceMap->at(u) = 0;
}

//when discovering a new vertex, following is done:
//1. add its path to the Q
//2. create new path vector in shortestPathsMap
//3. update shortest distance
void centrality_bfs_visitor::discover_vertex(const Vertex &u, const Graph &g)
{
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
    PathList pathList(1, path);

    pathQueue.emplace(path);  //1
    shortestPathsMap->at(u) = pathList; //2
    shortestDistanceMap->at(u) = path.size() - 1; //3
}

//when finding a vertex v that has already been discovered,
//check if the path leading to v is another shortest path
//if so, add the path to the path map
void centrality_bfs_visitor::gray_target(const Edge &e, const Graph &g)
{
    Vertex v = target(e, g);
    //check if the path is same length as shortest
    //if so, then it is anotehr shortest path for u
    if(pathQueue.front().size() == shortestDistanceMap->at(v))
    {
        shortestPathsMap->at(v).emplace_back(pathQueue.front());
        shortestPathsMap->at(v).back().push_back(v);
        //print_shortest_paths();
    }
}

//when finished examining all the adjacent vertices of v, pop the path from the Q. (this is when BFS goes to next vertex)
void centrality_bfs_visitor::finish_vertex(const Vertex &v, const Graph &g)
{
    pathQueue.pop();
}
///visitor fuctions end

///getters start
std::vector< std::vector<Path> >* centrality_bfs_visitor::getShortestPathsMap(){
    return shortestPathsMap;
}

std::vector<unsigned>* centrality_bfs_visitor::getShortestDistanceMap(){
    return shortestDistanceMap;
}
///getters end

///ctors start
centrality_bfs_visitor::centrality_bfs_visitor(std::vector<unsigned>* ShortestDistanceMap, std::vector<PathList>* ShortestPathsMap)
{
    shortestDistanceMap = ShortestDistanceMap;
    shortestPathsMap = ShortestPathsMap;
}

///ctors end
///misc function for dev use
void centrality_bfs_visitor::print_shortest_paths()
{
    int counter = -1;
    for(auto it = shortestPathsMap->begin(); it != shortestPathsMap->end(); ++it)
    {
        ++counter;
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

void centrality_bfs_visitor::print_shortest_distances()
{
    int counter = -1;
    for(auto it = shortestDistanceMap->begin(); it != shortestDistanceMap->end(); ++it)
    {
        counter++;
        std::cout << "Length of shortest path from 0 to vertex " << counter << " is " << *it << std::endl;

    }
}

