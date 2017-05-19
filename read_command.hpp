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
#include "betweenness_centrality.hpp"


using namespace std;

//reads cin for a command, prints to file
//expects the file to be already open, and closed elsewhere
//template expects Graph or DirectedGraph
template<class graph>
void readCommand(const graph &g, string filename)
{
    ofstream outFile;
    outFile.open("Graph Properties.txt", ios_base::out | ios_base::app );
    outFile << "File: " << filename << endl;

    bool b = true;
    while (b)
    {
        cout << "Select modes available: cc, adiam, ediam, etri, lcc, prank, ebc, abc (type exit to quit)" << endl;
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
            cout << "Select a vertex from " << boost::num_vertices(g) << " vertices. (-1 for average)" << endl;
            unsigned v;
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

            vector<double> pageRank;
            page_rank(g, pageRank);
            for(auto it = pageRank.begin(); it != pageRank.end(); ++it)
            {
                pRankFile << *it << endl;
            }
            pRankFile.close();
            outFile << "Page Rank computed." << endl;
        }
        else if (command == "aprank") //approx page rank, outputs to separate file
        {
            ofstream pRankFile;
            pRankFile.open("Approximate Page Rank.txt", ios_base::out | ios_base::app);
            pRankFile.precision(10);
            pRankFile << filename << endl;

            vector<double> pageRank;
            approx_page_rank(g, pageRank);
            for(auto it = pageRank.begin(); it != pageRank.end(); ++it)
            {
                pRankFile << *it << endl;
            }
            pRankFile.close();
            outFile << "Approx Page Rank computed." << endl;
        }
        else if (command == "abc") //aprox betwenness centrality
        {
            cout << "Select a vertex from " << boost::num_vertices(g) << "vertices." << endl;
            unsigned v;
            cin >> v;
            outFile.precision(8);
            outFile << "Approx Betweenness Centrality " << approx_betweenness_centrality(g, boost::vertex(v, g)) << endl;
            getline(cin, command); //to clear up the whitespace
        }
        else if (command == "ebc") //betweenness centrality, outputs to separate file
        {
            ofstream bcFile;
            bcFile.open("Betweenness Centrality.txt", ios_base::out | ios_base::app);
            bcFile.precision(4);
            bcFile << filename << endl;

            vector<float> centralityVector(boost::num_vertices(g), 0);
            betweenness_centrality(g, centralityVector);
            for(auto it = centralityVector.begin(); it != centralityVector.end(); ++it)
            {
                bcFile << *it << endl;
            }

            bcFile.close();
            outFile << "Page Rank computed." << endl;
        }
        else if (command == "adiam") outFile << "Approx Diameter: " << approx_graph_diameter(g) << endl;
        else if (command == "ediam") outFile << "Exact Diameter: " << simple_graph_diameter(g) << endl;
        else if (command == "etri") outFile << "Exact Triangle Count: "  << exact_triangle_count(g) << endl;
        else if (command == "vert") cout << "Num Vert: " << boost::num_vertices(g) << endl;
        else if (command == "edge") cout << "Num Edge: " << boost::num_edges(g) << endl;
        else if (command == "exit") break;

        else if (command == "test")
        {
            vector<float> centralityVector2(boost::num_vertices(g), 0);
            betweenness_centrality(g, centralityVector2);
            cout<<"Centralities:";
            for(auto it = centralityVector2.begin(); it != centralityVector2.end(); ++it)
            {
                cout << " " << *it;
            }
            cout<<endl;

            std::map<Vertex, float>centralityMap;
            boost::associative_property_map< std::map<Vertex, float> >centralityPropMap(centralityMap);

            boost::shared_array_property_map<double, boost::property_map<Graph, vertex_index_t>::const_type>
                centrality_map(num_vertices(g), get(boost::vertex_index, g));

            boost::brandes_betweenness_centrality(g, centralityPropMap);
            cout<<"Centralities:";
            for(auto it = centralityMap.begin(); it != centralityMap.end(); ++it)
            {
                cout << " " << it->second;
            }
            cout<<endl;

        }


        else cout << "Did not recognize command." << endl;
    }
    outFile.close();
}

#endif // READ_COMMAND_HPP_INCLUDED
