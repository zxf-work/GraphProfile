#ifndef BFS_PRINT_VISITOR_H_INCLUDED
#define BFS_PRINT_VISITOR_H_INCLUDED

#include "common.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <iostream>

struct bfs_print_visitor : boost::default_bfs_visitor
{
    void examine_vertex(const Graph::vertex_descriptor &s, const Graph &g) const {
      std::cout << "Examine vertex: " << s << std::endl;
    }
};

#endif // BFS_PRINT_VISITOR_H_INCLUDED
