#ifndef LOCAL_CLUSTERING_COEFF_HPP_INCLUDED
#define LOCAL_CLUSTERING_COEFF_HPP_INCLUDED

#include "common.h"
#include <utility>

using namespace boost;

//computes LCC of a vertex
//LCC is the proportion of pair of neighbours that form a triangle
double local_cluster_coeff(const Vertex &v, const Graph &g)
{
    int localTriCount = 0;

    if (out_degree(v, g) != 0)
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
    double maxTriangle = out_degree(v,g)*out_degree(v,g) / 2;
    return localTriCount / maxTriangle;

}

#endif // LOCAL_CLUSTERING_COEFF_HPP_INCLUDED
