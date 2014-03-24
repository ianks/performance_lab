//-*-c++-*-
#ifndef _Filter_h_
#define _Filter_h_

using namespace std;

class Filter {
  short int divisor;
  short int dim;
  short int *data;

public:
  Filter(int _dim);
  short int get(int r, int c) const;
  void set(int r, int c, int value);

  short int getDivisor() const;
  void setDivisor(int value);

  short int getSize() const;
  void info() const;
};

#endif
