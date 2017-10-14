#ifndef LOCAL_CLUSTERING_COEFF_HPP_INCLUDED
#define LOCAL_CLUSTERING_COEFF_HPP_INCLUDED

#include "common.h"

using namespace boost;

//computes LCC of a vertex
//LCC is the proportion of pair of neighbours that form a triangle
double local_cluster_coeff(const Vertex &v, const Graph &g);

double average_cluster_coeff(const Graph &g);

#endif // LOCAL_CLUSTERING_COEFF_HPP_INCLUDED
