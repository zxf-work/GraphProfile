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

        if (filename == "suitemodetop")
        {
            vector<string>filenames;
            string filename;
            cout << "Enter Filenames (type exit to finish)" << endl;
            cin >> filename;
            while(filename != "exit")
            {
                filenames.push_back(filename);
                cin >> filename;
            }
            suite_test_top_k(filenames);
        }
        if (filename == "suitemodetri")
        {
            vector<string>filenames;
            string filename;
            cout << "Enter Filenames (type exit to finish)" << endl;
            cin >> filename;
            while(filename != "exit")
            {
                filenames.push_back(filename);
                cin >> filename;
            }
            suite_test_triangle_top_k(filenames);
        }
        if (filename == "suitemodetree")
        {
            vector<string>filenames;
            string filename;
            cout << "Enter Filenames (type exit to finish)" << endl;
            cin >> filename;
            while(filename != "exit")
            {
                filenames.push_back(filename);
                cin >> filename;
            }
            suite_test_spanning_tree(filenames);
        }
        if (filename == "suitemode")
        {
            vector<string>filenames;
            vector<string>filenames2;
            filenames.push_back("C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\facebook_combined.txt");
            filenames.push_back("C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\CondMat.txt");
            filenames.push_back("C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\astroph.txt");
            filenames.push_back("C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\enron.txt");
            filenames.push_back("C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\epinions.txt");
            filenames.push_back("C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\Autonomous.txt");
            filenames.push_back("C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\youtube.txt");
            //filenames.push_back("C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\skitter.txt");
            //filenames2.push_back("C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\skitter.txt");

            //suite_test(filenames);
            //suite_test_spanning_tree(filenames);
            suite_test_top_k(filenames);
            suite_test_triangle_top_k(filenames);
            //suite_test_top_k(filenames2);
        }

        Graph g;

        if (adjacency_list_read(g, filename.c_str()))
        {
            readCommand(g, filename);
        }
    }

    return 0;
}
