#ifndef READ_COMMAND_HPP_INCLUDED
#define READ_COMMAND_HPP_INCLUDED

#include <iostream>
#include "common.h"
#include <fstream>
#include <string>
#include "adjacency_list_read.hpp"
#include "edge_list_print.hpp"
#include "graph_diameter.hpp"
#include "connected_comp.hpp"
#include "triangle_count.hpp"
#include "local_cluster_coeff.hpp"
#include <iomanip>
#include <vector>
#include "page_rank.hpp"

using namespace std;

//reads cin for a command, prints to file
//expects the file to be already open, and closed elsewhere
//template expects Graph or DirectedGraph
template<class graph>
void readCommand(const graph &g, string filename)
{
    ofstream outFile;
    outFile.open("Graph Properties.txt");
    outFile << "File: " << filename << endl;

    bool b = true;
    while (b)
    {
        cout << "Select modes available: cc, adiam, ediam, etri, lcc, prank (type exit to quit)" << endl;
        string command;
        getline(cin, command);

        if(command == "cc") //connected components
        {
            std::vector<int> connectedCompCount = connected_comp(g);
            outFile << "Connected Components: " << connectedCompCount.at(0) << endl;

            outFile << "Largest Component Sizes: ";
            for(std::vector<int>::iterator i = connectedCompCount.begin() + 1; i != connectedCompCount.end(); ++i)
            {
                outFile << " " << *i;
            }
            outFile << endl;
        }
        else if (command == "lcc") //local cluster coeff
        {
            cout << "Select a vertex from " << boost::num_vertices(g) << " vertices." << endl;
            int v;
            cin >> v;
            outFile.precision(4);
            if (v == -1) outFile << "Average LCC: " << average_cluster_coeff(g) << endl;
            else outFile << "LCC of vertex " << v << ": " << local_cluster_coeff(v, g) << endl;
            getline(cin, command); //to clear up the whitespace
        }
        else if (command == "prank") //page rank, outputs to separate file
        {
            ofstream pRankFile;
            pRankFile.open("Page Rank.txt", ios_base::out | ios_base::app);
            pRankFile.precision(10);
            pRankFile << filename << endl;
            vector<double> pageRank = page_rank(g);
            for(auto it = pageRank.begin(); it != pageRank.end(); ++it)
            {
                pRankFile << *it << endl;
            }
            pRankFile.close();
            outFile << "Page Rank computed." << endl;
        }
        else if (command == "adiam") outFile << "Approx Diameter: " << approx_graph_diameter(g) << endl;
        else if (command == "ediam") outFile << "Exact Diameter: " << simple_graph_diameter(g) << endl;
        else if (command == "etri") outFile << "Exact Triangle Count: "  << exact_triangle_count(g) << endl;
        else if (command == "vert") cout << "Num Vert: " << boost::num_vertices(g) << endl;
        else if (command == "edge") cout << "Num Edge: " << boost::num_edges(g) << endl;
        else if (command == "exit") break;

        else cout << "Did not recognize command." << endl;
    }
    outFile.close();
}

#endif // READ_COMMAND_HPP_INCLUDED
