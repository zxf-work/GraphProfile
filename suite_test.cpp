#include "common.h"
#include <vector>
#include <string>
#include "adjacency_list_read.hpp"
#include <fstream>
#include "graph_diameter.hpp"
#include "graph_reduction.hpp"
#include "connected_comp.hpp"
#include "edge_list_print.hpp"
#include <map>
#include "reachability.hpp"
#include <math.h>
#include "shortest_path.hpp"
#include "betweenness_centrality.hpp"
#include "onion_decomp.hpp"
#include "page_rank.hpp"
#include "suite_test.hpp"

using namespace std;


void testing_funcs(const Graph &g, ofstream& outFile, std::multimap<Vertex, Vertex>& ReachabilityPairs,
                    const std::map<std::pair<Vertex, Vertex>, unsigned>& distanceMap,
                    const std::vector<double>& originalPageRank, bool original)
{
/*
    outFile << "Edges: " << boost::num_edges(g) << endl;
    outFile << "Vertices: " << boost::num_vertices(g) << endl;;
    //outFile << "Exact Diameter: " << simple_graph_diameter(g) << endl;
    outFile << "Approx Diamter: " << approx_graph_diameter(g) << endl;

    pair<vertices_size_type, vertices_size_type> diameterBounds = khaled_approx_diameter(g);
    outFile << "Khaled Diameter Bounds: " << diameterBounds.first << "," <<diameterBounds.second <<endl;

    //cc
    std::vector<unsigned> connectedCompCount = connected_comp(g);
    outFile << "Connected Components: " << connectedCompCount.at(0) << endl;

    outFile << "Largest Component Sizes: ";
    for(std::vector<unsigned>::iterator i = connectedCompCount.begin() + 1; i != connectedCompCount.end(); ++i)
    {
        outFile << " " << *i;
    }
    outFile << endl;

    //page rank
    if(!original)
    {
        std::pair<unsigned, unsigned> pageRankResults = page_rank_test(g, originalPageRank);
        outFile << "Page Rank Count: # in top 0.15% of original - " << pageRankResults.first;
        outFile << ", # in top 1% of original - " << pageRankResults.second << endl;
    }
    std::vector<double>newPageRanks;
    page_rank(g, newPageRanks);
    multimap<double, Vertex> pageRankMap;
    for(unsigned i = 0; i < boost::num_vertices(g); ++i)
    {
        if(!original) pageRankMap.emplace(newPageRanks.at(i), i);
        else pageRankMap.emplace(originalPageRank.at(i), i);
    }
    double p = (boost::num_vertices(g) * 0.0015) > 15 ? boost::num_vertices(g) * 0.0015 : 15;
    outFile << "Top "<<p<<" Page Rank vertices:";
    for (int i = 0; i < p && i < boost::num_vertices(g); ++i)
    {
        auto it = pageRankMap.begin();
        advance(it, boost::num_vertices(g) - 1 - i);
        outFile << " " << it->second;
    }
    outFile << endl;

*/
    //reachability
    if(!original)
    {
        unsigned reachabilityCount = 0;
        outFile << "Reachability Testing Pairs: ";
        for(auto it = ReachabilityPairs.begin(); it != ReachabilityPairs.end(); ++it)
        {
            if(!reachability(g, it->first, it->second))
            {
                outFile << "(" << it->first << ", " << it->second << ") ";
                ++reachabilityCount;
            }
        }
        outFile << endl;
        outFile << "Total unreachable pairs: " << reachabilityCount <<endl;

    //distance
    //NOTE: if original distance is close to int::max, and then the pair becomes disconnected, could result in false result
        outFile << "Distance Deltas: ";
        std::vector<unsigned>distanceRecorder(10, 0);
        for(auto it = ReachabilityPairs.begin(); it != ReachabilityPairs.end(); ++it)
        {
            unsigned dist = graph_distance(g, it->first, it->second);
            unsigned distDelta = dist > distanceMap.at(*it) ? dist - distanceMap.at(*it) : distanceMap.at(*it) - dist;
            if(distanceRecorder.size() < (distDelta+1)  && distDelta <= 100) distanceRecorder.resize(distDelta+1);
            if (dist != 0) ++distanceRecorder.at(distDelta); //disregard when unreachable
        }
        for(int j = 0; j < 10; ++j)
        {
            outFile << j << " - " << distanceRecorder.at(j) << " ";
        }
        outFile<<endl;

    }

    //onion decomp
    std::vector<unsigned> onionLayerCount;
    outFile << "Layers: " << onion_decomp(g, onionLayerCount) << endl;
    outFile << "Layer Count: ";
    for(auto it = onionLayerCount.begin(); it != onionLayerCount.end(); ++it)
    {
        outFile << *it << " ";
    }
    outFile<< endl;

    outFile<<endl;



}

void betweenness_print(const Graph& g, const Graph &h, ofstream& outFile)
{
    std::pair<unsigned, unsigned> betweennessPair = betweenness_centrality_test(g, h);
        outFile << "Betweenness Centrality: " << betweennessPair.first << ", " << betweennessPair.second <<endl;
}

//uses teh testing_func test suite to computete data on a bunch of graphs, and their reduced versions
//the reduced versions are computed with top k degree neighbours alg., where k = 2,4,8,16
//testing functions are: approx diam, page rank, reachability, shortest path, cc
void suite_test_top_k(vector<string> filenames)
{
    ofstream outFile;
    outFile.open("Top K Test Results.txt");
    for(auto it = filenames.begin(); it != filenames.end(); ++it)
    {
        Graph g;
        if (adjacency_list_read(g, it->c_str()))
        {
            //set up pairs for reachability and shortest path, and page rank
            std::vector<double> pageRankVector;
            page_rank(g, pageRankVector);
            unsigned n = num_vertices(g);
            int reachabilityTestSize = 1000;
            int loopsize = 0;
            std::multimap<Vertex, Vertex> VertexPairMap;
            while(VertexPairMap.size() < reachabilityTestSize)
            {
                if (loopsize == 1000000) {cout << "Long Loop for pairs."<< endl; break;}
                Vertex v = vertex(rand() % n, g);
                Vertex u = vertex(rand() % n, g);
                if(reachability(g, v, u)) VertexPairMap.emplace(v, u);
                ++loopsize;
            }
            std::map<pair<Vertex, Vertex>, unsigned> distanceMap;
            for(auto pairIt = VertexPairMap.begin(); pairIt != VertexPairMap.end(); ++pairIt)
            {
                distanceMap.emplace(*pairIt, graph_distance(g, pairIt->first, pairIt->second));
            }
            //testing original graph
            outFile << *it << endl;
            testing_funcs(g, outFile, VertexPairMap, distanceMap, pageRankVector, true);
            outFile << "Reachability Testing Pairs: ";
            for(auto it = VertexPairMap.begin(); it != VertexPairMap.end(); ++it)
            {
                outFile << "(" << it->first << ", " << it->second << ") ";
            }
            outFile << endl << endl;

            //testing reduced graphs
            for(int i = 1 ; i < 5; ++i)
            {
                int k = pow(2, i);
                Graph h;
                graph_reduction(g, h, k);
                outFile << "Keep top " << k << " neighbours." << endl;
                testing_funcs(h, outFile, VertexPairMap, distanceMap, pageRankVector);
//                betweenness_print(g, h, outFile);
            }
        }


    }
    outFile.close();


}

//uses teh testing_func test suite to computete data on a bunch of graphs, and their reduced versions
//the reduced versions are computed with triangle avoid alg., where k = 2,4,8,16
//testing functions are: approx diam, page rank, reachability, shortest path, cc
void suite_test_triangle_top_k(vector<string> filenames)
{
    ofstream outFile;
    outFile.open("Triangle Top K Test Results.txt");
    for(auto it = filenames.begin(); it != filenames.end(); ++it)
    {
        Graph g;
        if (adjacency_list_read(g, it->c_str()))
        {
            //set up pairs for reachability and shortest path, and page rank
            std::vector<double> pageRankVector;
            page_rank(g, pageRankVector);
            unsigned n = num_vertices(g);
            int reachabilityTestSize = 1000;
            int loopsize = 0;
            std::multimap<Vertex, Vertex> VertexPairMap;
            while(VertexPairMap.size() < reachabilityTestSize)
            {
                if (loopsize == 1000000) {cout << "Long Loop for pairs."<< endl; break;}
                Vertex v = vertex(rand() % n, g);
                Vertex u = vertex(rand() % n, g);
                if(reachability(g, v, u)) VertexPairMap.emplace(v, u);
                ++loopsize;
            }
            std::map<pair<Vertex, Vertex>, unsigned> distanceMap;
            for(auto pairIt = VertexPairMap.begin(); pairIt != VertexPairMap.end(); ++pairIt)
            {
                distanceMap.emplace(*pairIt, graph_distance(g, pairIt->first, pairIt->second));
            }
            //testing original graph
            outFile << *it << endl;
            testing_funcs(g, outFile, VertexPairMap, distanceMap, pageRankVector, true);
            outFile << "Reachability Testing Pairs: ";
            for(auto it = VertexPairMap.begin(); it != VertexPairMap.end(); ++it)
            {
                outFile << "(" << it->first << ", " << it->second << ") ";
            }
            outFile << endl << endl;

            //testing reduced graphs
            for(int i = 1; i < 5; ++i)
            {
                int k = pow(2, i);
                Graph h;
                graph_reduction_triangle_avoid(g, h, k);
                outFile << "Keep triangle top " << k << " neighbours." << endl;
                testing_funcs(h, outFile, VertexPairMap, distanceMap, pageRankVector);
//                betweenness_print(g, h, outFile);
            }
        }


    }
    outFile.close();


}


//uses teh testing_func test suite to computete data on a bunch of graphs, and their reduced versions
//the reduced versions are computed with combined spanning trees
//testing functions are: approx diam, page rank, reachability, shortest path, cc
void suite_test_spanning_tree(vector<string>filenames)
{
    ofstream outFile;
    outFile.open("Spanning Tree Test Results.txt");

    for(auto it = filenames.begin(); it != filenames.end(); ++it)
    {
        Graph g;
        if(adjacency_list_read(g, it->c_str()))
        {
            //set up pairs for reachability and shortest path, and page rank
            std::vector<double> pageRankVector;
            page_rank(g, pageRankVector);
            unsigned n = num_vertices(g);
            int reachabilityTestSize = 1000;
            int loopsize = 0;
            std::multimap<Vertex, Vertex> VertexPairMap;
            while(VertexPairMap.size() < reachabilityTestSize)
            {
                if (loopsize == 1000000) {cout << "Long Loop for pairs."<< endl; break;}
                Vertex v = vertex(rand() % n, g);
                Vertex u = vertex(rand() % n, g);
                if(reachability(g, v, u)) VertexPairMap.emplace(v, u);
                ++loopsize;
            }
            std::map<pair<Vertex, Vertex>, unsigned> distanceMap;
            for(auto pairIt = VertexPairMap.begin(); pairIt != VertexPairMap.end(); ++pairIt)
            {
                distanceMap.emplace(*pairIt, graph_distance(g, pairIt->first, pairIt->second));
            }
            //testing original graph
            outFile << *it << endl;
            testing_funcs(g, outFile, VertexPairMap, distanceMap, pageRankVector, true);
            outFile << "Reachability Testing Pairs: ";
            for(auto it = VertexPairMap.begin(); it != VertexPairMap.end(); ++it)
            {
                outFile << "(" << it->first << ", " << it->second << ") ";
            }
            outFile << endl << endl;

            //testing reduced graphs
            //start the spanning trees with the vertices in the middle when sorted by degree
            for(int i = 0; i < 2; ++i)
            {
                //create reduced graph before testing
                int k = i * 2 + 3;

                //sort vertices by degree
                std::multimap<unsigned, Vertex> vertexDegreeMap;
                for(auto vi = boost::vertices(g).first; vi != boost::vertices(g).second; ++vi)
                {
                    vertexDegreeMap.emplace(boost::degree(*vi, g), *vi);
                }
                //pick the vertices as roots
                std::vector<Vertex> roots;
                for(int j = 0; j < k; ++j)
                {
                    auto it = vertexDegreeMap.begin();
                    //advance(it, (vertexDegreeMap.size() / 2) - i - 1 + j); //pick the ones in the middle
                    advance(it, vertexDegreeMap.size() - 1 - j); //largest degree
                    roots.push_back(it->second);
                }
                for(auto it = roots.begin(); it != roots.end(); ++it)
                {
                    cout << *it << " ";
                }
                cout <<endl;
                //actually create reduced graph
                Graph h;
                graph_reduction_high_degree_tree(g, h, roots);
                outFile << "Spanning tree with middle " << k << " vertices as roots." << endl;
                //t est reduced graph
                testing_funcs(h, outFile, VertexPairMap, distanceMap, pageRankVector);
//                betweenness_print(g, h, outFile);
            }
        }
    }

}

//uses teh testing_func test suite to computete data on a bunch of graphs, and their reduced versions
//the reduced versions are computed with highest degree neighbours, spanning tree, triangle avoid, and proportional neighbour alg
//testing functions are: approx diam, page rank, reachability, shortest path, cc
void suite_test(vector<string> filenames)
{
    ofstream outFile;
    outFile.open("Suite Mode File.txt");

    for(auto it = filenames.begin(); it != filenames.end(); ++it)
    {
        Graph g;

        if (adjacency_list_read(g, it->c_str()))
        {
            //set up pairs for reachability and shortest path, and page rank
            std::vector<double> pageRankVector;
            page_rank(g, pageRankVector);
            unsigned n = num_vertices(g);
            int reachabilityTestSize = 1000;
            int loopsize = 0;
            std::multimap<Vertex, Vertex> VertexPairMap;
            while(VertexPairMap.size() < reachabilityTestSize)
            {
                if (loopsize == 1000000) {cout << "Long Loop for pairs."<< endl; break;}
                Vertex v = vertex(rand() % n, g);
                Vertex u = vertex(rand() % n, g);
                if(reachability(g, v, u)) VertexPairMap.emplace(v, u);
                ++loopsize;
            }

            outFile << *it << endl;
            std::map<pair<Vertex, Vertex>, unsigned> distanceMap;
            for(auto pairIt = VertexPairMap.begin(); pairIt != VertexPairMap.end(); ++pairIt)
            {
                distanceMap.emplace(*pairIt, graph_distance(g, pairIt->first, pairIt->second));
            }
            //test original
            //testing_funcs(g, outFile, VertexPairMap, distanceMap, pageRankVector, true);
            outFile << "Reachability Testing Pairs: ";
            for(auto it = VertexPairMap.begin(); it != VertexPairMap.end(); ++it)
            {
                outFile << "(" << it->first << ", " << it->second << ") ";
            }
            outFile << endl << endl;
/*
            //graph reduction 1
            Graph* h1 = new Graph;
            graph_reduction(g, *h1, 8);

            outFile << "Reduction 1: keep top 8 neighbours" << endl;
            testing_funcs(*h1, outFile, VertexPairMap, distanceMap, pageRankVector);
            delete h1;
*/
            //graph reduction 2
            Graph* h2 = new Graph;
            graph_reduction_percentage(g, *h2, median_cutoff(g), 4);

            outFile << "Reduction 2: keep 1 of 4 neighbours (cutoff 8)" << endl;
            testing_funcs(*h2, outFile, VertexPairMap, distanceMap, pageRankVector);
            delete h2;
/*
            //graph reduction 3
            Graph* h3 = new Graph;
            graph_reduction_spanning_tree(g, *h3, high_degree_vertices(g, 5));

            outFile << "Reduction 3: spanning tree of top 5 deg. vertices" << endl;
            testing_funcs(*h3, outFile, VertexPairMap, distanceMap, pageRankVector);
            delete h3;

            //graph reduction 4
            Graph* h4 = new Graph;
            graph_reduction_triangle_avoid(g, *h4, 5);

            outFile << "Reduction 4: keep top 5 deg neighbours, prioritizing non triangles" << endl;
            testing_funcs(*h4, outFile, VertexPairMap, distanceMap, pageRankVector);
            delete h4;
*/
        }


    }
    outFile.close();
}

