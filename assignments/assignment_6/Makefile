CXX = g++
CXXFLAGS = -std=c++17 -O3
LDFLAGS= -pthread

assignment_6_omp: assignment_6_omp.cpp
	g++ ${CXXFLAGS} ${LDFLAGS} -fopenmp assignment_6_omp.cpp -o assignment_6_omp

assignment_6_ff: assignment_6_ff.cpp
	g++ ${CXXFLAGS} ${LDFLAGS} -DTRACE_FASTFLOW assignment_6_ff.cpp -o assignment_6_ff

assignment_6_ff_alt: assignment_6_ff_alt.cpp
	g++ ${CXXFLAGS} ${LDFLAGS} -DTRACE_FASTFLOW assignment_6_ff_alt.cpp -o assignment_6_ff_alt

assignment_6_grppi: assignment_6_grppi.cpp
	g++ ${CXXFLAGS} ${LDFLAGS} assignment_6_grppi.cpp -o assignment_6_grppi -fconcepts -fconcepts-ts -fpermissive