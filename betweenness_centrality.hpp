#ifndef BETWEENNESS_CENTRALITY_HPP_INCLUDED
#define BETWEENNESS_CENTRALITY_HPP_INCLUDED

#include "common.h"
#include <utility>
#include <vector>
#include "centrality_bfs_visitor.hpp"
#include <boost/graph/betweenness_centrality.hpp>

using namespace boost;




void betweenness_centrality(const Graph &g, std::vector<float> &centralityVector)
{
    typedef std::vector< std::vector<int> > CentralityMatrix;
    typedef std::vector<CentralityMatrix> CentralityMap;
    //CentralityMap[v][s][t] denotes the # of sp's from s to t that contain v

    CentralityMap centralityMap;
    centralityMap.resize(num_vertices(g));
    for(auto it = centralityMap.begin(); it != centralityMap.end(); ++it)
    {
        *it = std::vector< std::vector<int> >(num_vertices(g), std::vector<int>(num_vertices(g), 0));
    }

    //shortestPathsCount[s][t] contains the # of sp'd from s to t
    std::vector< std::vector<int> >shortestPathsCount(num_vertices(g), std::vector<int>(num_vertices(g), 0));


    for (unsigned i = 0; i < num_vertices(g); ++i)
    {
        std::vector<PathList> shortestPathsMap;
        std::vector<unsigned> shortestDistanceMap;

        shortestPathsMap.resize(num_vertices(g));
        shortestDistanceMap.resize(num_vertices(g));

        centrality_bfs_visitor vis = centrality_bfs_visitor(&shortestDistanceMap, &shortestPathsMap);
        breadth_first_search(g, vertex(i, g), visitor(vis));

        for(unsigned t = 0; t < num_vertices(g); ++t)
        {
            //go through each path
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

    for (unsigned i = 0; i < num_vertices(g); ++i)
    {
        float centralitySum = 0;

        unsigned betweenPathCount = 0;
        unsigned totalPathCount = 0;
        for(unsigned s = 0 ; s < num_vertices(g) - 1; ++s)
        {
            for(unsigned t = s + 1; t < num_vertices(g); ++t)
            {
                if(s != i && t != i)
                {
                    betweenPathCount = betweenPathCount + centralityMap[i][s][t];
                    totalPathCount = totalPathCount + shortestPathsCount[s][t];
                    centralitySum = (float)betweenPathCount / totalPathCount;
                }
            }
        }

        centralityVector[i] = centralitySum;

    }


}


#endif // BETWEENNESS_CENTRALITY_HPP_INCLUDED
