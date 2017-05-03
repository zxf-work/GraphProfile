#ifndef EDGE_LIST_PRINT_HPP_INCLUDED
#define EDGE_LIST_PRINT_HPP_INCLUDED

#include "common.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

void edge_list_print(const Graph &g)
{
    std::pair<EdgeIterator, EdgeIterator> gEdgeIt = edges(g);
    for(EdgeIterator i = gEdgeIt.first; i != gEdgeIt.second; ++i)
    {
        std::cout << source(*i, g) << " To " << target(*i, g) << std::endl;
    }
}

#endif // EDGE_LIST_PRINT_HPP_INCLUDED
