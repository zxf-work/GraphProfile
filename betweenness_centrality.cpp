#include "common.h"
#include <utility>
#include <vector>
#include "centrality_bfs_visitor.hpp"
#include <boost/graph/betweenness_centrality.hpp>
#include <map>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <iostream>
#include "tbb/tbb.h"
#include "betweenness_centrality.hpp"

using namespace boost;


//alg: use BFS to find all shortests paths from s to t, for all s t
//then count the number of shortest paths from s to t that contain v, compute this count for all v not s or t
//then compute BC(v) with this data. BC(v) = sum of (# of st paths through v / # of st paths) for all s t pairs, s, t not v
//O(n) BFS ops = O(n^2 + mn). O(n^3) summations. O(n^3) space.
//Tends to crash on larger graphs
void betweenness_centrality(const Graph &g, std::vector<float> &centralityVector)
{
    typedef std::vector< std::vector<int> > CentralityMatrix;
    typedef std::vector<CentralityMatrix> CentralityMap;
    //CentralityMap[v][s][t] denotes the # of sp's from s to t that contain v

    int n = num_vertices(g);

    CentralityMap centralityMap(n);

    for(auto it = centralityMap.begin(); it != centralityMap.end(); ++it)
    {
        *it = std::vector< std::vector<int> >(num_vertices(g), std::vector<int>(num_vertices(g), 0));
    }

    //shortestPathsCount[s][t] contains the # of sp'd from s to t
    std::vector< std::vector<int> >shortestPathsCount(num_vertices(g), std::vector<int>(num_vertices(g), 0));

    //performing the multiple BFS and storing path data
    for (unsigned i = 0; i < num_vertices(g); ++i)
    {
        std::vector<PathList> shortestPathsMap(n);
        std::vector<unsigned> shortestDistanceMap(n);

        //BFS
        centrality_bfs_visitor vis = centrality_bfs_visitor(&shortestDistanceMap, &shortestPathsMap);
        breadth_first_search(g, vertex(i, g), visitor(vis));

        //storing path data
        for(unsigned t = 0; t < num_vertices(g); ++t)
        {
            //go through each path, for each vertex v in the path, increase the # of shortest st/it paths with v
            for(auto pathListIt = shortestPathsMap.at(t).begin(); pathListIt != shortestPathsMap.at(t).end(); ++pathListIt)
            {
                ++shortestPathsCount.at(i).at(t);
                if (pathListIt->size() > 2)
                {
                    for(auto pathIt = pathListIt->begin() + 1; pathIt != pathListIt->end() - 1; ++pathIt)
                    {
                        ++centralityMap[*pathIt][i][t];
                    }
                }
            }

        }

    }

    //once all BFS is finished, use the data to sum up # of st paths, and # of st paths through a vertex v
    //using this, computer BC(v) for all v
    for (unsigned i = 0; i < num_vertices(g); ++i)
    {
        float centralitySum = 0;

        for(unsigned s = 0 ; s < num_vertices(g) - 1; ++s)
        {
            for(unsigned t = s + 1; t < num_vertices(g); ++t)
            {
                if(s != i && t != i)
                {
                    unsigned betweenPathCount = 0;
                    unsigned totalPathCount = 0;

                    betweenPathCount = centralityMap[i][s][t];
                    totalPathCount = shortestPathsCount[s][t];
                    if (totalPathCount != 0) centralitySum = centralitySum + (float)betweenPathCount / totalPathCount;
                }
            }
        }

        centralityVector[i] = centralitySum;

    }
}

//multithreaded implementation

void betweenness_centrality_multithread(const Graph &g, std::vector<float> &centralityVector)
{
    typedef std::vector< std::vector<int> > CentralityMatrix;
    typedef std::vector<CentralityMatrix> CentralityMap;
    //CentralityMap[v][s][t] denotes the # of sp's from s to t that contain v

    int n = num_vertices(g);

    CentralityMap centralityMap(n);

    for(auto it = centralityMap.begin(); it != centralityMap.end(); ++it)
    {
        *it = std::vector< std::vector<int> >(num_vertices(g), std::vector<int>(num_vertices(g), 0));
    }

    //shortestPathsCount[s][t] contains the # of sp'd from s to t
    std::vector< std::vector<int> >shortestPathsCount(num_vertices(g), std::vector<int>(num_vertices(g), 0));


    //performing the multiple BFS and storing path data

    tbb::parallel_for(tbb::blocked_range2d<int>(0, n, 0, n),
    [&g, &centralityMap, &shortestPathsCount, &n](const tbb::blocked_range2d<int> &r){

        for(int i=r.rows().begin(), i_end=r.rows().end(); i<i_end; i++){

        std::vector<PathList> shortestPathsMap(n);
        std::vector<unsigned> shortestDistanceMap(n);

        //BFS
        centrality_bfs_visitor vis = centrality_bfs_visitor(&shortestDistanceMap, &shortestPathsMap);
        breadth_first_search(g, vertex(i, g), visitor(vis));

            for(int t=r.cols().begin(), t_end=r.cols().end(); t<t_end; t++){
                //go through each path, for each vertex v in the path, increase the # of shortest st/it paths with v
                tbb::parallel_for_each(shortestPathsMap.at(t).begin(), shortestPathsMap.at(t).end(),
                    [&shortestPathsCount, &centralityMap, i, t](auto pathListIt){

                        //for(auto pathListIt=pathListRange.begin(); pathListIt < pathListRange.end(); pathListIt++){
                            ++shortestPathsCount.at(i).at(t);
                            if (pathListIt.size() > 2)
                            {
                                for(auto pathIt = pathListIt.begin() + 1; pathIt != pathListIt.end() - 1; ++pathIt)
                                {
                                    ++centralityMap[*pathIt][i][t];
                                }
                            }
                        //}

                });

                //go through each path, for each vertex v in the path, increase the # of shortest st/it paths with v
                // for(auto pathListIt = shortestPathsMap.at(t).begin(); pathListIt != shortestPathsMap.at(t).end(); ++pathListIt)
                // {
                //     ++shortestPathsCount.at(i).at(t);
                //     if (pathListIt->size() > 2)
                //     {
                //         for(auto pathIt = pathListIt->begin() + 1; pathIt != pathListIt->end() - 1; ++pathIt)
                //         {
                //             ++centralityMap[*pathIt][i][t];
                //         }
                //     }
                // }
            }
        }
    });

    //once all BFS is finished, use the data to sum up # of st paths, and # of st paths through a vertex v
    //using this, computer BC(v) for all v
    tbb::parallel_for( tbb::blocked_range2d<int>(0, n, 0, n),
    [&centralityMap, &shortestPathsCount, &centralityVector, n]( const tbb::blocked_range2d<int> &r ) {
        for(int i=r.rows().begin(), i_end=r.rows().end(); i<i_end; i++){

            float centralitySum = 0;

            for(int j=r.cols().begin(), j_end=r.cols().end(); j<j_end; j++){

                tbb::parallel_for(j+1, n,
                    [&centralityMap, &shortestPathsCount, &centralityVector, &centralitySum, n, i, j](int t){
                        if(j != i && t != i)
                        {
                            unsigned betweenPathCount = 0;
                            unsigned totalPathCount = 0;

                            betweenPathCount = centralityMap[i][j][t];
                            totalPathCount = shortestPathsCount[j][t];
                            if (totalPathCount != 0) centralitySum = centralitySum + (float)betweenPathCount / totalPathCount;
                        }
                    });
                // for(int t = j + 1; t < n; ++t)
                // {
                //     if(j != i && t != i)
                //     {
                //         unsigned betweenPathCount = 0;
                //         unsigned totalPathCount = 0;

                //         betweenPathCount = centralityMap[i][j][t];
                //         totalPathCount = shortestPathsCount[j][t];
                //         if (totalPathCount != 0) centralitySum = centralitySum + (float)betweenPathCount / totalPathCount;
                //     }
                // }
            }

            centralityVector[i] = centralitySum;
        }
    });
}

//approximate BC for a single vertex
//alg: repeatedly select vertex vi
//perform SSSP(BFS) and compute dependency score of vi wrt v - delta_vi*(v)
//add this dependency score to a sum S
//stop once S >= n * c, for some constant c.
//then BC(v) = nS / k, where k random vertices were chosen
float approx_betweenness_centrality(const Graph &g, const Vertex &v)
{
    unsigned n = num_vertices(g);
    unsigned iterations = 0;
    float sum = 0;
    float c = 0.5;
    srand(time(NULL));

    while(sum < float(c*n))
    {
        ++iterations;
        if(iterations > 10000)
        {
            std::cout << "Long loop for betweenness centrality. Vertex :" << v << std::endl;
            break;
        }
        Vertex vi = vertex(rand() % num_vertices(g), g);

        std::vector<PathList> shortestPathsMap(n);
        std::vector<unsigned> shortestDistanceMap(n);


        //BFS
        centrality_bfs_visitor vis(&shortestDistanceMap, &shortestPathsMap);
        breadth_first_search(g, vi, visitor(vis));

        //add to the sum
        //for each vertex u != vi or v, add delta_vi,u(v) to sum
        for(unsigned u = 0; u < num_vertices(g); ++u)
        {
            if(vertex(u, g) != vi && vertex(u, g) != v)
            {
                unsigned pathCount = 0;
                unsigned uPathCount = 0; //# of shortest paths with u

                PathList pathList = shortestPathsMap.at(u);
                for(auto pathListIt = shortestPathsMap.at(u).begin(); pathListIt != shortestPathsMap.at(u).end(); ++pathListIt)
                {
                    ++pathCount;
                    //only check if the path has >2 vertices/length 1
                    if(pathListIt->size() > 2)
                    {
                        //go through the path, except the start and end vertices
                        for(auto pathIt = pathListIt->begin() + 1; pathIt != pathListIt->end() -1; ++pathIt)
                        {
                            if(*pathIt == vertex(u,g)) ++uPathCount;
                        }
                    }
                }

                if (pathCount != 0) //pathCount should only be 0 if graph is not connected
                    sum = sum + (float)uPathCount / pathCount;
            }
        }

    }
    return n * sum / iterations;

}

//expects graph g, reduced graph h with same # of vertices
//computers approx BC for each vertex in g and h
//returns # of vertices of h in top 0.15% of BC that are in top 0.15% of g,
//and # of vertices of h in top 1% and not 0.15% of BC that are in h
std::pair<unsigned, unsigned>betweenness_centrality_test(const Graph &g, const Graph &h)
{
    unsigned n = num_vertices(g);
    std::multimap<float, Vertex> originalBetweennessMap;
    std::multimap<float, Vertex> reducedBetweennessMap;

    //compute BC
    for(unsigned v = 0; v < n; ++v)
    {
        originalBetweennessMap.emplace(approx_betweenness_centrality(g, v), v);
        reducedBetweennessMap.emplace(approx_betweenness_centrality(h, v), v);
    }

    double p = (n * 0.0015) > 15 ? n * 0.0015 : 15; //percentile for 0.15%
    std::pair<unsigned, unsigned>betweennessCount; //first number counts # of vertices in original top 0.15, second counts # in top 1%
    auto betweennessIt = reducedBetweennessMap.rbegin();
    for(int i = 0 ; i < p; ++i) //check each top 0.15% of vertices in reduced graph
    {
        Vertex v = betweennessIt->second;

        //check if top ranking vector is in the original's top rank (top 0.15% or top 1%)
        bool top15 = false;
        bool top1 = false;
        auto originalIt = originalBetweennessMap.rbegin();
        for(float j = 0; j < (n * 0.01 ); ++j)
        {
            if (originalIt->second == v)
            {
                if (j < p) top15 = true;
                else top1 = true;
                break;
            }
            ++originalIt;
        }
        if(top15) ++betweennessCount.first;
        if (top1) ++betweennessCount.second;
        ++betweennessIt;
    }

    return betweennessCount;
}

