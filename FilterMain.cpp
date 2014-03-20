#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"

using namespace std;

#include "rtdsc.h"

//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int
main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
    delete input;
    delete output;
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

struct Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());

  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int div;
    input >> div;
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
	int value;
	input >> value;
	filter -> set(i,j,value);
      }
    }
    return filter;
  }
}


  double
applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output)
{

  long long cycStart, cycStop;

  cycStart = rdtscll();

  // these values were previously used in for loops
  // meaning that each function/variable was called n times
  const int input_height   = input->height;
  const int input_width    = input->width;
  const int filter_size    = filter->getSize();
  const int filter_divisor = filter->getDivisor();
  output->width  = input_width;
  output->height = input_height;

  // create filter array to avoid unneccesary memory references
  // increased score from 55 to 71
  int filter_array[3][3];
  for (int i = 0; i < 3; i++){
    for (int j = 0; j < 3; j++){
      filter_array[i][j] = filter->get(i,j);
    }
  }

  // move by rows first to optimize use of DRAM cache
  for(int p = 0; p < 3; p++) {
    for(int r = 1; r <= input_height; r++) {
      for(int c = 1; c <= input_width; c++) {

        // using acc alone increased score from 55 to 56
        int acc = 0;

        // go rows then columns here too
        for (int i = 0; i < filter_size; i++) {
          for (int j = 0; j < filter_size; j++) {
            // checking if 1 increased score from 71 to 75
            if (filter_array[i][j] == 1)
              acc = acc + input->color[p][r + j - 1][c + j - 1];

            else if (filter_array[i][j] == -1)
              acc = -(acc + input->color[p][r + j - 1][c + j - 1]);
            
            else
              acc = acc + (input->color[p][r + j -1][c+ j -1] * filter_array[i][j]);
          }
        }

        acc = acc / filter_divisor;

        if ( acc  < 0 )
          acc = 0;

        if ( acc  > 255 )
          acc = 255;

        output->color[p][r][c] = acc;
      }
    }
  }

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  //variablize the output width and height
  double diffPerPixel = diff / (input_width * input_height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diffPerPixel);
  return diffPerPixel;
}
