#include <iostream>
#include "adjacency_list_read.hpp"
#include <utility>
#include <boost/graph/graph_traits.hpp>
#include <string>
#include "bfs_print_visitor.hpp"
#include <boost/graph/breadth_first_search.hpp>
#include "edge_list_print.hpp"

using namespace std;
using namespace boost;

int main()
{
    Graph g;


    string filename;// = "C:\\Users\\Eric\\Documents\\Vincent Work\\Graph data\\facebook_combined.txt";
    cout << "Enter Adjacency List Filename:" << endl;
    getline(cin, filename);
    cout << "File: " << filename << endl;
    if (adjacency_list_read(g, filename.c_str()))
    {
        edge_list_print(g);

        graph_traits<Graph>::vertex_descriptor u = boost::vertex(1158, g);
        bfs_print_visitor vis;
        breadth_first_search(g, u, visitor(vis));
    }

    /**
    boost::graph_traits<Graph>::vertex_descriptor u, v;
    u = boost::vertex(1158, g);
    v = boost::vertex(1159, g);


    boost::property_map<Graph, boost::edge_weight_t>::type weight = get(edge_weight, g);
    cout << "Weight of 1159: " << get(weight, v) << endl;
    **/


    //cout << "Num Edges: ";
    //cout << boost::num_edges(g) << endl;
    return 0;
}
