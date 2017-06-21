#ifndef ADJACENCYLISTREAD_H_INCLUDED
#define ADJACENCYLISTREAD_H_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <boost/graph/adjacency_list.hpp>
#include "common.h"

using namespace boost;

// expects file to be an adjecency list that is separated by only whitespace, no punctuation
// ex. 1 2 2 3 4 5 is a graph with edges 1-2, 2-3, 4-5.
bool adjacency_list_read(Graph& g, const char* filename)
{
    std::ifstream datafile;
    datafile.open(filename);

    if(!datafile)
    {
        std::cerr << "Cannot find file." << std::endl;
        return false;
    }
    else {
        size_t edgeCounter = 0;
        while (!datafile.eof())
        {


            long long edgeStart;
            long long edgeEnd;

            if(datafile >> edgeStart && datafile >> edgeEnd)
            {
                add_edge(edgeStart, edgeEnd, EdgeWeightProperty(1), g);
            }


            ++edgeCounter;
            if (edgeCounter % 50000 == 0) { std::cout << "Adding edge #" << edgeCounter << std::endl; }
        }
        return true;
    }
}




#endif // ADJACENCYLISTREAD_H_INCLUDED
