#ifndef MIN_SPANNING_TREE_HPP_INCLUDED
#define MIN_SPANNING_TREE_HPP_INCLUDED

#include "common.h"
#include <map>
#include <boost/graph/breadth_first_search.hpp>
#include <vector>

using namespace boost;

struct spanning_tree_bfs_visitor : default_bfs_visitor
{
private:
    std::map<Vertex, Vertex>* predMap;

public:
    void tree_edge(const Edge& e, const Graph &g);

    std::map<Vertex, Vertex>* getPredMap();

    spanning_tree_bfs_visitor(std::map<Vertex, Vertex>* predMap);
};

//creates a spanning tree rooted at v, stores the edges in tree
void min_spanning_tree(const Graph& g, Vertex v, std::map<Vertex, Vertex>& tree);

//returns the highest x degree vertices
std::vector<Vertex> high_degree_vertices(const Graph &g, int x = 5);

//perform bfs, recording the edges of the tree
//self made bfs, nothing "high degree"
void high_degree_bfs(const Graph &g, const Vertex &v, std::multimap<Vertex, Vertex>&tree);

#endif // MIN_SPANNING_TREE_HPP_INCLUDED
