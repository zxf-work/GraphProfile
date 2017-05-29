#ifndef MIN_SPANNING_TREE_HPP_INCLUDED
#define MIN_SPANNING_TREE_HPP_INCLUDED

#include "common.h"
#include <map>
#include <vector>
#include <boost/graph/breadth_first_search.hpp>

using namespace boost;

struct spanning_tree_bfs_visitor : default_bfs_visitor
{
private:
    std::vector<Vertex>* predMap;

public:
    void tree_edge(const Edge& e, const Graph &g)
    {
        predMap->at(source(e, g)) = target(e, g);
    }


    std::vector<Vertex>* getPredMap() {return predMap;}

    spanning_tree_bfs_visitor(std::vector<Vertex>* predMap) : predMap(predMap) {}

};

//creates a spanning tree rooted at v, stores the edges in tree
void min_spanning_tree(const Graph& g, Vertex v, std::map<Vertex, Vertex>& tree)
{
    std::vector<Vertex> predMap(num_vertices(g));

    //BFS
    spanning_tree_bfs_visitor vis = spanning_tree_bfs_visitor(&predMap);
    breadth_first_search(g, v, visitor(vis));

    for(unsigned i = 0 ; i < size(predMap); ++i)
    {
        tree.emplace(i, predMap.at(i));
    }
}

#endif // MIN_SPANNING_TREE_HPP_INCLUDED