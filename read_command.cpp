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
#include "degrees.hpp"

#ifdef TBB
#include "tbb/tbb.h"
#endif


using namespace std;


//reads cin for a command, prints to file
//expects the file to be already open, and closed elsewhere
//template expects Graph or DirectedGraph
void readCommand(const Graph &g, string filename)
{
    //for consistent filenames between multithread only and single thread only executables
    //and compatibility with GraphDisplay
    #if defined(TBB) && defined(SEQ)
        string multithread_file_ending="-multithread.txt";
    #else
        string multithread_file_ending=".txt";
    #endif

    ofstream outFile;
    string outFileName = filename.substr(0, filename.length()-4);
    outFile.open(outFileName + "-graph-properties.txt", ios_base::out | ios_base::app );
    outFile << "File: " << filename << endl;
    time_t startTime;
    time_t endTime;

    #ifdef TBB
    cout<<"Number of threads available: "<<tbb::task_scheduler_init::default_num_threads()<<endl;
    #endif

    bool b = true;
    while (b)
    {
        cout << "Select modes available: abc, adiam, aprank, bbc, bc, cc, dist, degrees, ediam, emdiam, edge, etri, kdiam, lcc, od, prank, vert, reduce, reducetri, reducetree, reducetreetop, reducehighdegree, reducehighdegreetop, reducepercent (type exit to quit)" << endl;
        string command;
        getline(cin, command);

        if(command == "cc") //connected components
        {
            vector<unsigned> connectedCompCount;

            #ifdef SEQ
            startTime = time(NULL);
            connectedCompCount = connected_comp(g);
            endTime = time(NULL);
            outFile << "Connected Components: " << connectedCompCount.at(0) << endl;
            cout << "Connected Components: " << connectedCompCount.at(0) << endl;
            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;

            outFile << "Largest Component Sizes: ";
            for(std::vector<unsigned>::iterator i = connectedCompCount.begin() + 1; i != connectedCompCount.end(); ++i)
            {
                outFile << " " << *i;
            }
            outFile << endl;
            #endif


            #ifdef TBB
            startTime = time(NULL);
            connectedCompCount = connected_comp_multithread(g);
            endTime = time(NULL);
            outFile << "Connected Components, Multithreaded: " << connectedCompCount.at(0) << endl;
            cout << "Connected Components, Multithreaded: " << connectedCompCount.at(0) << endl;
            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;

            outFile << "Largest Component Sizes, Multithreaded: ";
            for(std::vector<unsigned>::iterator i = connectedCompCount.begin() + 1; i != connectedCompCount.end(); ++i)
            {
                outFile << " " << *i;
            }
            outFile << endl;
            #endif

        }
        else if (command == "lcc") //local cluster coeff
        {
            cout << "Select a vertex from " << boost::num_vertices(g) << " vertices. (-1 for average)" << endl;
            long v;
            double lcc;
            cin >> v;
            outFile.precision(4);

            if (v == -1){
                #ifdef SEQ
                startTime = time(NULL);
                lcc = average_cluster_coeff(g);
                endTime = time(NULL);

                outFile << "Average LCC: " << lcc << endl;
                cout<< "Average LCC: " << lcc << endl;
                cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
                #endif
                #ifdef TBB
                // startTime = time(NULL);
                // lcc = average_cluster_coeff_multithread(g);
                // endTime = time(NULL);

                // outFile << "Average LCC, Multithreaded: " << lcc << endl;
                // cout<< "Average LCC, Multithreaded: " << lcc << endl;
                // cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;


                startTime = time(NULL);
                lcc = average_cluster_coeff_multithread_reduce(g);
                endTime = time(NULL);

                outFile << "Average LCC, reduce: " << lcc << endl;
                cout<< "Average LCC, reduce: " << lcc << endl;
                cerr<<"Time elapsed, reduce: "<<difftime(endTime, startTime)<<endl;
                #endif
            }
            else{
                #ifdef SEQ
                startTime = time(NULL);
                lcc = local_cluster_coeff(v, g);
                endTime = time(NULL);

                outFile << "LCC of vertex " << v << ": " << lcc << endl;
                cout<< "LCC of vertex " << v << ": " << lcc << endl;
                cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
                #endif
                #ifdef TBB
                startTime = time(NULL);
                lcc = local_cluster_coeff_multithread(v, g);
                endTime = time(NULL);

                outFile << "LCC of vertex " << v << ", Multithreaded: " << lcc << endl;
                cout << "LCC of vertex " << v << ", Multithreaded: " << lcc << endl;
                cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
                #endif
            }

            cin.ignore(); //to clear up the whitespace
        }
        else if (command == "prank") //page rank, outputs to separate file
        {
            vector<double> pageRank;
            ofstream pRankFile;

            #ifdef TBB
            startTime = time(NULL);
            page_rank_multithread(g, pageRank);
            endTime = time(NULL);
            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;

            pRankFile.open(outFileName + "-page-rank" + multithread_file_ending, ios_base::out | ios_base::app);
            pRankFile.precision(10);
            pRankFile << filename << endl;
            for(auto it = pageRank.begin(); it != pageRank.end(); ++it)
            {
                pRankFile << *it << endl;
            }
            pRankFile.close();
            #endif
            #ifdef SEQ
            startTime = time(NULL);
            page_rank(g, pageRank);
            endTime = time(NULL);
            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;

            pRankFile.open(outFileName + "-page-rank.txt", ios_base::out | ios_base::app);
            pRankFile.precision(10);
            pRankFile << filename << endl;
            for(auto it = pageRank.begin(); it != pageRank.end(); ++it)
            {
                pRankFile << *it << endl;
            }
            pRankFile.close();
            #endif
            cout << "Page Rank computed." << endl;
        }
        else if (command == "aprank") //approx page rank, outputs to separate file
        {
            vector<double> pageRank;
            ofstream pRankFile;

            #ifdef TBB
            startTime = time(NULL);
            approx_page_rank_multithread(g, pageRank);
            endTime = time(NULL);
            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;

            pRankFile.open(outFileName + "-approximate-page-rank" + multithread_file_ending, ios_base::out | ios_base::app);
            pRankFile.precision(10);
            pRankFile << filename << endl;
            for(auto it = pageRank.begin(); it != pageRank.end(); ++it)
            {
                pRankFile << *it << endl;
            }
            pRankFile.close();
            #endif
            #ifdef SEQ
            startTime = time(NULL);
            approx_page_rank(g, pageRank);
            endTime = time(NULL);
            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;

            pRankFile.open(outFileName + "-approximate-page-rank.txt", ios_base::out | ios_base::app);
            pRankFile.precision(10);
            pRankFile << filename << endl;
            for(auto it = pageRank.begin(); it != pageRank.end(); ++it)
            {
                pRankFile << *it << endl;
            }
            pRankFile.close();
            #endif
            cout << "Approximate Page Rank computed." << endl;
        }
        else if (command == "abc") //aprox betwenness centrality
        {
            cout << "Select a vertex from " << boost::num_vertices(g) << "vertices." << endl;
            unsigned v;
            cin >> v;
            cin.ignore();
            outFile.precision(8);

            float abc;

            #ifdef TBB
            startTime = time(NULL);
            abc = approx_betweenness_centrality_multithread(g, boost::vertex(v, g));
            endTime = time(NULL);
            outFile << "Approx Betweenness Centrality, Multithreaded " << abc<< endl;
            cout << "Approx Betweenness Centrality, Multithreaded " << abc << endl;
            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            #endif
            #ifdef SEQ
            startTime = time(NULL);
            abc = approx_betweenness_centrality(g, boost::vertex(v, g));
            endTime = time(NULL);
            outFile << "Approx Betweenness Centrality " << abc<< endl;
            cout << "Approx Betweenness Centrality " << abc << endl;
            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            #endif
        }
        else if (command == "bc") //betweenness centrality, outputs to separate file
        {
            ofstream bcFile;
            bcFile.open(outFileName + "-betweenness-centrality.txt", ios_base::out | ios_base::app);
            bcFile.precision(4);
            bcFile << filename << endl;

            vector<float> centralityVector(boost::num_vertices(g), 0);

            #ifdef TBB
            startTime = time(NULL);
            betweenness_centrality_multithread(g, centralityVector);
            endTime = time(NULL);

            cerr<<"Time elapsed, Multithreaded, double parallel_for: "<<difftime(endTime, startTime)<<endl;


            // startTime = time(NULL);
            // betweenness_centrality_multithread_3parallel(g, centralityVector);
            // endTime = time(NULL);

            // cerr<<"Time elapsed, Multithreaded, triple parallel_for: "<<difftime(endTime, startTime)<<endl;
            #endif
            #ifdef SEQ
            startTime = time(NULL);
            betweenness_centrality(g, centralityVector);
            endTime = time(NULL);

            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            #endif
            for(auto it = centralityVector.begin(); it != centralityVector.end(); ++it)
            {
                bcFile << *it << endl;
            }

            bcFile.close();
            cout<< "Betweenness Centrality computed." << endl;
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
            Graph h1,h2;
            int x;
            cout << "Enter # Vertices to keep." << endl;
            cin >> x;
            cin.ignore();
            #ifdef SEQ
            startTime = time(NULL);
            graph_reduction(g, h1, x);
            endTime = time(NULL);
            edge_list_print_file(h1, outFileName + "-reduced-graph.txt");
            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            #endif
            #ifdef TBB
            startTime = time(NULL);
            graph_reduction_multithread(g, h2, x);
            endTime = time(NULL);
            edge_list_print_file(h2, outFileName + "-reduced-graph" + multithread_file_ending);
            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            #endif
        }
        else if (command == "reducetree")
        {
            Graph h1, h2;
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
            #ifdef SEQ
            startTime = time(NULL);
            graph_reduction_spanning_tree(g, h1, roots);
            endTime = time(NULL);
            edge_list_print_file(h1, outFileName + "-reduced-graph-tree.txt");
            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            #endif
            #ifdef TBB
            startTime = time(NULL);
            graph_reduction_spanning_tree_multithread(g, h2, roots);
            endTime = time(NULL);
            edge_list_print_file(h2, outFileName + "-reduced-graph-tree" + multithread_file_ending);
            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            #endif
        }
        else if (command == "reducetreetop")
        {
            Graph h1,h2;
            cout << "Select # Vertices as roots. " << endl;
            int x;
            cin >> x;
            cin.ignore();
            #ifdef SEQ
            startTime = time(NULL);
            graph_reduction_spanning_tree(g, h1, high_degree_vertices(g, x));
            endTime = time(NULL);
            edge_list_print_file(h1, outFileName + "-reduced-graph-tree2.txt");
            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            #endif
            #ifdef TBB
            startTime = time(NULL);
            graph_reduction_spanning_tree_multithread(g, h2, high_degree_vertices_multithread(g, x));
            endTime = time(NULL);
            edge_list_print_file(h2, outFileName + "-reduced-graph-tree2" + multithread_file_ending);
            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            #endif
        }
        else if (command == "reducehighdegree")
        {
            Graph h1, h2;
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
            #ifdef SEQ
            startTime = time(NULL);
            graph_reduction_high_degree_tree(g, h1, roots);
            endTime = time(NULL);
            edge_list_print_file(h1, outFileName + "-reduced-graph-high-degree.txt");
            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            #endif

            #ifdef TBB
            startTime = time(NULL);
            graph_reduction_high_degree_tree_multithread(g, h2, roots);
            endTime = time(NULL);
            edge_list_print_file(h2, outFileName + "-reduced-graph-high-degree" + multithread_file_ending);
            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            #endif
        }
        else if (command == "reducehighdegreetop")
        {
            Graph h1,h2;
            cout << "Select # Vertices as roots. " << endl;
            int x;
            cin >> x;
            cin.ignore();
            #ifdef SEQ
            startTime = time(NULL);
            graph_reduction_high_degree_tree(g, h1, high_degree_vertices(g, x));
            endTime = time(NULL);
            edge_list_print_file(h1, outFileName + "-reduced-graph-high-degree2.txt");
            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            #endif

            #ifdef TBB
            startTime = time(NULL);
            graph_reduction_high_degree_tree_multithread(g, h2, high_degree_vertices_multithread(g, x));
            endTime = time(NULL);
            edge_list_print_file(h2, outFileName + "-reduced-graph-high-degree2" + multithread_file_ending);
            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            #endif
        }
        else if (command == "reducetri")
        {
            Graph h1, h2;
            cout << "Select # Vertices to keep. " << endl;
            int x;
            cin >> x;
            cin.ignore();

            #ifdef TBB
            startTime = time(NULL);
            graph_reduction_triangle_avoid_multithread(g, h2, x);
            endTime = time(NULL);
            edge_list_print_file(h2, outFileName + "-reduced-graph-triangle" + multithread_file_ending);
            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            #endif
            #ifdef SEQ
            startTime = time(NULL);
            graph_reduction_triangle_avoid(g, h1, x);
            endTime = time(NULL);
            edge_list_print_file(h1, outFileName + "-reduced-graph-triangle.txt");
            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            #endif
        }
        else if (command == "reducepercent")
        {
            Graph h1, h2;
            cout << "Select Proportion of vertices to keep if past median" << endl;
            int x;
            cin >> x;
            cin.ignore();

            #ifdef SEQ
            startTime = time(NULL);
            graph_reduction_percentage(g, h1, median_cutoff(g), x);
            endTime = time(NULL);
            edge_list_print_file(h1, outFileName + "-reduced-graph-proportion.txt");
            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            #endif

            #ifdef TBB
            startTime = time(NULL);
            graph_reduction_percentage_multithread(g, h2, median_cutoff(g), x);
            endTime = time(NULL);
            edge_list_print_file(h2, outFileName + "-reduced-graph-proportion" + multithread_file_ending);
            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            #endif
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

            vertices_size_type adiam;

            #ifdef SEQ
            startTime = time(NULL);
            adiam = approx_graph_diameter(g);
            endTime = time(NULL);
            outFile << "Approx Diameter: " << adiam << endl;
            cout << "Approx Diameter: " << adiam << endl;
            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            outFile<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            #endif

            #ifdef TBB
            startTime = time(NULL);
            adiam = approx_graph_diameter_multithread(g);
            endTime = time(NULL);
            outFile << "Approx Diameter, Multithreaded: " << adiam  << endl;
            cout << "Approx Diameter, Multithreaded: " << adiam << endl;
            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            outFile<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            #endif
        }
        else if (command == "emdiam") outFile << "Exact Diameter (M): " << memory_graph_diamter(g) << endl;
        else if (command == "ediam") outFile << "Exact Diameter: " << simple_graph_diameter(g) << endl;
        else if (command == "etri"){

            unsigned long etri;

            #ifdef TBB
            startTime = time(NULL);
            etri = exact_triangle_count_multithread(g);
            endTime = time(NULL);

            outFile << "Exact Triangle Count, Multithreaded: " << etri  << endl;
            cout << "Exact Triangle Count, Multithreaded: " << etri << endl;

            cerr<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            outFile<<"Time elapsed, Multithreaded: "<<difftime(endTime, startTime)<<endl;
            #endif
            #ifdef SEQ
            startTime = time(NULL);
            etri = exact_triangle_count(g);
            endTime = time(NULL);

            outFile << "Exact Triangle Count: "  << etri << endl;
            cout << "Exact Triangle Count: "  << etri << endl;

            cerr<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            outFile<<"Time elapsed: "<<difftime(endTime, startTime)<<endl;
            #endif
        }
        else if (command == "vert"){
            cout << "Num Vert: " << boost::num_vertices(g) << endl;
            outFile << "Num Vert: " << boost::num_vertices(g) << endl;
        }
        else if (command == "edge"){
            cout << "Num Edge: " << boost::num_edges(g) << endl;
            outFile << "Num Edge: " << boost::num_edges(g) << endl;
        }
        else if (command == "degrees"){

            vector<pair<int, Vertex>> degrees;
            ofstream degreeFile;

            vertex_degrees(g, degrees);

            degreeFile.open(outFileName + "-vertex-degrees.txt", ios_base::out | ios_base::app);
            degreeFile.precision(10);
            degreeFile << filename << endl;
            degreeFile << "degree, vertex" << endl;
            for(auto it = degrees.begin(); it != degrees.end(); ++it)
            {
                degreeFile << it->first <<", "<<it->second << endl;
            }
            degreeFile.close();

            cout << "Degrees computed." << endl;
        }
        else if (command == "exit") break;
        else cout << "Did not recognize command." << endl;
    }
    outFile.close();
}

