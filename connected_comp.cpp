#include "common.h"
#include <boost/graph/connected_components.hpp>
#include <utility>
#include <vector>
#include <algorithm>
#include "connected_comp.hpp"
#include "tbb/tbb.h"


using namespace boost;

//returns (# of CC, percentiles of largest CC's)
//lists out size of CC's until 90% of vertices were accounted for
std::vector<unsigned> connected_comp(const Graph &g)
{
    unsigned V = num_vertices(g);
    std::vector<unsigned> component(V);

    unsigned numComp = connected_components(g, &component[0]);

    //counting the CC's
    unsigned sum = 0;

    std::vector<unsigned> componentCount(numComp, 0);
    for (std::vector<unsigned>::iterator i = component.begin(); i != component.end(); ++i)
    {
        ++componentCount[*i];
    }

    //build the return value
    std::vector<unsigned> retVal;
    retVal.push_back(numComp);

    while(((sum * 100) / V )<= 90)
    {
        unsigned* largest = std::max_element(&componentCount[0], &componentCount[0] + componentCount.size());
        sum = sum + *largest;
        retVal.push_back(*largest);
        *largest = 0;

    }

    return retVal;

}


//multithread version
std::vector<unsigned> connected_comp_multithread(const Graph &g)
{
    unsigned V = num_vertices(g);
    std::vector<unsigned> component(V);

    unsigned numComp = connected_components(g, &component[0]);

    //counting the CC's
    unsigned sum = 0;

    std::vector<unsigned> componentCount(numComp, 0);
    tbb::parallel_for_each(component.begin(), component.end()
        [&componentCount](unsigned i){
            ++componentCount[i];
        }
    );

    //build the return value
    std::vector<unsigned> retVal;
    retVal.push_back(numComp);

    while(((sum * 100) / V )<= 90)
    {
        unsigned* largest = std::max_element(&componentCount[0], &componentCount[0] + componentCount.size());
        sum = sum + *largest;
        retVal.push_back(*largest);
        *largest = 0;
    }

    return retVal;

}

