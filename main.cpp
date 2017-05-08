#include <iostream>
#include <fstream>
#include "adjacency_list_read.hpp"
#include <utility>
#include <boost/graph/graph_traits.hpp>
#include <string>
#include "bfs_print_visitor.hpp"
#include <boost/graph/breadth_first_search.hpp>
#include "edge_list_print.hpp"
#include "graph_diameter.hpp"
#include "connected_comp.hpp"
#include "triangle_count.hpp"
#include "local_cluster_coeff.hpp"
#include <iomanip>

using namespace std;

int main()
{
    string filename;// = "C:\\Users\\Eric\\Documents\\Vincent Work\\Graph data\\facebook_combined.txt";
    cout << "Enter Adjacency List Filename: (type exit to quit)" << endl;

    ofstream outFile;
    outFile.open("Graph Properties.txt");

    while (true)
    {
        getline(cin, filename);

        if (filename == "stop" || filename == "exit") break;
        if (filename == "default") filename = "C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\facebook_combined.txt";
        if (filename == "small") filename == "C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\small test.txt";

        Graph g;

        outFile << "File: " << filename << endl;

        if (adjacency_list_read(g, filename.c_str()))
        {
            cout << "Select modes available: cc, adiam, ediam, etri, lcc (type exit to quit)" << endl;

            while (true)
            {
                string command;
                getline(cin, command);

                if(command == "cc")
                {
                    std::vector<int> connectedCompCount = connected_comp(g);
                    outFile << "Connected Components: " << connectedCompCount.at(0) << endl;

                    outFile << "Largest Component Sizes:";
                    for(std::vector<int>::iterator i = connectedCompCount.begin() + 1; i != connectedCompCount.end(); ++i)
                    {
                        outFile << " " << *i;
                    }
                    outFile << endl;
                }
                else if (command == "lcc")
                {
                    cout << "Select a vertex from " << boost::num_vertices(g) << " vertices." << endl;
                    int v;
                    cin >> v;
                    outFile.precision(4);
                    outFile << "LCC of vertex " << v << ": " << local_cluster_coeff(v, g) << endl;
                    getline(cin, command); //to clear up the whitespace
                }
                else if (command == "adiam") outFile << "Approx Diameter :" << approx_graph_diameter(g) << endl;
                else if (command == "ediam") outFile << "Exact Diameter :" << simple_graph_diameter(g) << endl;
                else if (command == "etri") outFile << "Exact Triangle Count: "  << exact_triangle_count(g) << endl;
                else if (command == "vert") cout << "Num Vert: " << boost::num_vertices(g) << endl;
                else if (command == "edge") cout << "Num Edge: " << boost::num_edges(g) << endl;
                else if (command == "exit") break;

                else cout << "Did not recognize command." << endl;
                cout << "Select modes available: cc, adiam, ediam, etri, lcc (type exit to quit)" << endl;
                }
            }
        cout << endl << "Enter Adjacency List Filename: (type exit to quit)" <<endl;
    }

    outFile.close();

    return 0;
}
