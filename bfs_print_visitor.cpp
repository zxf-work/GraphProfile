#include "common.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <iostream>
#include "bfs_print_visitor.hpp"

//prints the vertex when visited via BFS

void bfs_print_visitor::examine_vertex(const Graph::vertex_descriptor &s, const Graph &g) {
  std::cout << "Examine vertex: " << s << std::endl;
}
