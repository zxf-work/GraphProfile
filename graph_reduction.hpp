#ifndef GRAPH_REDUCTION_HPP_INCLUDED
#define GRAPH_REDUCTION_HPP_INCLUDED

#include "common.h"
#include <vector>
#include "min_spanning_tree.hpp"

//testing
#include <iostream>

using namespace boost;


//expects graph g, and empty graph h, integer x
//takes graph g, and creates a reduced graph h
//for each vertex of g, keep x edges, to the lowest degree neighbours
void graph_reduction_reverse(const Graph &g, Graph &h, unsigned x = 3);

//expects graph g, and empty graph h, integer x
//takes graph g, and creates a reduced graph h
//for each vertex of g, keep x edges, to the highest degree neighbours
void graph_reduction(const Graph &g, Graph &h, unsigned x = 3);

//returns median value of degree
//TODO: put in util file
unsigned median_cutoff(const Graph &g);


//expects graph g, empty graph h
//takes graph g, creates reduced graph h
//for each vertex v, if the degree of v > cutoff, then randomly select percentage of its edges
void graph_reduction_percentage(const Graph &g, Graph &h, unsigned cutoff = 10, unsigned percentage = 4);

//expects graph g, empty graph h, and list of vertices from g
//creates a graph by joining spanning trees.
//roots is a vector of vertices from which the spanning trees are rooted
void graph_reduction_spanning_tree(const Graph& g, Graph &h, const std::vector<Vertex>& roots);

//expects graph g, empty graph h, and list of vertices from g
//creates a graph by joining spanning trees using high_degree_bfs (self made bfs instead of boost's bfs)
//roots is a vector of vertices from which the spanning trees are rooted
void graph_reduction_high_degree_tree(const Graph &g, Graph &h, const std::vector<Vertex>& roots);


//expcets graph g, empty graph h, positive integer x
//creates a graph by selecting the highest degree neighbours of each vertex
//for each vertex v, each neighbour is set to priority 1 first
//when edge vu is chosen as an edge to add, set all vertex pairs w, y to priority 2
//if y, z are neighbours of v, and u, w, y form a triangle
void graph_reduction_triangle_avoid(const Graph& g, Graph &h, unsigned x = 3);

#endif // GRAPH_REDUCTION_HPP_INCLUDED
