#include "common.h"
#include <utility>
#include <numeric>
#include <functional>
#include <iostream>
#include "local_cluster_coeff.hpp"

#ifdef TBB
#include "tbb/tbb.h"
#endif

using namespace boost;

#ifdef SEQ
//computes LCC of a vertex
//LCC is the proportion of pair of neighbours that form a triangle
double local_cluster_coeff(const Vertex &v, const Graph &g)
{
    double localTriCount = 0;
    int deg = degree(v, g);

    if (deg >= 2) // if vertex has less than 3 neighbors, they cannot form a triangle in the first place
    {
        //check every pair of neighbours
        std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIter = adjacent_vertices(v, g);
        for(AdjacencyIterator ni1 = neighbourIter.first; ni1 != neighbourIter.second; ++ni1)
        {   
            for(AdjacencyIterator ni2 = ni1; ni2 != neighbourIter.second; ++ni2)
            {                
                if(ni1 != ni2){
                    std::pair<Edge, bool> e = edge(*ni1, *ni2, g);
                    if(e.second){
                        localTriCount++;
                    }
                }
            }
        }
    }
    else return 0;

    double maxTriangle = (double)deg * (deg-1) / 2;
    return localTriCount / maxTriangle;
}

double average_cluster_coeff(const Graph &g)
{
    double sum = 0;
    for(VertexIterator vi = vertices(g).first; vi != vertices(g).second; ++vi)
    {
        sum = sum + local_cluster_coeff(*vi, g);
    }
    return sum / num_vertices(g);
}
#endif

/*=================================================================
=                     Multithreaded versions                      =
=================================================================*/

#ifdef TBB
double local_cluster_coeff_multithread(const Vertex &v, const Graph &g)
{
    double localTriCount = 0;
    int deg = degree(v, g);
    tbb::mutex TriCountMutex;

    if (deg >= 2) // if vertex has less than 3 neighbors, they cannot form a triangle in the first place
    {
        //check every pair of neighbours
        //neighbourIter is a pair (first neighbor, last neighbor)
        std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIter = adjacent_vertices(v, g);

        tbb::parallel_for_each(neighbourIter.first, neighbourIter.second,
            [&g, &localTriCount, &neighbourIter, &TriCountMutex](long int ni1){
                tbb::parallel_for_each(neighbourIter.first, neighbourIter.second,
                    [&g, &localTriCount, &ni1, &neighbourIter, &TriCountMutex](long int ni2){
                        if(ni1 > ni2){
                            std::pair<Edge, bool> e = edge(ni1, ni2, g);
                            if(e.second){
                                TriCountMutex.lock();
                                localTriCount++;
                                TriCountMutex.unlock();
                            }
                        }
                    }
                );
            });

    }
    else return 0;

    double maxTriangle = (double)deg * (deg-1) / 2;
    return localTriCount / maxTriangle;
}

double average_cluster_coeff_multithread(const Graph &g)
{
    double sum = 0;
    tbb::mutex sumMutex;

    tbb::parallel_for_each(vertices(g).first, vertices(g).second,
        [&sum, &g, &sumMutex](long int vi){
            double lcci = local_cluster_coeff_multithread(vi, g);
            sumMutex.lock();
            sum = sum + lcci;
            sumMutex.unlock();
        });
    return sum / num_vertices(g);
}

double average_cluster_coeff_multithread_reduce(const Graph &g)
{
    double sum = tbb::parallel_reduce(
        tbb::blocked_range<long int>(0, num_vertices(g)),
        0.0,
        [&sum, &g](const tbb::blocked_range<long int> & r, double init)->double{
            for(long int i=r.begin(); i!=r.end(); i++){
                init += local_cluster_coeff_multithread(vertex(i, g),g);
            }
            return init;
        },        
        std::plus<double>()
    );
    return sum / num_vertices(g);
}
#endif