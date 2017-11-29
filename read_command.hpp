#ifndef READ_COMMAND_HPP_INCLUDED
#define READ_COMMAND_HPP_INCLUDED

#include "common.h"
#include <string>

using namespace std;

//reads cin for a command, prints to file
//expects the file to be already open, and closed elsewhere
//template expects Graph or DirectedGraph
void readCommand(const Graph &g, string filename);

#endif // READ_COMMAND_HPP_INCLUDED
