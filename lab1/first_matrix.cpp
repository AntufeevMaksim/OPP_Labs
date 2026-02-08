#include "first_matrix.hpp"

double FirstMatrix::get(int i, int j)
{
    return i == j ? 2.0 : 1.0;
}