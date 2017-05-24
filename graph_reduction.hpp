#ifndef GRAPH_REDUCTION_HPP_INCLUDED
#define GRAPH_REDUCTION_HPP_INCLUDED

#include "common.h"
#include <utility>
#include <map>

//testing
#include <iostream>

using namespace boost;

//expects graph g, and empty graph h
//takes graph g, and creates a reduced graph h
//for each vertex of g, keep x edges, to the highest degree neighbours
void graph_reduction(const Graph& g, Graph &h, int x = 3)
{
    unsigned n = num_vertices(g);

    //add the vertices
    if(n != 0)
    {
        add_edge(0, (unsigned)num_vertices(g), h);
        remove_edge(0, (unsigned)num_vertices(g), h);
    }

    //for each vertex g, go through its neighbours and keep x edges, to the highest degree neighbours
    for(unsigned i = 0; i < num_vertices(g); ++i)
    {
        unsigned d = degree(i, g);
        if (d > x)
        {
            //get list of all the neighbours of vertex i
            std::map<unsigned, Vertex> neighbourMap;
            std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIter = adjacent_vertices(i, g);
            for(AdjacencyIterator ni1 = neighbourIter.first; ni1 != neighbourIter.second; ++ni1)
            {
                neighbourMap.emplace((unsigned)degree(*ni1, g), *ni1);
            }
            //add edges to top x neighbours, maps are sorted by key value (aka the degree)
            for(int y = 0; y < x; ++y)
            {
                auto it = neighbourMap.begin();
                std::advance(it, y);
                add_edge(i, it->second, h);
            }

        }
        else //keep all edges if there are not enough neigbours
        {
            std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIter = adjacent_vertices(i, g);
            for(AdjacencyIterator ni1 = neighbourIter.first; ni1 != neighbourIter.second; ++ni1)
            {
                add_edge(i, *ni1, h);
            }
        }
    }
}

//expects graph g, empty graph h
//takes graph g, creates reduced graph h
//for each vertex v, if the degree of v > cutoff, then randomly select percentage of its edges
void graph_reduction_percentage(const Graph &g, Graph &h, unsigned cutoff = 20, unsigned percentage = 5)
{
    unsigned n = num_vertices(g);

    //add the vertices
    if(n != 0)
    {
        add_edge(0, (unsigned)num_vertices(g), h);
        remove_edge(0, (unsigned)num_vertices(g), h);
    }

    for(unsigned i = 0; i < num_vertices(g); ++i)
    {
        unsigned d = degree(i, g);
        if(d > cutoff)
        {
            std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIter = adjacent_vertices(i, g);
            auto it = neighbourIter.first;
            for(unsigned j = 0; j < d; ++j)
            {
                if(j % percentage == 0) add_edge(i, *it, h);
                ++it;
            }
        }
        else
        {
            std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIter = adjacent_vertices(i, g);
            for(AdjacencyIterator ni1 = neighbourIter.first; ni1 != neighbourIter.second; ++ni1)
            {
                add_edge(i, *ni1, h);
            }
        }
    }
}

#endif // GRAPH_REDUCTION_HPP_INCLUDED
