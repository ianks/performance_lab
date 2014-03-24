##
##

CXX	= g++-4.6
CXXFLAGS = -m64 -Os -funroll-loops -fopenmp -std=c++0x
LIBS = -pg

goals: judge
	echo "Done"

filter: FilterMain.cpp Filter.cpp cs1300bmp.cc
	$(CXX) $(CXXFLAGS) -o filter FilterMain.cpp Filter.cpp cs1300bmp.cc $(LIBS)

#gprof:

test: filter
	-./Judge -p ./filter -i boats.bmp


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
