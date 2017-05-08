CXX = g++
INCLUDE = ./boost_1_63_0
CXXFLAGS = -Wall -MMD -std=c++14 -I$(INCLUDE)
EXEC = GraphProfile
OBJECTS = main.o
DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC}

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${EXEC} ${DEPENDS}
