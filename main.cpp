#include <iostream>
#include <fstream>
#include "adjacency_list_read.hpp"
#include <utility>
#include <string>
#include "read_command.hpp"

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
        if (filename == "small") filename == "C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\small test.txt";

        Graph g;

        if (adjacency_list_read(g, filename.c_str()))
        {
            readCommand(g, filename);
        }
    }

    return 0;
}
