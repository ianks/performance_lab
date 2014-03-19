##
##

CXX	= g++
CXXFLAGS= -O3 -static
LIBS = -pg

goals: judge
	echo "Done"

filter: FilterMain.cpp Filter.cpp cs1300bmp.cc
	$(CXX) $(CXXFLAGS) -o filter FilterMain.cpp Filter.cpp cs1300bmp.cc $(LIBS)
##
## Parameters for the test run
##

#gprof:
	

FILTERS = gauss.filter vline.filter hline.filter emboss.filter
IMAGES = boats.bmp blocks-small.bmp
TRIALS = 1 2 3 4

judge: filter
	-./Judge -p ./filter -i boats.bmp
	-./Judge -p ./filter -i blocks-small.bmp

valgrind: filter
	valgrind --tool=callgrind --simulate-cache=yes --collect-jumps=yes ./filter gausss.filter blocks-small.bmp

clean:
	-rm *.o
	-rm filter
	-rm filtered-*.bmp
