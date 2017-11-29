CXX = g++
INCLUDE = ./boost_1_63_0
TBB_INCLUDE=/u4/b2cui/tbb-2018_U1/include
TBB_LIBRARY_RELEASE=/u4/b2cui/tbb-2018_U1/build/linux_intel64_gcc_cc5.4.1_libc2.23_kernel4.4.0_release
CXXFLAGS = -MMD -std=c++14 -DSEQ -DTBB -I$(INCLUDE) -I$(TBB_INCLUDE) -Wl,-rpath,${TBB_LIBRARY_RELEASE} -L${TBB_LIBRARY_RELEASE} -ltbb
EXEC = GraphProfile

seq_build: CXXFLAGS = -MMD -std=c++14 -DSEQ -I$(INCLUDE)
tbb_build: CXXFLAGS = -MMD -std=c++14 -DTBB -I$(INCLUDE) -I$(TBB_INCLUDE) -Wl,-rpath,${TBB_LIBRARY_RELEASE} -L${TBB_LIBRARY_RELEASE} -ltbb


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
					degrees.o \
					main.o

# compile both parallelized and non parallelized versions of functions
# use to benchmark performance
${EXEC}: ${OBJECTS}
	${CXX} ${OBJECTS} ${CXXFLAGS} -o ${EXEC}

# compile only parallelized functions
# use in production/on machines with TBB installed
.PHONY: tbb
tbb:
	$(MAKE) clean
	$(MAKE) tbb_build

tbb_build: ${OBJECTS}
	${CXX} ${OBJECTS} ${CXXFLAGS} -o ${EXEC}

# compile only unparallelized functions
# use when TBB is not available - e.g. on windows computers
.PHONY: seq
seq:
	$(MAKE) clean
	$(MAKE) seq_build

seq_build: ${OBJECTS}
	${CXX} ${OBJECTS} ${CXXFLAGS} -o ${EXEC}

.PHONY: clean
clean:
	rm -f *.o *.d ${EXEC}