#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;
typedef boost::graph_traits<Graph>::edge_iterator EdgeIterator;

#endif // COMMON_H_INCLUDED
