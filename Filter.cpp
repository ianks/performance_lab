#include "Filter.h"
#include <iostream>

Filter::Filter(int _dim)
{
  divisor = 1;
  dim = _dim;
  data = new short int[dim * dim];
}

short int Filter::get(int r, int c) const
{
  return data[ r * dim + c ];
}

void Filter::set(int r, int c, int value)
{
  data[ r * dim + c ] = value;
}

short int Filter::getDivisor() const
{
  return divisor;
}

void Filter::setDivisor(int value)
{
  divisor = value;
}

short int Filter::getSize() const
{
  return dim;
}

void Filter::info() const
{
  cout << "Filter is.." << endl;
  for (int col = 0; col < dim; col++) {
    for (int row = 0; row < dim; row++) {
      int v = get(row, col);
      cout << v << " ";
    }
    cout << endl;
  }
}
