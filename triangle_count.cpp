#include "common.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <utility>
#include "triangle_count.hpp"
#include "tbb/tbb.h"

using namespace boost;

//for each vertex v, examines every pair of neighbours u w
//if index of v < u, w, check if there is a uw edge, forming a triangle
unsigned long exact_triangle_count(const Graph &g)
{

    int triangleCount = 0;
    std::pair<VertexIterator, VertexIterator> vertexIter = vertices(g);
    for(VertexIterator vi = vertexIter.first; vi != vertexIter.second; ++vi)
    {
        Vertex v = *vi;
        std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIter = adjacent_vertices(v, g);
        for(AdjacencyIterator ni1 = neighbourIter.first; ni1 != neighbourIter.second; ++ni1)
        {
            //to check all pairs, loop through all neighbours n1 of v
            //and pairs with all neighbours n2 of v that appear on the list after n1
            for(AdjacencyIterator ni2 = ni1; ni2 != neighbourIter.second; ++ni2)
            {
                //if v is the smallest index, check if edge btwn n1 and n2 exist
                if(v < *ni1 && v < *ni2)
                {
                    std::pair<Edge, bool> e = edge(*ni1, *ni2, g);
                    if(e.second) triangleCount++;
                }
            }

        }
    }

    return triangleCount;
}

/*----------  multithreaded implementation  ----------*/

unsigned long exact_triangle_count_multithread(const Graph &g)
{
    return tbb::parallel_reduce(
        tbb::blocked_range<unsigned>(0U, num_vertices(g)),
        0,
        //map function
        [&g](const tbb::blocked_range<unsigned>& r, unsigned value){

            tbb::mutex valueMutex;

            for(unsigned v = r.begin(); v != r.end(); v++){

                //first create a vector of the neighbours of the vertex
                std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIter = adjacent_vertices(v, g);
                tbb::concurrent_vector<Vertex> neighbours;
                tbb::parallel_for_each(neighbourIter.first, neighbourIter.second,
                    [&neighbours](Vertex n){
                        neighbours.push_back(n);
                    }
                );

                //reduce over the neighbors, counting the # of triangles
                value += tbb::parallel_reduce(
                    tbb::blocked_range2d<unsigned>(0U, neighbours.size(), 0U, neighbours.size()),
                    0,
                    [&g, &v, &neighbours](tbb::blocked_range2d<unsigned>&r, unsigned val){
                        for(auto i=r.rows().begin(); i != r.rows().end(); i++){
                            for(auto j=r.cols().begin(); j != r.cols().end(); j++){
                                auto ni = neighbours[i];
                                auto nj = neighbours[j];
                                if(v < ni && v < nj && ni < nj)
                                {
                                    std::pair<Edge, bool> e = edge(ni, nj, g);
                                    if(e.second) val++;
                                }
                            }
                        }
                        return val;
                    },
                    std::plus<unsigned>()
                );



                /*============================================================================================
                =            Alternative implementation: double parallel_for_each loop with mutex            =
                ============================================================================================*/

                // tbb::parallel_for_each(neighbourIter.first, neighbourIter.second,
                //     [&g, &v, &value, &neighbourIter, &valueMutex](auto n1){
                //         //to check all pairs, loop through all neighbours n1 of v
                //         //and pairs with all neighbours n2 of v that appear on the list after n1
                //         tbb::parallel_for_each(neighbourIter.first, neighbourIter.second,
                //             [&g, &v, &n1, &value, &valueMutex](auto n2){
                //                 //if v is the smallest index, check if edge btwn n1 and n2 exist
                //                 if(v < n1 && v < n2 && n1 < n2)
                //                 {
                //                     std::pair<Edge, bool> e = edge(n1, n2, g);
                //                     if(e.second){
                //                         valueMutex.lock();
                //                         value++;
                //                         valueMutex.unlock();
                //                     }
                //                 }
                //             }
                //         );
                //     }
                // );
            }

            return value;
        },
        //reduce function
        std::plus<unsigned>()
    );
}
