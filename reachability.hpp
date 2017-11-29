#ifndef REACHABILITY_HPP_INCLUDED
#define REACHABILITY_HPP_INCLUDED

#include "common.h"
#include <boost/graph/breadth_first_search.hpp>
#include <utility>
#include <vector>

using namespace boost;

struct reachability_bfs_visitor : default_bfs_visitor
{
    std::vector<Vertex>* targets;
    std::map<Vertex, bool>* targetMap;

    void discover_vertex(const Vertex &u, const Graph &g);
};

struct reachability_single_bfs_visitor : default_bfs_visitor
{
    bool* reachability;
    Vertex* vertex;

    void discover_vertex(const Vertex &u, const Graph &g);
    reachability_single_bfs_visitor(Vertex* vertex, bool* reachability);
};

bool reachability(const Graph &g, const Vertex& v, Vertex& u);

//tests the reachability of x pairs of vertices that are connected ing if they are connected in h
//expects g, h to have the same # of vertices
//returns # of vertex pairs that are reachable
int random_reachability_test(const Graph &g, const Graph &h, int x);

//deprecated function
std::multimap<Vertex, Vertex> reachability_test(const Graph &g, std::multimap<Vertex, Vertex>& PairMap);

#endif // REACHABILITY_HPP_INCLUDED
