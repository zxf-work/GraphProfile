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
//for each vertex of g, keep x edges, to the lowest degree neighbours
void graph_reduction_reverse(const Graph &g, Graph &h, unsigned x = 3)
{
    unsigned n = num_vertices(g);

    //add the vertices
    if(n != 0)
    {
        add_edge(0, num_vertices(g) - 1, h);
        remove_edge(0, num_vertices(g) - 1, h);
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

//expects graph g, and empty graph h, integer x
//takes graph g, and creates a reduced graph h
//for each vertex of g, keep x edges, to the highest degree neighbours
void graph_reduction(const Graph &g, Graph &h, unsigned x = 3)
{
    unsigned n = num_vertices(g);

    //add the vertices
    if(n != 0)
    {
        add_edge(0, num_vertices(g) - 1, h);
        remove_edge(0, num_vertices(g) - 1, h);
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
        }
        //add edges to up to d/up to x neighbours
        for(int y = 0; y < x && y < d; ++y)
        {
            auto it = neighbourMap.begin();
            std::advance(it, d - 1 - y);
            add_edge(i, it->second, h);
        }
    }
}

//returns median value of degree
//TODO: put in util file
unsigned median_cutoff(const Graph &g)
{
    std::multimap<unsigned, Vertex> degreeMap;
    for(VertexIterator vi = boost::vertices(g).first; vi != boost::vertices(g).second; ++vi)
    {
        degreeMap.emplace(boost::degree(*vi, g), *vi);
    }
    auto it = degreeMap.begin();
    std::advance(it, (degreeMap.size() / 2));

    unsigned median = it->first;
    return median;
}


//expects graph g, empty graph h
//takes graph g, creates reduced graph h
//for each vertex v, if the degree of v > cutoff, then randomly select percentage of its edges
void graph_reduction_percentage(const Graph &g, Graph &h, unsigned cutoff = 10, unsigned percentage = 4)
{
    unsigned n = num_vertices(g);

    //add the vertices
    if(n != 0)
    {
        add_edge(0, num_vertices(g) - 1, h);
        remove_edge(0, num_vertices(g) - 1, h);
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
        add_edge(0, num_vertices(g) - 1, h);
        remove_edge(0, num_vertices(g) - 1, h);
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

void graph_reduction_high_degree_tree(const Graph &g, Graph &h, const std::vector<Vertex>& roots)
{
    unsigned n = num_vertices(g);

    //add vertices
    if (n != 0)
    {
        add_edge(0, num_vertices(g) - 1, h);
        remove_edge(0, num_vertices(g) - 1, h);
    }

    for(auto it = roots.begin(); it != roots.end(); ++it)
    {
        Vertex root = *it;
        std::multimap<Vertex, Vertex> tree;

        high_degree_bfs(g, root, tree); //creates tree

        //add edges to graph
        for(auto treeIt = tree.begin(); treeIt != tree.end(); ++treeIt)
        {
            add_edge(treeIt->first, treeIt->second, h);
        }
    }
}


//expcets graph g, empty graph h, positive integer x
//creates a graph by selecting the highest degree neighbours of each vertex
//for each vertex v, each neighbour is set to priority 1 first
//when edge vu is chosen as an edge to add, set all vertex pairs w, y to priority 2
//if y, z are neighbours of v, and u, w, y form a triangle
void graph_reduction_triangle_avoid(const Graph& g, Graph &h, unsigned x = 3)
{
    unsigned n = num_vertices(g);

    //add the vertices
    if (n != 0)
    {
        add_edge(0, num_vertices(g) - 1, h);
        remove_edge(0, num_vertices(g) - 1, h);
    }

    if (x != 0)
    {
        for(unsigned i = 0; i < num_vertices(g); ++i)
        {
            unsigned d = degree(i, g);

            if(d <= x) //when degree <= # edges to add, add all
            {
                for(auto it = adjacent_vertices(i, g).first; it != adjacent_vertices(i, g).second; ++it)
                {
                    add_edge(i, *it, h);
                }
            }
            else
            {
                std::multimap<unsigned, Vertex> neighbourDegreeMap;
                std::map<Vertex, int> priorityMap;
                for(auto it = adjacent_vertices(i, g).first; it != adjacent_vertices(i, g).second; ++it)
                {
                    neighbourDegreeMap.emplace(degree(*it, g), *it);
                    priorityMap.emplace(*it, 1);
                }

                unsigned edgesAdded = 0;
                int priorityCount = 1; //when adding edges, only add edges with priority set to this number
                //edge adding loop, loop until sufficient number added
                while(edgesAdded < x)
                {
                    //check highest degree neighbours first
                    for(auto it = neighbourDegreeMap.rbegin(); it != neighbourDegreeMap.rend(); ++it)
                    {
                        if(edgesAdded >= x) break;
                        if(priorityMap.at(it->second) <= priorityCount)
                        {
                            add_edge(i, it->second, h);
                            ++edgesAdded;

                            //increase priority of other vertices that form triangle, need to check all other pairs of neighbours
                            for(auto n1 = neighbourDegreeMap.begin(); n1 != --neighbourDegreeMap.end(); ++n1)
                            {
                                auto n1copy = n1;
                                for(auto n2 = ++n1copy; n2 != neighbourDegreeMap.end(); ++n2)
                                {
                                    if(edge(it->second, n1->second, g).second && edge(it->second, n2->second, g).second && edge(n1->second, n2->second, g).second) //if there is triangle
                                    {
                                        ++priorityMap.at(n1->second);
                                        ++priorityMap.at(n2->second);
                                    }
                                }
                            }
                        }
                    }

                    ++priorityCount; //move on to next priority setting
                }
            }
        }
    }

}

#endif // GRAPH_REDUCTION_HPP_INCLUDED
