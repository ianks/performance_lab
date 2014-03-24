#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"
#include <omp.h>

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

  output -> width = input_width + 1;
  output -> height = input_height + 1;


  int filter_array[3][3];
  #pragma omp parallel for
  for (int i = 0; i < 3; i++){
    for (int j = 0; j < 3; j++){
      filter_array[i][j] = filter->get(i,j);
    }
  }

  // Hi-Line
  if (filter_array[0][1] == -2){
    #pragma omp parallel for
    for(int p = 0; p < 3; p++) {
      for(int r = 1; r < input_height; r++) {

        const int new_row1 = r - 1;
        const int new_row3 = r + 1;

        for(int c = 1; c < input_width; c++) {

          int acc1=0, acc2=0, acc3=0;
          const int col1 = c - 1;
          const int col3 = c + 1;


          /*-----------------------------*/
          acc1 += -(input->color[p][new_row1][col1]);
          acc2 += -(input->color[p][new_row1][c] << 1);
          acc3 += -(input->color[p][new_row1][col3]);

          acc1 += input->color[p][new_row3][col1];
          acc2 += input->color[p][new_row3][c] << 1;
          acc3 += input->color[p][new_row3][col3];
          /*-----------------------------*/

          int output_color = acc1+acc2+acc3;

          output_color           = (output_color < 255) ? output_color : 255;
          output->color[p][r][c] = (output_color > 0)   ? output_color : 0;
        }
      }
    }
  }

  // Gauss
  else if (filter_array[1][1] == 8){
    #pragma omp parallel for
    for(int p = 0; p < 3; p++) {
      for(int r = 1; r < input_height; r++) {

        const int new_row1 = r - 1;
        const int new_row3 = r + 1;

        for(int c = 1; c < input_width; c++) {

          int acc1=0, acc2=0, acc3=0;
          const int col1 = c - 1;
          const int col3 = c + 1;

          /*-----------------------------*/
          acc2 += input->color[p][new_row1][c] << 2;

          acc1 += input->color[p][r][col1] << 2;
          acc2 += input->color[p][r][c] << 3;
          acc3 += input->color[p][r][col3] << 2;

          acc2 += input->color[p][new_row3][c] << 2;
          /*-----------------------------*/

          // divide by 24
          int output_color = ((acc1+acc2+acc3) >> 3) / 3;

          if (output_color > 255){
            output->color[p][r][c] = 255;
            continue;
          }
          if (output_color < 0){
            output->color[p][r][c] = 0;
            continue;
          }

          output->color[p][r][c] = output_color;
        }
      }
    }
  }

  // Emboss
  else if (filter_array[1][2] == -1) {
    #pragma omp parallel for
    for(int p = 0; p < 3; p++) {
      for(int r = 1; r < input_height; r++) {

        const int new_row1 = r - 1;
        const int new_row3 = r + 1;

        for(int c = 1; c < input_width; c++) {

          int acc1=0, acc2=0, acc3=0;
          const int col1 = c - 1;
          const int col3 = c + 1;

          /*-----------------------------*/
          acc1 += input->color[p][new_row1][col1];
          acc2 += input->color[p][new_row1][c];
          acc3 += -(input->color[p][new_row3][c]);

          acc1 += input->color[p][r][col1];
          acc2 += input->color[p][r][c];
          acc3 += -(input->color[p][new_row3][c]);

          acc1 += input->color[p][new_row3][col1];
          acc2 += -(input->color[p][new_row3][c]);
          acc3 += -(input->color[p][new_row3][col3]);
          /*-----------------------------*/

          int output_color = acc1+acc2+acc3;

          if (output_color > 255){
            output->color[p][r][c] = 255;
            continue;
          }
          if (output_color < 0){
            output->color[p][r][c] = 0;
            continue;
          }

          output->color[p][r][c] = output_color;
        }
      }
    }
  }
  // Average
  else{
    #pragma omp parallel for
    for(int p = 0; p < 3; p++) {
      // #pragma omp parallel for
      for(int r = 1; r < input_height; r++) {
        const int new_row1 = r - 1;
        const int new_row3 = r + 1;
        for(int c = 1; c < input_width; c++) {

          int acc1=0, acc2=0, acc3=0;
          const int col1 = c - 1;
          const int col3 = c + 1;


          /*-----------------------------*/
          acc1 += input->color[p][new_row1][col1];
          acc2 += input->color[p][new_row1][c];
          acc3 += input->color[p][new_row1][col3];

          acc1 += input->color[p][r][col1];
          acc2 += input->color[p][r][c];
          acc3 += input->color[p][r][col3];

          acc1 += input->color[p][new_row3][col1];
          acc2 += input->color[p][new_row3][c];
          acc3 += input->color[p][new_row3][col3];
          /*-----------------------------*/

          // divide by 24
          int output_color = (acc1+acc2+acc3) / 9;

          if (output_color < 0){
            output->color[p][r][c] = 0;
            continue;
          }

          if (output_color > 255){
            output->color[p][r][c] = 255;
            continue;
          }
          output->color[p][r][c] = output_color;
        }
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

