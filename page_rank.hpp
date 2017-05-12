#ifndef PAGE_RANK_HPP_INCLUDED
#define PAGE_RANK_HPP_INCLUDED

#include "common.h"
#include <utility>
#include <stdlib.h>

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

#endif // PAGE_RANK_HPP_INCLUDED
