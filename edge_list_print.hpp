#ifndef EDGE_LIST_PRINT_HPP_INCLUDED
#define EDGE_LIST_PRINT_HPP_INCLUDED

#include "common.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <fstream>
#include <string>

//prints edges 1 line at a time
//formatted EDGENAME To EDGENAME
void edge_list_print(const Graph &g)
{
    std::pair<EdgeIterator, EdgeIterator> gEdgeIt = edges(g);
    for(EdgeIterator i = gEdgeIt.first; i != gEdgeIt.second; ++i)
    {
        std::cout << source(*i, g) << " To " << target(*i, g) << std::endl;
    }
}

void edge_list_print_file(const Graph &g, std::string filename = "Graph Output.txt")
{
    std::ofstream outputFile;
    outputFile.open(filename.c_str());

    std::pair<EdgeIterator, EdgeIterator> gEdgeIt = edges(g);
    for(EdgeIterator i = gEdgeIt.first; i != gEdgeIt.second; ++i)
    {
        outputFile << source(*i, g) << " " << target(*i, g) << std::endl;
    }
}

#endif // EDGE_LIST_PRINT_HPP_INCLUDED
