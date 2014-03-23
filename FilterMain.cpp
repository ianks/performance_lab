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

  const int input_height   = input->height - 1;
  const int input_width    = input->width - 1;
  const int filter_size    = 3;
  const int filter_divisor = filter->getDivisor();

  output -> width = input -> width;
  output -> height = input -> height;

  int filter_array[filter_size][filter_size];
  for (int i = 0; i < filter_size; i++){
    for (int j = 0; j < filter_size; j++){
      filter_array[i][j] = filter->get(i,j);
    }
  }

  // int color_copy[3][input_width][input_height];
  // for (int i = 0; i < 3; ++i){
  //   for (int j = 1; j < input_width; ++j){
  //     for (int k = 1; k < input_height; ++k){
  //       color_copy[i][j][k] = input->color[i][j][k];
  //     }
  //   }
  // }

  for(int plane = 0; plane < 3; plane++) {
    for(int row = 1; row < input_height; row++) {
      int tmp = row - 1;
      for(int col = 1; col < input_width; col++) {


          int acc = 0;
          int acc1=0, acc2=0, acc3=0;

          for (int i = 0; i < filter_size; i++) {
            int new_row = tmp + i;
            acc1 += input->color[plane][new_row][col + 0 - 1] * filter_array[i][0];
            acc2 += input->color[plane][new_row][col + 1 - 1] * filter_array[i][1];
            acc2 += input->color[plane][new_row][col + 2 - 1] * filter_array[i][2];
          }

        acc = (acc1 + acc2 + acc3) / filter_divisor;

        if ( acc  < 0 )
          acc = 0;
        if ( acc  > 255 )
          acc = 255;

        output -> color[plane][row][col] = acc;
      }
    }
  }

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
    diff, diff / (output -> width * output -> height));
  return diffPerPixel;
}

