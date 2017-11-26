#include "common.h"
#include <vector>
#include <utility>
#include <algorithm>
#include <boost/graph/graph_traits.hpp>
#include "degrees.hpp"

#ifdef TBB
#include "tbb/tbb.h"
#endif

using namespace std;

//calculates the degrees of all the vertices, and sorts it
// into the vector degrees
void vertex_degrees(const Graph &g, vector<pair<int, Vertex>>& degrees){

	pair<VertexIterator, VertexIterator> vp = vertices(g);
	for (auto v = vp.first; v != vp.second; ++v)
	{
        degrees.push_back(make_pair(degree(*v, g), *v));
    }

 //    #ifdef TBB
	// tbb::parallel_sort(degrees.begin(), degrees.end());
	// #else
	sort(degrees.begin(), degrees.end());
	//#endif

}