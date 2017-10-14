#include "common.h"
#include <map>
#include <vector>
#include <boost/graph/breadth_first_search.hpp>
#include <queue>
#include "min_spanning_tree.hpp"

using namespace boost;

void spanning_tree_bfs_visitor::tree_edge(const Edge& e, const Graph &g)
{
    predMap->emplace(target(e, g), source(e, g));
}


std::map<Vertex, Vertex>* spanning_tree_bfs_visitor::getPredMap() {return predMap;}

spanning_tree_bfs_visitor::spanning_tree_bfs_visitor(std::map<Vertex, Vertex>* predMap) : predMap(predMap) {}


//creates a spanning tree rooted at v, stores the edges in tree
void min_spanning_tree(const Graph& g, Vertex v, std::map<Vertex, Vertex>& tree)
{
    //BFS
    spanning_tree_bfs_visitor vis = spanning_tree_bfs_visitor(&tree);
    breadth_first_search(g, v, visitor(vis));
}

//returns the highest x degree vertices
std::vector<Vertex> high_degree_vertices(const Graph &g, int x)
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
        retVal.push_back(it->second);
    }

    return retVal;
}

//perform bfs, recording the edges of the tree
//self made bfs, nothing "high degree"
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
        //add neighbours to the Q
        for(auto ni = neighbourIt.first; ni != neighbourIt.second; ++ni)
        {
            //only add if not visited before
            if(visitedMap.find(*ni) == visitedMap.end())
            {
                mainQ.push(*ni);
                tree.emplace(current, *ni);
                visitedMap.emplace(*ni, true);
            }
        }

        mainQ.pop();
    }
}
