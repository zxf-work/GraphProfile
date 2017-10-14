#ifndef EDGE_LIST_PRINT_HPP_INCLUDED
#define EDGE_LIST_PRINT_HPP_INCLUDED

#include "common.h"
#include <string>

//prints edges 1 line at a time
//formatted EDGENAME To EDGENAME
void edge_list_print(const Graph &g);

void edge_list_print_file(const Graph &g, std::string filename = "Graph Output.txt");

#endif // EDGE_LIST_PRINT_HPP_INCLUDED
