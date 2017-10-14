#include "common.h"
#include <boost/graph/breadth_first_search.hpp>
#include <utility>
#include <vector>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "reachability.hpp"

using namespace boost;

void reachability_bfs_visitor::discover_vertex(const Vertex &u, const Graph &g)
{
    targetMap->at(u) = true;
}

void reachability_single_bfs_visitor::discover_vertex(const Vertex &u, const Graph &g)
{
    if(u == *vertex) *reachability = true;
}

reachability_single_bfs_visitor::reachability_single_bfs_visitor(Vertex* vertex, bool* reachability) : 
    vertex(vertex), reachability(reachability) {}

bool reachability(const Graph &g, const Vertex& v, Vertex& u)
{
    bool reachable = false;
    reachability_single_bfs_visitor vis(&u, &reachable);
    breadth_first_search(g, v, visitor(vis));

    return reachable;

}

//tests the reachability of x pairs of vertices that are connected ing if they are connected in h
//expects g, h to have the same # of vertices
//returns # of vertex pairs that are reachable
int random_reachability_test(const Graph &g, const Graph &h, int x)
{
    srand(time(NULL));

    unsigned n = num_vertices(g);
    std::multimap<Vertex, Vertex> VertexPairMap;
    while(VertexPairMap.size() < x)
    {
        Vertex v = vertex(rand() % n, g);
        Vertex u = vertex(rand() % n, g);
        if(reachability(g, v, u)) VertexPairMap.emplace(v, u);
    }
    int reachableCount = 0;
    for(auto it = VertexPairMap.begin(); it != VertexPairMap.end(); ++it)
    {
        if(reachability(h, it->first, it->second)) reachableCount++;
    }
    return reachableCount;
}


//deprecated function
std::multimap<Vertex, Vertex> reachability_test(const Graph &g, std::multimap<Vertex, Vertex>& PairMap)
{
    std::multimap<Vertex,Vertex> retVal;
    for(auto it = PairMap.begin(); it != PairMap.end(); ++it)
    {
        if(!reachability(g, it->first, it->second))
        {
            retVal.emplace(it->first, it->second);
        }
    }
    return retVal;
}
