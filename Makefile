CXX = g++
INCLUDE = ./boost_1_63_0
CXXFLAGS = -MMD -std=c++14 -I$(INCLUDE)
EXEC = GraphProfile
OBJECTS = adjacency_list_read.o \
			betweenness_centrality.o \
			bfs_furthest_vertex.o \
			bfs_print_visitor.o \
			centrality_bfs_visitor.o \
			connected_comp.o \
			edge_list_print.o \
			graph_diameter.o \
			graph_reduction.o \
			local_cluster_coeff.o \
			min_spanning_tree.o \
			onion_decomp.o \
			page_rank.o \
			reachability.o \
			read_command.o \
			shortest_path.o \
			suite_test.o \
			triangle_count.o \
			main.o

DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC}

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${EXEC} ${DEPENDS}
