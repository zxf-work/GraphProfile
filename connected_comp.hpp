#ifndef CONNECTED_COMP_HPP_INCLUDED
#define CONNECTED_COMP_HPP_INCLUDED

#include "common.h"
#include <vector>


using namespace boost;

//returns (# of CC, percentiles of largest CC's)
//lists out size of CC's until 90% of vertices were accounted for
#ifdef SEQ
std::vector<unsigned> connected_comp(const Graph &g);
#endif

//multithreaded version
#ifdef TBB
std::vector<unsigned> connected_comp_multithread(const Graph &g);
#endif

#endif // CONNECTED_COMP_HPP_INCLUDED
