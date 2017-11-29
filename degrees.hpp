#ifndef DEGREE_PRINT_HPP_INCLUDED
#define DEGREE_PRINT_HPP_INCLUDED

#include "common.h"
#include <vector>
#include <utility>

//returns a vector of all the vertices in G
//and their degree
void vertex_degrees(const Graph &g, std::vector<std::pair<int, Vertex>>& degrees);

#endif // DEGREE_PRINT_HPP_INCLUDED
