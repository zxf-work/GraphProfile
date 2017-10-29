#ifndef PAGE_RANK_HPP_INCLUDED
#define PAGE_RANK_HPP_INCLUDED

#include "common.h"
#include <utility>
#include <vector>


using namespace boost;

const double damp = 0.85;
//https://en.wikipedia.org/wiki/PageRank for more info
//erases any info on current vector passed on
//stops when the total difference after an iteration is no larger than 0.0001
void page_rank(const Graph &g, std::vector<double> &pageRankVector);

//perform N random walks, 1 starting from each vertex
//page for a vertex v is the # of times v is encountered during walks/ total vertices encountered during walks
void approx_page_rank(const Graph &g, std::vector<double> &pageRankVector);

//computes page rank for g (assuming reduced graph)
//computes # of vertices in the top 0.15% of page rank value of g,
//in top 0.15% page rank of original, and in top 1% (but not top 0.15%) of the original
std::pair<unsigned, unsigned> page_rank_test(const Graph &g, const std::vector<double>& originalPageRanks);


//multithreaded versions
void page_rank_multithread(const Graph &g, std::vector<double> &pageRankVector);

void approx_page_rank_multithread(const Graph &g, std::vector<double> &pageRankVector);

std::pair<unsigned, unsigned> page_rank_test_multithread(const Graph &g, const std::vector<double>& originalPageRanks);

#endif // PAGE_RANK_HPP_INCLUDED
