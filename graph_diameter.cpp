#include "common.h"
#include <utility>
#include <boost/graph/graph_traits.hpp>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <algorithm>
#include "bfs_furthest_vertex.hpp"
#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include "graph_reduction.hpp"
#include "graph_diameter.hpp"

#ifdef TBB
#include "tbb/tbb.h"
#endif

using namespace boost;

#ifdef TBB
//multithreaded implementation of approx_graph_diameter
vertices_size_type approx_graph_diameter_multithread(const Graph &g)
{
    srand(time(NULL));

    vertices_size_type lowerdiam = std::numeric_limits<unsigned>::max();
    vertices_size_type diameter = 0;

    tbb::parallel_for( tbb::blocked_range<std::size_t>( 0, 1000 ),
        [&g, &lowerdiam, &diameter](tbb::blocked_range<std::size_t>& r){
            for (std::size_t i = r.begin(); i != r.end(); ++i)
            {
                //start at random vertex
                Vertex v = vertex(rand() % num_vertices(g), g);
                //find next vertex u
                std::pair<Vertex, vertices_size_type> uPair = bfs_furthest_vertex(g, v);
                if (uPair.second > diameter) diameter = uPair.second;
                if (uPair.second < lowerdiam && uPair.second != 0) lowerdiam = uPair.second;

                //find next vertex w
                std::pair<Vertex, vertices_size_type> wPair = bfs_furthest_vertex(g, v);
                if (wPair.second > diameter) diameter = wPair.second;
                if (wPair.second < lowerdiam && uPair.second != 0) lowerdiam = wPair.second;

                //stop if lowerdiam is 2x the diam
                if(lowerdiam * 2 == diameter) break;
            }
        }
    );

    return diameter;
}
#endif


#ifdef SEQ
//idea: use BFS starting at a random vertex v0, find a vertex u0 that is furthest away from v0
//use BFS at vertex u0, find vertex w0 that is furthest away from u0, with distance d0
//repeat with another random vertex v1 != v0
//possible TODO: disallow repeated vertices as v
vertices_size_type approx_graph_diameter(const Graph &g)
{
    srand(time(NULL));

    vertices_size_type lowerdiam = std::numeric_limits<unsigned>::max();
    vertices_size_type diameter = 0;
    //main loop
    for (int i = 0; i < 1000; i++)
    {
        //start at random vertex
        Vertex v = vertex(rand() % num_vertices(g), g);
        //find next vertex u
        std::pair<Vertex, vertices_size_type> uPair = bfs_furthest_vertex(g, v);
        if (uPair.second > diameter) diameter = uPair.second;
        if (uPair.second < lowerdiam && uPair.second != 0) lowerdiam = uPair.second;

        //find next vertex w
        std::pair<Vertex, vertices_size_type> wPair = bfs_furthest_vertex(g, v);
        if (wPair.second > diameter) diameter = wPair.second;
        if (wPair.second < lowerdiam && uPair.second != 0) lowerdiam = wPair.second;

        //stop if lowerdiam is 2x the diam
        if(lowerdiam * 2 == diameter) break;
    }
    return diameter;


}
#endif

//if bfs_count = 2, iterations = 1000, same as above.
unsigned approx_graph_diameter_bfs(const Graph &g, int bfs_count, int iterations)
{
    srand(time(NULL));

    vertices_size_type diameter = 0;

    //iteration loop
    for (int i = 0; i < iterations; ++i)
    {
        //start at random vertex
        Vertex v = vertex(rand() % num_vertices(g), g);

        for(int j = 0; j < bfs_count; ++j)
        {
            //find next vertex u
            std::pair<Vertex, vertices_size_type> uPair = bfs_furthest_vertex(g, v);
            v = uPair.first;
            if (uPair.second > diameter) diameter = uPair.second;
        }

    }

    return diameter;
}

//finds an upper bound and a lower bound to the diameter
//upper bound: diameter of a spanning tree
//this diameter is measured by 2BFS (exact for trees)
//lower bound: 2BFS
//forces 10 iterations
std::pair<vertices_size_type, vertices_size_type> khaled_approx_diameter(const Graph &g)
{
    //create ordered list of vertices by degree (ordered smallest to greatest)
    std::multimap<unsigned, Vertex> vertexDegreeMap;
    for(unsigned v = 0; v < num_vertices(g); ++v)
    {
        vertexDegreeMap.emplace(degree(v, g), v);
    }


    vertices_size_type lowerbound = 0;
    vertices_size_type upperbound = std::numeric_limits<unsigned>::max();
    int iterations = 0;
    for(auto it = vertexDegreeMap.rbegin(); it!= vertexDegreeMap.rend(); ++it)
    {
        if(upperbound - lowerbound <= 5 && iterations > 10) break;

        ++iterations;

        Vertex v = it->second;

        //compute upper bound
        Graph h;
        std::vector<Vertex> root;
        root.push_back(v);
        //create tree
        graph_reduction_spanning_tree(g, h, root);
        //compute diameter by 2BFS
        std::pair<Vertex, vertices_size_type> uPair = bfs_furthest_vertex(h, v);
        std::pair<Vertex, vertices_size_type> upperboundPair = bfs_furthest_vertex(h, uPair.first);  //diameter is wPair.second

        if(upperboundPair.second < upperbound) upperbound = upperboundPair.second;

        //compute lowerbound
        std::pair<Vertex, vertices_size_type> wPair = bfs_furthest_vertex(g, v);
        std::pair<Vertex, vertices_size_type> lowerboundPair = bfs_furthest_vertex(g, wPair.first);
        std::cout << wPair.first << " " << lowerboundPair.first << std::endl;
        if(lowerboundPair.second > lowerbound && lowerboundPair.second != 0) lowerbound = lowerboundPair.second;


    }

    return std::pair<vertices_size_type, vertices_size_type>(lowerbound, upperbound);

}

//computes all pairs shortest path problem
//uses either johnson's method or floyd's method
//space complexity O(V^2)
//time complextity O(E V logV) for johnson or O(V^3) for floyd
//uses johnson if dense is set to true
int simple_graph_diameter(const Graph &g, bool dense)
{
    unsigned V = num_vertices(g);
    std::vector< std::vector<int> > D(V, std::vector<int>(V, std::numeric_limits<int>::max()));
    if (num_edges(g) == 0) return -1;
    //std::vector<int> weights(num_edges(g), 1);

    std::cout << "Starting all pairs shortest paths." << std::endl;
    if(dense) floyd_warshall_all_pairs_shortest_paths(g, D);
    else johnson_all_pairs_shortest_paths(g, D);
    std::cout<< "Finished all pairs shortest paths." << std::endl;
    int shortestPath[V];
    for (unsigned i = 0; i < V; ++i)
    {
        shortestPath[i] = 0;
        for(unsigned j = 0; j < V; ++j)
        {
            if (shortestPath[i] < D[i][j] && D[i][j] != std::numeric_limits<int>::max()) shortestPath[i] = D[i][j];
                //shortestPath[i] = *std::max_element(&D[i][0], &D[i][V] + 1);
        }

    }
    return *std::max_element(shortestPath, shortestPath+V);
}

//comptue all pairs shortest path, one vertex at a time
//space complextiy O(V)
//time complexity O(V^2 * EV)
//returns longest shortest path aka diameter
unsigned memory_graph_diamter(const Graph &g)
{
    unsigned current_diam = 0;
    for(auto vi = vertices(g).first; vi != vertices(g).second; ++vi)
    {
        if(*vi % 1000 == 0) std::cout << "Current Diam: " << *vi << " " << current_diam <<std::endl;
        std::pair<Vertex, vertices_size_type> bfsPair = bfs_furthest_vertex(g, *vi);
        if(bfsPair.second > current_diam) current_diam = bfsPair.second;
    }
    return current_diam;
}
