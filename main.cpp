#include <iostream>
#include <fstream>
#include "adjacency_list_read.hpp"
#include <utility>
#include <string>
#include "read_command.hpp"
#include "suite_test.hpp"

using namespace std;

int main()
{
    string filename;// = "C:\\Users\\Eric\\Documents\\Vincent Work\\Graph data\\facebook_combined.txt";

    while (true)
    {
        cout << "Enter Adjacency List Filename: (type exit to quit)" << endl;
        getline(cin, filename);

        if (filename == "stop" || filename == "exit") break;
        if (filename == "default") filename = "C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\facebook_combined.txt";
        if (filename == "small") filename = "C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\small test.txt";
        if (filename == "enron") filename = "C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\enron.txt";

        if (filename == "suitemode")
        {
            vector<string>filenames;
            filenames.push_back("C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\facebook_combined.txt");

            suite_test(filenames);
        }

        Graph g;

/*
        Graph t1;
        Graph t2;
        adjacency_list_read(t1, "C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\Reduced Graph3.txt");
        adjacency_list_read(t2, "C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\Reduced Graph5.txt");
        std::pair<EdgeIterator, EdgeIterator> gEdgeIt = boost::edges(t1);

        for(auto it = gEdgeIt.first; it != gEdgeIt.second; ++it)
        {
            if (! boost::edge(boost::source(*it, t1), boost::target(*it, t1), t2).second)
            {
                cout << boost::source(*it, t1) << " " << boost::target(*it, t1) << endl;
            }
        }
*/
        if (adjacency_list_read(g, filename.c_str()))
        {
            readCommand(g, filename);
        }
    }

    return 0;
}
