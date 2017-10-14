#include "common.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <utility>
#include "triangle_count.hpp"

using namespace boost;

//for each vertex v, examines every pair of neighbours u w
//if index of v < u, w, check if there is a uw edge, forming a triangle
unsigned long exact_triangle_count(const Graph &g)
{

    int triangleCount = 0;
    std::pair<VertexIterator, VertexIterator> vertexIter = vertices(g);
    for(VertexIterator vi = vertexIter.first; vi != vertexIter.second; ++vi)
    {
        Vertex v = *vi;
        std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIter = adjacent_vertices(v, g);
        for(AdjacencyIterator ni1 = neighbourIter.first; ni1 != neighbourIter.second; ++ni1)
        {
            //to check all pairs, loop through all neighbours n1 of v
            //and pairs with all neighbours n2 of v that appear on the list after n1
            for(AdjacencyIterator ni2 = ni1; ni2 != neighbourIter.second; ++ni2)
            {
                //if v is the smallest index, check if edge btwn n1 and n2 exist
                if(v < *ni1 && v < *ni2)
                {
                    std::pair<Edge, bool> e = edge(*ni1, *ni2, g);
                    if(e.second) triangleCount++;
                }
            }

        }
    }

    return triangleCount;

}
