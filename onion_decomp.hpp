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
unsigned onion_decomp(const Graph &h, std::vector<unsigned> &layerCount);


#endif // ONION_DECOMP_HPP_INCLUDED
