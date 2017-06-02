#ifndef SUITE_TEST_HPP_INCLUDED
#define SUITE_TEST_HPP_INCLUDED

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

using namespace std;

void testing_funcs(const Graph &g, ofstream& outFile)
{

    outFile << "Edges: " << boost::num_edges(g) << endl;
    //outFile << "Exact Diameter: " << simple_graph_diameter(g) << endl;
    outFile << "Approx Diamter: " << approx_graph_diameter(g) << endl;

    //cc
    std::vector<int> connectedCompCount = connected_comp(g);
    outFile << "Connected Components: " << connectedCompCount.at(0) << endl;

    outFile << "Largest Component Sizes: ";
    for(std::vector<int>::iterator i = connectedCompCount.begin() + 1; i != connectedCompCount.end(); ++i)
    {
        outFile << " " << *i;
    }
    outFile << endl;

    //page rank
    std::vector<double> pageRanks;
    page_rank(g, pageRanks);

    map<double, Vertex> pageRankMap;
    for(unsigned i = 0; i < boost::num_vertices(g); ++i)
    {
        pageRankMap.emplace(pageRanks.at(i), i);
    }
    outFile << "Top 15 Page Rank vertices:";
    for (int i = 0; i < 15 && i < boost::num_vertices(g); ++i)
    {
        auto it = pageRankMap.begin();
        advance(it, i);
        outFile << " " << it->second;
    }
    outFile << endl;

}


void suite_test(vector<string> filenames)
{
    ofstream outFile;
    outFile.open("Suite Mode File.txt");

    for(auto it = filenames.begin(); it != filenames.end(); ++it)
    {
        Graph g;

        if (adjacency_list_read(g, it->c_str()))
        {
            unsigned n = num_vertices(g);
            int reachabilityTestSize = 100;
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
            testing_funcs(g, outFile);
            outFile << "Reachability Testing Pairs: ";
            for(auto it = VertexPairMap.begin(); it != VertexPairMap.end(); ++it)
            {
                outFile << "(" << it->first << ", " << it->second << ") ";
            }
            outFile << endl << endl;

            //graph reduction 1
            Graph* h1 = new Graph;
            graph_reduction(g, *h1, 5);

            outFile << "Reduction 1: keep top 5 neighbours" << endl;
            testing_funcs(*h1, outFile);
            outFile << "Reachability Testing Pairs: ";
            for(auto it = VertexPairMap.begin(); it != VertexPairMap.end(); ++it)
            {
                if(!reachability(*h1, it->first, it->second)) outFile << "(" << it->first << ", " << it->second << ") ";
            }
            outFile << endl << endl;
            delete h1;

            //graph reduction 2
            Graph* h2 = new Graph;
            graph_reduction_percentage(g, *h2);

            outFile << "Reduction 2: keep 1 of 4 neighbours (cutoff 10)" << endl;
            testing_funcs(*h2, outFile);
            outFile << "Reachability Testing Pairs: ";
            for(auto it = VertexPairMap.begin(); it != VertexPairMap.end(); ++it)
            {
                if(!reachability(*h2, it->first, it->second)) outFile << "(" << it->first << ", " << it->second << ") ";
            }
            outFile << endl << endl;
            delete h2;

            //graph reduction 3
            Graph* h3 = new Graph;
            graph_reduction_spanning_tree(g, *h3, high_degree_vertices(g, 5));

            outFile << "Reduction 3: spanning tree of top 5 deg. vertices" << endl;
            testing_funcs(*h3, outFile);
            outFile << "Reachability Testing Pairs: ";
            for(auto it = VertexPairMap.begin(); it != VertexPairMap.end(); ++it)
            {
                if(!reachability(*h3, it->first, it->second)) outFile << "(" << it->first << ", " << it->second << ") ";
            }
            outFile << endl << endl;
            delete h3;

            //graph reduction 4
            Graph* h4 = new Graph;
            graph_reduction_triangle_avoid(g, *h4, 5);

            outFile << "Reduction 4: keep top 5 deg neighbours, prioritizing non triangles" << endl;
            testing_funcs(*h4, outFile);
            outFile << "Reachability Testing Pairs: ";
            for(auto it = VertexPairMap.begin(); it != VertexPairMap.end(); ++it)
            {
                if(!reachability(*h4, it->first, it->second)) outFile << "(" << it->first << ", " << it->second << ") ";
            }
            outFile << endl << endl;
            delete h4;

        }


    }
    outFile.close();
}


#endif // SUITE_TEST_HPP_INCLUDED
