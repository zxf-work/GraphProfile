#include "common.h"
#include <map>
#include <vector>
#include <boost/graph/breadth_first_search.hpp>
#include <queue>
#include <iostream>
#include "min_spanning_tree.hpp"

#ifdef TBB
#include "tbb/tbb.h"
#endif

using namespace boost;

/*================================================================
=                      common functions                          =
================================================================*/
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


/*=========================================================================
=                       Single threaded versions                          =
=========================================================================*/

#ifdef SEQ
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
    for(int i = 0; i < x && i < num_vertices(g); ++i)
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
#endif

/*=========================================================================
=                       Multithreaded versions                            =
=========================================================================*/

#ifdef TBB
//returns the highest x degree vertices
std::vector<Vertex> high_degree_vertices_multithread(const Graph &g, int x)
{
    tbb::concurrent_priority_queue<std::pair<unsigned,Vertex>> vertexPQ;
    std::pair<VertexIterator, VertexIterator> vertexIter = vertices(g);

    tbb::parallel_for_each(vertexIter.first, vertexIter.second,
        [&g, &vertexPQ](auto vi){
            vertexPQ.push(std::make_pair((unsigned)degree(vi, g), vi));
        }
    );

    std::vector<Vertex> retVal;

    std::pair<unsigned,Vertex> top;

    for(int i = 0; i < x; ++i)
    {
        if(vertexPQ.try_pop(top)){
            retVal.push_back(top.second);
        }
        else{
            break; //queue is empty, break
        }
    }

    return retVal;
}

//perform bfs, recording the edges of the tree
//self made bfs, nothing "high degree"
void high_degree_bfs_multithread(const Graph &g, const Vertex &v, tbb::concurrent_unordered_multimap<Vertex, Vertex>&tree)
{
    std::vector<Vertex> mainQ;
    //use unordered_map instead of hash_map because we are mainly
    //concerned about concurrent insertion and traversal, not erasure
    tbb::concurrent_unordered_map<Vertex, bool> visitedMap;

    mainQ.push_back(v);
    visitedMap.emplace(v, true);


    tbb::parallel_do(mainQ.begin(), mainQ.end(),
        [&g, &visitedMap, &tree](Vertex v, tbb::parallel_do_feeder<Vertex>& feeder){

            std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIt = adjacent_vertices(v, g);
            //add neighbours to the Q
            //do not parallelize this loop, segfaults if you do
            for(auto ni = neighbourIt.first; ni != neighbourIt.second; ++ni)
            {
                //only add if not visited before
                if(visitedMap.find(*ni) == visitedMap.end())
                {
                    feeder.add(*ni);
                    tree.emplace(v, *ni);
                    visitedMap.emplace(*ni, true);
                }
            }
        }
    );
}
#endif