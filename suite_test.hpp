#ifndef SUITE_TEST_HPP_INCLUDED
#define SUITE_TEST_HPP_INCLUDED

#include "common.h"
#include <vector>
#include <string>
#include <fstream>
#include <map>

using namespace std;


void testing_funcs(const Graph &g, ofstream& outFile, std::multimap<Vertex, Vertex>& ReachabilityPairs,
                    const std::map<std::pair<Vertex, Vertex>, unsigned>& distanceMap,
                    const std::vector<double>& originalPageRank, bool original = false);

void betweenness_print(const Graph& g, const Graph &h, ofstream& outFile);

//uses teh testing_func test suite to computete data on a bunch of graphs, and their reduced versions
//the reduced versions are computed with top k degree neighbours alg., where k = 2,4,8,16
//testing functions are: approx diam, page rank, reachability, shortest path, cc
void suite_test_top_k(vector<string> filenames);

//uses teh testing_func test suite to computete data on a bunch of graphs, and their reduced versions
//the reduced versions are computed with triangle avoid alg., where k = 2,4,8,16
//testing functions are: approx diam, page rank, reachability, shortest path, cc
void suite_test_triangle_top_k(vector<string> filenames);

//uses teh testing_func test suite to computete data on a bunch of graphs, and their reduced versions
//the reduced versions are computed with combined spanning trees
//testing functions are: approx diam, page rank, reachability, shortest path, cc
void suite_test_spanning_tree(vector<string>filenames);

//uses teh testing_func test suite to computete data on a bunch of graphs, and their reduced versions
//the reduced versions are computed with highest degree neighbours, spanning tree, triangle avoid, and proportional neighbour alg
//testing functions are: approx diam, page rank, reachability, shortest path, cc
void suite_test(vector<string> filenames);

#endif // SUITE_TEST_HPP_INCLUDED
