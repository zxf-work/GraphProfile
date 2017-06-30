#ifndef ONION_DECOMP_HPP_INCLUDED
#define ONION_DECOMP_HPP_INCLUDED

#include "common.h"
#include <vector>

using namespace boost;


//finds # of onion decmo layers, and # of vertices in each layer
//# vertices in layer i is stored in layerCount[i]
//returns # of maximal layer i such that layer i != 0 vertices
//using the vecS data type for vertex storage, vertices cannot be deleted
//then all vertices must be kep even with degree 0
//causes many usesless checks, graph type must be changed to improve run time
unsigned onion_decomp(const Graph &h, std::vector<unsigned> &layerCount)
{
    Graph g = h;
    unsigned layer = 0;
    layerCount.resize(1);
    unsigned removed_vertices = 0; //number of vertices with degree 0, thought of not in the graph

    //first count # of vertices of degree 0
    for(auto vi = vertices(g).first; vi != vertices(g).second; ++vi)
    {
        if(degree(*vi, g) == 0) ++layerCount.at(0);
    }
    removed_vertices = layerCount.at(0);

    while(true)
    {
        //check for stop when all vertices have 0 degree, all layers counted
        bool no_vertices = true;
        for(auto vi = vertices(g).first; vi != vertices(g).second; ++vi)
        {
            if(degree(*vi, g) != 0)
            {
                no_vertices = false;
                break;
            }
        }
        if(no_vertices) break;

        ++layer;
        layerCount.resize(layer+1);

        //for each layer i, keep removing vertices of degree =< i until all vertices have degree > i
        while(true)
        {
            for(auto vi = vertices(g).first; vi != vertices(g).second; ++vi)
            {
                //if vertex has degree <= "remove" vertices by removing all edges
                if(degree(*vi, g) <= layer && degree(*vi, g) != 0)
                {
                    while(degree(*vi, g) != 0)
                    {
                        //edge iterators become invalid after removing an edge, so once an edge is removed, must retrieve new iter
                        for(auto ni = adjacent_vertices(*vi, g).first; ni != adjacent_vertices(*vi, g).second; ++ni)
                        {
                            remove_edge(*vi, *ni, g);
                            break;
                        }
                    }

                }
            }

            //after "peeling" check if all vertice have degree layer
            bool finish_peel = true;
            for(auto vi = vertices(g).first; vi != vertices(g).second; ++vi)
            {
                if(degree(*vi, g) <= layer && degree(*vi, g) != 0)
                {
                    finish_peel = false;
                    break;
                }
            }
            if(finish_peel) break;
        }

        //count # of vertices removed in the layer
        unsigned dead_vertices = 0;
        for(auto vi = vertices(g).first;  vi != vertices(g).second; ++vi)
        {
            if(degree(*vi,g) == 0) ++dead_vertices;
        }
        //update removed_vertices, and # vertices in layer
        layerCount.at(layer) = dead_vertices - removed_vertices;
        removed_vertices = dead_vertices;

    }
    return layer;
}


#endif // ONION_DECOMP_HPP_INCLUDED
