#include "Filter.h"
#include <iostream>

Filter::Filter(int _dim)
{
  divisor = 1;
  dim = _dim;
  data = new int[dim * dim];
}

inline int Filter::get(int &r, int &c)
{
  return data[ r * dim + c ];
}

inline void Filter::set(int &r, int &c, int &value)
{
  data[ r * dim + c ] = value;
}

inline int Filter::getDivisor()
{
  return divisor;
}

inline void Filter::setDivisor(int &value)
{
  divisor = value;
}

inline int Filter::getSize()
{
  return dim;
}

void Filter::info()
{
  cout << "Filter is.." << endl;
  for (int row = 0; row < dim; row++) {
    for (int col = 0; col < dim; col++) {
      int v = get(row, col);
      cout << v << " ";
    }
    cout << endl;
  }
}
