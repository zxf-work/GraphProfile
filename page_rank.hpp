#ifndef PAGE_RANK_HPP_INCLUDED
#define PAGE_RANK_HPP_INCLUDED

#include "common.h"
#include <utility>
#include <stdlib.h>

#include <time.h>       /* time */
#include <unordered_map>


using namespace boost;

const double damp = 0.85;
//https://en.wikipedia.org/wiki/PageRank for more info
void page_rank(const Graph &g, std::vector<double> &pageRankVector)
{
    pageRankVector = std::vector<double>(num_vertices(g), (double)1 / num_vertices(g));
    std::vector<double> nextPageRanks(num_vertices(g), 0);

    double maxDifference = 1; //max difference between the iterations, pageRankVector, and nextPageRanks
    int timeCount = 0;
    while(maxDifference > 0.01) //this loop is for calculating the entire PR matrix multiple times.
    { //criterion is stop is that all page ranks have not differed by more than 0.01
        double difference = 0;

        std::vector<double>::iterator currentIter = pageRankVector.begin();
        std::vector<double>::iterator nextIter = nextPageRanks.begin();
        while(currentIter != pageRankVector.end() && nextIter != nextPageRanks.end()) //this loop is calcuating PR(v) for every v
        {
            Vertex v = vertex(currentIter - pageRankVector.begin(), g);

            //sum is sigma(PR(u;t) / deg(u)) for all u that are neighbours of v, as described on wiki page.
            double sum = 0;
            std::pair<AdjacencyIterator, AdjacencyIterator> neighbourIter = adjacent_vertices(v, g);
            for(AdjacencyIterator ni = neighbourIter.first; ni != neighbourIter.second; ++ni)
            {
                sum = sum + pageRankVector.at((int)*ni) / degree(*ni, g);
            }
            *nextIter = ((1 - damp) / num_vertices(g)) + damp * sum;

            if (timeCount != 0)
            {
                if (difference < abs(*nextIter - *currentIter) ) difference = abs(*nextIter - *currentIter);
            }

            ++currentIter;
            ++nextIter;
        }
        pageRankVector = nextPageRanks;
        maxDifference = difference;
        timeCount++;
    }

}

//perform N random walks, 1 starting from each vertex
void approx_page_rank(const Graph &g, std::vector<double> &pageRankVector)
{
    unsigned n = num_vertices(g);
    unsigned pathCount = 0;
    unsigned vertexCount = 0;//total # of times visited a vertex
    int newStartProb = 15; //probablity to start a new random walk, out of 100
    srand(time(NULL));

    pageRankVector = std::vector<double>(n, 0);
    while(pathCount < num_vertices(g))
    {
        std::unordered_map<Vertex, bool>pathMap;
        //Vertex v = vertex(rand() % num_vertices(g), g);
        Vertex v = vertex(pathCount, g);

        pathMap.insert({v, true});
        ++pageRankVector.at(v);
        ++vertexCount;

        Vertex lastVertex = v;
        while(true)
        {
            if(rand() % 100 < newStartProb) break;
            std::pair<AdjacencyIterator, AdjacencyIterator>neighbourIter = adjacent_vertices(lastVertex, g);

            //pick the next vertex by selecting a new vertex from the adjacent ones
            std::vector<Vertex>newVertices;
            for(AdjacencyIterator ni = neighbourIter.first; ni != neighbourIter.second; ++ni)
            {
                if(pathMap.find(*ni) == pathMap.end()) newVertices.push_back(*ni);
            }
            if(newVertices.size() == 0) break; //no new adjacent vertices

            Vertex nextVertex = newVertices.at(rand() % newVertices.size());
            pathMap.insert({nextVertex, true});
            lastVertex = nextVertex;
            ++pageRankVector.at(nextVertex);
            ++vertexCount;

        }
        ++pathCount;
    }
    for(auto it = pageRankVector.begin(); it != pageRankVector.end(); ++it)
    {
        *it = *it / vertexCount; //to ensure sum is 1
    }

}

#endif // PAGE_RANK_HPP_INCLUDED
