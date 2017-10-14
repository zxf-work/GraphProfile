#ifndef ADJACENCYLISTREAD_H_INCLUDED
#define ADJACENCYLISTREAD_H_INCLUDED

#include <boost/graph/adjacency_list.hpp>
#include "common.h"

using namespace boost;

// expects file to be an adjecency list that is separated by only whitespace, no punctuation
// ex. 1 2 2 3 4 5 is a graph with edges 1-2, 2-3, 4-5.
// should not be another words in the file
bool adjacency_list_read(Graph& g, const char* filename);

#endif // ADJACENCYLISTREAD_H_INCLUDED
