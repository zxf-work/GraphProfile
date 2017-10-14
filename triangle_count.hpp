#ifndef TRIANGLE_COUNT_HPP_INCLUDED
#define TRIANGLE_COUNT_HPP_INCLUDED

#include "common.h"

using namespace boost;

//for each vertex v, examines every pair of neighbours u w
//if index of v < u, w, check if there is a uw edge, forming a triangle
unsigned long exact_triangle_count(const Graph &g);

#endif // TRIANGLE_COUNT_HPP_INCLUDED
