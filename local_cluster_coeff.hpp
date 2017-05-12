#ifndef LOCAL_CLUSTERING_COEFF_HPP_INCLUDED
#define LOCAL_CLUSTERING_COEFF_HPP_INCLUDED

#include "common.h"
#include <utility>

using namespace boost;

//computes LCC of a vertex
//LCC is the proportion of pair of neighbours that form a triangle
double local_cluster_coeff(const Vertex &v, const Graph &g)
{
    double localTriCount = 0;

    int deg = degree(v, g);
    if (deg != 0 && deg != 1)
    {
        std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIter = adjacent_vertices(v, g);
        for(AdjacencyIterator ni1 = neighbourIter.first; ni1 != neighbourIter.second - 1; ++ni1)
        {
            for(AdjacencyIterator ni2 = ni1 + 1; ni2 != neighbourIter.second; ++ni2)
            {
                std::pair<Edge, bool> e = edge(*ni1, *ni2, g);
                if(e.second) localTriCount++;
            }
        }
    }
    else return 0;
    double maxTriangle = (double)deg * (deg-1) / 2;
    return localTriCount / maxTriangle;

}

double average_cluster_coeff(const Graph &g)
{
    double sum = 0;
    for(VertexIterator vi = vertices(g).first; vi != vertices(g).second; ++vi)
    {
        sum = sum + local_cluster_coeff(*vi, g);
    }
    return sum / num_vertices(g);
}

#endif // LOCAL_CLUSTERING_COEFF_HPP_INCLUDED
