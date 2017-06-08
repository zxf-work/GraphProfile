#ifndef MIN_SPANNING_TREE_HPP_INCLUDED
#define MIN_SPANNING_TREE_HPP_INCLUDED

#include "common.h"
#include <map>
#include <vector>
#include <boost/graph/breadth_first_search.hpp>
#include <queue>

using namespace boost;

struct spanning_tree_bfs_visitor : default_bfs_visitor
{
private:
    std::map<Vertex, Vertex>* predMap;

public:
    void tree_edge(const Edge& e, const Graph &g)
    {
        predMap->emplace(target(e, g), source(e, g));
    }


    std::map<Vertex, Vertex>* getPredMap() {return predMap;}

    spanning_tree_bfs_visitor(std::map<Vertex, Vertex>* predMap) : predMap(predMap) {}

};

//creates a spanning tree rooted at v, stores the edges in tree
void min_spanning_tree(const Graph& g, Vertex v, std::map<Vertex, Vertex>& tree)
{
    //BFS
    spanning_tree_bfs_visitor vis = spanning_tree_bfs_visitor(&tree);
    breadth_first_search(g, v, visitor(vis));
}

//returns the highest x degree vertices
std::vector<Vertex> high_degree_vertices(const Graph &g, int x = 5)
{
    std::multimap<unsigned, Vertex> vertexList;

    std::pair<VertexIterator, VertexIterator> vertexIter = vertices(g);
    for(auto it = vertexIter.first; it != vertexIter.second; ++it)
    {
        vertexList.emplace(degree(*it, g), *it);
    }

    std::vector<Vertex> retVal;
    for(int i = 0; i < x; ++i)
    {
        auto it = vertexList.begin();
        std::advance(it, num_vertices(g) - 1 - i);
        Vertex y = it->second;
        retVal.push_back(it->second);
    }

    return retVal;
}

//perform bfs, recording the edges of the tree
//choosing the highest degree neighbours to be added to the q first
void high_degree_bfs(const Graph &g, const Vertex &v, std::multimap<Vertex, Vertex>&tree)
{
    std::queue<Vertex> mainQ;
    std::map<Vertex, bool> visitedMap;
    mainQ.push(v);
    visitedMap.emplace(v, true);
    while(!mainQ.empty())
    {
        Vertex current = mainQ.front();

        std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIt = adjacent_vertices(current, g);
        //create ordered map of the neighbours, sorted by degree
        std::multimap<unsigned, Vertex> neighbourDegreeMap;
        for(auto ni = neighbourIt.first; ni != neighbourIt.second; ++ni)
        {
            neighbourDegreeMap.emplace(degree(*ni, g), *ni);
        }
        //add neighbours to the Q
        for(auto it = neighbourDegreeMap.begin(); it != neighbourDegreeMap.end(); ++it)
        {
            //only add if not visited before
            if(visitedMap.find(it->second) == visitedMap.end())
            {
                mainQ.push(it->second);
                tree.emplace(current, it->second);
                visitedMap.emplace(it->second, true);
            }
        }

        mainQ.pop();
    }
}
#endif // MIN_SPANNING_TREE_HPP_INCLUDED
