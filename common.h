#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>


typedef boost::property<boost::edge_weight_t, int> EdgeWeightProperty;
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, boost::no_property, EdgeWeightProperty> Graph;
typedef boost::graph_traits<Graph>::edge_iterator EdgeIterator;
typedef boost::graph_traits<Graph>::vertex_iterator VertexIterator;
typedef boost::graph_traits<Graph>::adjacency_iterator AdjacencyIterator;
typedef boost::graph_traits<Graph>::vertices_size_type vertices_size_type;
typedef Graph::vertex_descriptor Vertex;
typedef Graph::edge_descriptor Edge;

#endif // COMMON_H_INCLUDED
