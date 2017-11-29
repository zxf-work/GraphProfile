#ifndef LOCAL_CLUSTERING_COEFF_HPP_INCLUDED
#define LOCAL_CLUSTERING_COEFF_HPP_INCLUDED

#include "common.h"

using namespace boost;

#ifdef SEQ
//computes LCC of a vertex
//LCC is the proportion of pair of neighbours that form a triangle
double local_cluster_coeff(const Vertex &v, const Graph &g);

double average_cluster_coeff(const Graph &g);
#endif

#ifdef TBB
//multithreaded implementations
double local_cluster_coeff_multithread(const Vertex &v, const Graph &g);

double average_cluster_coeff_multithread(const Graph &g);

double average_cluster_coeff_multithread_reduce(const Graph &g);
#endif

#endif // LOCAL_CLUSTERING_COEFF_HPP_INCLUDED
