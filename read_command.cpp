#include <iostream>
#include <ctime>
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
#include "graph_reduction.hpp"
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "shortest_path.hpp"
#include "onion_decomp.hpp"
#include "read_command.hpp"


using namespace std;

//reads cin for a command, prints to file
//expects the file to be already open, and closed elsewhere
//template expects Graph or DirectedGraph
void readCommand(const Graph &g, string filename)
{
    ofstream outFile;
    string outFileName = filename.substr(0, filename.length()-4);
    outFile.open(outFileName + "-graph-properties.txt", ios_base::out | ios_base::app );
    outFile << "File: " << filename << endl;
    time_t startTime;
    time_t endTime;

    bool b = true;
    while (b)
    {
        cout << "Select modes available: abc, adiam, aprank, bbc, bc, cc, dist, ediam, emdiam, edge, etri, kdiam, lcc, od, prank, vert, reduce, reducetri, reducetree, reducetreetop, reducepercent (type exit to quit)" << endl;
        string command;
        getline(cin, command);

        if(command == "cc") //connected components
        {
            std::vector<unsigned> connectedCompCount = connected_comp(g);
            outFile << "Connected Components: " << connectedCompCount.at(0) << endl;

            outFile << "Largest Component Sizes: ";
            for(std::vector<unsigned>::iterator i = connectedCompCount.begin() + 1; i != connectedCompCount.end(); ++i)
            {
                outFile << " " << *i;
            }
            outFile << endl;

        }
        else if (command == "lcc") //local cluster coeff
        {
            cout << "Select a vertex from " << boost::num_vertices(g) << " vertices. (-1 for average)" << endl;
            long v;
            cin >> v;
            outFile.precision(4);
            if (v == -1) outFile << "Average LCC: " << average_cluster_coeff(g) << endl;
            else outFile << "LCC of vertex " << v << ": " << local_cluster_coeff(v, g) << endl;

            cin.ignore(); //to clear up the whitespace
        }
        else if (command == "prank") //page rank, outputs to separate file
        {
            ofstream pRankFile;
            pRankFile.open(outFileName + "-page-rank.txt", ios_base::out | ios_base::app);
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
            pRankFile.open(outFileName + "-approximate-page-rank.txt", ios_base::out | ios_base::app);
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
            cin.ignore();
            outFile.precision(8);
            outFile << "Approx Betweenness Centrality " << approx_betweenness_centrality(g, boost::vertex(v, g)) << endl;
        }
        else if (command == "bc") //betweenness centrality, outputs to separate file
        {
            ofstream bcFile;
            bcFile.open(outFileName + "-betweenness-centrality.txt", ios_base::out | ios_base::app);
            bcFile.precision(4);
            bcFile << filename << endl;

            vector<float> centralityVector(boost::num_vertices(g), 0);
            betweenness_centrality(g, centralityVector);
            for(auto it = centralityVector.begin(); it != centralityVector.end(); ++it)
            {
                bcFile << *it << endl;
            }

            bcFile.close();
            outFile << "Betweenness Centrality computed." << endl;
        }
        else if (command == "bbc") //brandes bc
        {
            std::map<Vertex, float>centralityMap;
            boost::associative_property_map< std::map<Vertex, float> >centralityPropMap(centralityMap);

            boost::brandes_betweenness_centrality(g, centralityPropMap);

            ofstream bcFile;
            bcFile.open(outFileName + "-brandes-BC.txt", ios_base::out | ios_base::app);
            bcFile.precision(4);
            bcFile << filename << endl;

            for(auto it = centralityMap.begin(); it != centralityMap.end(); ++it)
            {
                bcFile << " " << it->second;
            }
            bcFile.close();
            outFile << "Brandes BC computed." << endl;
        }
        else if (command == "reduce") //create reduced graph
        {
            Graph h;
            int x;
            cout << "Enter # Vertices to keep." << endl;
            cin >> x;
            cin.ignore();
            graph_reduction(g, h, x);
            edge_list_print_file(h, outFileName + "-reduced-graph.txt");
        }
        else if (command == "reduce5") //create reduced graph
        {
            Graph h;
            graph_reduction(g, h, 5);
            edge_list_print_file(h, outFileName + "-reduced-graph5.txt");
        }
        else if (command == "reduce5r") //create reduced graph
        {
            Graph h;
            graph_reduction_reverse(g, h, 5);
            edge_list_print_file(h, outFileName + "-reduced-graph5.txt");
        }
        else if (command == "reducetree")
        {
            Graph h;
            cout << "Select Vertices as roots. (-1 to stop)" <<endl;
            vector<Vertex> roots;
            long x = 0;
            while(x != -1)
            {
                cin >> x;
                if(x >= 0) roots.push_back(x);
            }
            cin.ignore();
            cout << "Roots: ";
            for(auto it = roots.begin(); it != roots.end(); ++it)
            {
                cout << *it << " " << endl;
            }
            graph_reduction_spanning_tree(g, h, roots);
            edge_list_print_file(h, outFileName + "-reduced-graph Tree.txt");
        }
        else if (command == "reducetreetop")
        {
            Graph h;
            cout << "Select # Vertices as roots. " << endl;
            int x;
            cin >> x;
            cin.ignore();
            graph_reduction_spanning_tree(g, h, high_degree_vertices(g, x));
            edge_list_print_file(h, outFileName + "-reduced-graph-tree2.txt");

        }
        else if (command == "reducetri")
        {
            Graph h;
            cout << "Select # Vertices to keep. " << endl;
            int x;
            cin >> x;
            cin.ignore();
            graph_reduction_triangle_avoid(g, h, x);
            edge_list_print_file(h, outFileName + "-reduced-graph-triangle.txt");
        }
        else if (command == "reducepercent")
        {
            Graph h;
            cout << "Select Proportion of vertices to keep if past median" << endl;
            int x;
            cin >> x;
            cin.ignore();
            graph_reduction_percentage(g, h, median_cutoff(g), x);
            edge_list_print_file(h, outFileName + "-reduced-graph-proportion.txt");
        }
        else if (command == "dist")
        {
            Vertex v;
            Vertex u;
            cout << "Select first vertex" << endl;
            cin >> v;
            cout << "Select second vertex" << endl;
            cin >> u;
            outFile << "Distance from " << v << " to " << u << ": " << graph_distance(g, v, u) << endl;
            cin.ignore(); //to clear up the whitespace

        }
        else if (command == "od") //onion decomp
        {
            std::vector<unsigned> onionLayerCount;
            outFile << "Layers: " << onion_decomp(g, onionLayerCount) << endl;
            outFile << "Layer Count: ";
            for(auto it = onionLayerCount.begin(); it != onionLayerCount.end(); ++it)
            {
                outFile << *it << " ";
            }
            outFile<< endl;
        }
        else if (command == "kdiam")
        {
            pair<vertices_size_type, vertices_size_type> diameterBounds = khaled_approx_diameter(g);
            outFile << "Khaled's Approx Diameter Bounds: " << diameterBounds.first << ", " << diameterBounds.second << endl;
        }
        else if (command == "adiam"){
            startTime = time(NULL);

            outFile << "Approx Diameter: " << approx_graph_diameter(g) << endl;
            cout << "Approx Diameter: " << approx_graph_diameter(g) << endl;

            endTime = time(NULL);


            cout<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            outFile<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;


            startTime = time(NULL);

            outFile << "Approx Diameter, Multithreaded: " << approx_graph_diameter_multithread(g) << endl;
            cout << "Approx Diameter, Multithreaded: " << approx_graph_diameter_multithread(g) << endl;

            endTime = time(NULL);

            cout<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            outFile<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
        }
        else if (command == "emdiam") outFile << "Exact Diameter (M): " << memory_graph_diamter(g) << endl;
        else if (command == "ediam") outFile << "Exact Diameter: " << simple_graph_diameter(g) << endl;
        else if (command == "etri") outFile << "Exact Triangle Count: "  << exact_triangle_count(g) << endl;
        else if (command == "vert") cout << "Num Vert: " << boost::num_vertices(g) << endl;
        else if (command == "edge") cout << "Num Edge: " << boost::num_edges(g) << endl;
        else if (command == "exit") break;


        else if (command == "test")
        {
            cout << boost::edge(0,0, g).second <<endl;
        }

        else cout << "Did not recognize command." << endl;
    }
    outFile.close();
}
