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
    string file;

    while (true)
    {
        cout << "Enter Adjacency List Filename: (type exit to quit)" << endl;
        getline(cin, file);
        bool suitemode = false;
        Graph g;

        if (file == "stop" || file == "exit") break;
        else if (file == "default") file = "C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\facebook_combined.txt";
        else if (file == "small") file = "C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\small test.txt";
        else if (file == "enron") file = "C:\\Users\\Vincent Luong\\Documents\\Git\\GraphProfile\\Graph Data\\enron.txt";

        else if (file == "suitemodetop")
        {
            suitemode = true;
            vector<string>filenames;
            string filename;
            cout << "Enter Filenames (type exit to finish)" << endl;
            getline(cin, filename);
            while(filename != "exit")
            {
                filenames.push_back(filename);
                getline(cin, filename);
            }
            suite_test_top_k(filenames);
        }
        else if (file == "suitemodetri")
        {
            suitemode = true;
            vector<string>filenames;
            string filename;
            cout << "Enter Filenames (type exit to finish)" << endl;
            getline(cin, filename);
            while(filename != "exit")
            {
                filenames.push_back(filename);
                getline(cin, filename);
            }
            suite_test_triangle_top_k(filenames);
        }
        else if (file == "suitemodetree")
        {
            suitemode = true;
            vector<string>filenames;
            string filename;
            cout << "Enter Filenames (type exit to finish)" << endl;
            getline(cin, filename);
            while(filename != "exit")
            {
                filenames.push_back(filename);
                getline(cin, filename);
            }
            suite_test_spanning_tree(filenames);
        }
        else if (file == "suitemode")
        {
            suitemode = true;
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

            suite_test(filenames);
            //suite_test_spanning_tree(filenames);
            //suite_test_top_k(filenames);
            //suite_test_triangle_top_k(filenames);
            //suite_test_top_k(filenames2);
        }

        //for dev use
        if (!suitemode)
        {
            if (adjacency_list_read(g, file.c_str()))
            {
                readCommand(g, file);
            }
        }

    }

    return 0;
}
