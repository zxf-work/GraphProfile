# GraphProfile
Fast and accurate profiling of graph data


Graph metrics to profile:

1. Diameter/Effective-Diameter;
2. Triangle count;
3. Connected components;
4. Degree (In/Out) distribution;
5. LCC: local clustering coeffcient;
6. Various sampling/heuristics based graph reduction
7. Centrality, pagerank, and maximum independent set

Set up instructions:

1. Ensure that you have g++, make, and c++14 installed on your machine.
	Basic tools needed for c++ development.
2. Install Intel TBB on your machine:
	TBB is the library used to do multithreading and parallel processing for the graph profiling algorithms.
	Download the appropriate package for your OS from https://github.com/01org/tbb/releases, extract, and move to a folder. cd into the folder and `make` to build our the library.
	Notes:
	Don't try to follow the official instructions on the TBB site, they suck, are heavily dependent on Microsoft VS, don't work, and make no sense.
	YMMV with Windows machines. I couldn't to get TBB to work on my machine, and ended up just doing all my development on my student linux machine (linux.student.cs.uwaterloo) instead, via ssh and FTP.
3. Point the TBB_INCLUDE and TBB_LIBRARY_RELEASE variables in the Makefile to the locations where you've just installed TBB.
	This is so that g++ will be able to find the location of the library when compiling/linking. You'll notice that they're currently pointed to `/u4/b2cui/tbb-2018_U1/include` and `/u4/b2cui/tbb-2018_U1/build/linux_intel64_gcc_cc5.4.1_libc2.23_kernel4.4.0_release`, which is where I had installed my TBB.

Build instructions:

The default build (`make`) will create an executable that runs both the parallelized and unparallelized algorithms for every command, for benchmarking purposes. To create an executable that only runs the parallelized version, or only the unparallelized version (if you don't have TBB readily installed), for production use, use `make tbb` or `make seq`, respectivly. Note that `make tbb` and `make seq` will do a full clean before building. If you do not want this behaviour, you may need to edit the Makefile.
