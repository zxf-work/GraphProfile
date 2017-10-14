#ifndef BFS_PRINT_VISITOR_H_INCLUDED
#define BFS_PRINT_VISITOR_H_INCLUDED

#include "common.h"
#include <iostream>

//prints the vertex when visited via BFS
struct bfs_print_visitor : boost::default_bfs_visitor
{
    void examine_vertex(const Graph::vertex_descriptor &s, const Graph &g);
};

#endif // BFS_PRINT_VISITOR_H_INCLUDED
