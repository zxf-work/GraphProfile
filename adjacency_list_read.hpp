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


            std::string edgeStart;
            datafile >> edgeStart;
            std::string edgeEnd;
            datafile >> edgeEnd;

            //add the edge
            //std::cout << "Adding Edge (" << std::stoi(edgeStart) << "," << std::stoi(edgeEnd)<< ")" << std::endl;
            add_edge(std::stoi(edgeStart), std::stoi(edgeEnd), EdgeWeightProperty(1), g);

            ++edgeCounter;
            if (edgeCounter % 50000 == 0) { std::cout << "Adding edge #" << edgeCounter << std::endl; }
    /**
            //vertex counting
            //int numVertex = 0;
            if(isNumber)
            {
                int largerVertex = std::stoi(edgeStart) > std::stoi(edgeEnd) ? std::stoi(edgeStart):std::stoi(edgeEnd);
                if (numVertex < largerVertex) numVertex  = largerVertex;
            }
            else {
                numVertex++;
            }

    **/

        }
        return true;
    }
}




#endif // ADJACENCYLISTREAD_H_INCLUDED