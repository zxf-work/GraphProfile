#ifndef GRAPH_REDUCTION_HPP_INCLUDED
#define GRAPH_REDUCTION_HPP_INCLUDED

#include "common.h"
#include <utility>
#include <map>
#include "min_spanning_tree.hpp"

//testing
#include <iostream>

using namespace boost;


//expects graph g, and empty graph h, integer x
//takes graph g, and creates a reduced graph h
//for each vertex of g, keep x edges, to the highest degree neighbours
void graph_reduction(const Graph &g, Graph &h, unsigned x = 3)
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

        std::multimap<unsigned, Vertex> neighbourMap;
        //std::vector< std::pair<unsigned, Vertex> > neighbourMap; //this vector stores the neighbour and the neighbour's degrees
        std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIter = adjacent_vertices(i, g);
        for(AdjacencyIterator ni1 = neighbourIter.first; ni1 != neighbourIter.second; ++ni1)
        {
            neighbourMap.emplace((unsigned)degree(*ni1, g), *ni1);
            if(i == 98) std::cout << i << " " << *ni1 << std::endl;
        }
        //add edges to up to d/up to x neighbours
        for(int y = 0; y < x && y < d; ++y)
        {
            auto it = neighbourMap.begin();
            std::advance(it, y);
            add_edge(i, it->second, h);
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
        unsigned d = in_degree(i, g);
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

//expects graph g, empty graph h, and list of vertices from g
//creates a graph by joining spanning trees.
//roots is a vector of vertices from which the spanning trees are rooted
void graph_reduction_spanning_tree(const Graph& g, Graph &h, const std::vector<Vertex>& roots)
{
    unsigned n = num_vertices(g);

    //add vertices
    if (n != 0)
    {
        add_edge(0, (unsigned)num_vertices(g), h);
        remove_edge(0, (unsigned)num_vertices(g), h);
    }

    for(auto it = roots.begin(); it != roots.end(); ++it)
    {
        Vertex root = *it;
        std::map<Vertex, Vertex> tree;

        min_spanning_tree(g, root, tree); //creates tree

        //add edges to graph
        for(auto treeIt = tree.begin(); treeIt != tree.end(); ++treeIt)
        {
            add_edge(treeIt->first, treeIt->second, h);
        }

    }
}

#endif // GRAPH_REDUCTION_HPP_INCLUDED
