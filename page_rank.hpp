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
//erases any info on current vector passed on
void page_rank(const Graph &g, std::vector<double> &pageRankVector)
{
    pageRankVector = std::vector<double>(num_vertices(g), (double)1 / num_vertices(g));
    std::vector<double> nextPageRanks(num_vertices(g), 0);

    double maxDifference = 1; //max difference between the iterations, pageRankVector, and nextPageRanks
    double totalDifference = 1;
    int timeCount = 0;
    while(totalDifference > 0.0001) //this loop is for calculating the entire PR matrix multiple times.
    { //criterion is stop is that all page ranks have not differed by more than 0.01
        double difference = 0;
        totalDifference = 0;
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
                sum = sum + pageRankVector.at((unsigned)*ni) / degree(*ni, g);
            }
            *nextIter = ((1 - damp) / num_vertices(g)) + damp * sum;

            if (timeCount != 0)
            {
                if (difference < abs(*nextIter - *currentIter) ) difference = fabs(*nextIter - *currentIter);
                totalDifference = totalDifference + fabs(*nextIter - *currentIter);
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

//computes page rank for g (assuming reduced graph)
//computes # of vertices in the top 0.15% of page rank value of g,
//in top 0.15% page rank of original, and in top 1% (but not top 0.15%) of the original
std::pair<unsigned, unsigned> page_rank_test(const Graph &g, const std::vector<double>& originalPageRanks)
{
    unsigned n = num_vertices(g);
    std::vector<double>pageRankVector(n, 0);
    page_rank(g, pageRankVector); //compute page rank

    //create ordered map of new page ranks
    std::multimap<double, Vertex> pageRankMap;
    for(unsigned i = 0; i < pageRankVector.size(); ++i)
    {
        pageRankMap.emplace(pageRankVector.at(i), i);
    }

    //create oredered map of old page ranks
    std::multimap<double, Vertex> originalPageRankMap;
    for(unsigned i = 0; i < originalPageRanks.size(); ++i)
    {
        originalPageRankMap.emplace(originalPageRanks.at(i), i);
    }

    double p = (n * 0.0015) > 15 ? n * 0.0015 : 15; //percentile for 0.15%
    std::pair<unsigned, unsigned>pageRankCount; //first number counts # of vertices in original top 0.15, second counts # in top 1%
    auto pageRankIt = pageRankMap.rbegin();
    for(int i = 0 ; i < p; ++i) //check each top 0.15% of vertices in reduced graph
    {
        Vertex v = pageRankIt->second;

        //check if top ranking vector is in the original's top rank (top 0.15% or top 1%)
        bool top15 = false;
        bool top1 = false;
        auto originalIt = originalPageRankMap.rbegin();
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
        if(top15) ++pageRankCount.first;
        if (top1) ++pageRankCount.second;
        ++pageRankIt;
    }

    return pageRankCount;

}

#endif // PAGE_RANK_HPP_INCLUDED
