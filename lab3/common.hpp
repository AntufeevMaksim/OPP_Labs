#pragma once

#include "matrix.hpp"

#include <vector>
#include <string>
std::pair<int, int> computeGridSize(int N);
int init_N(char* arg);
void write_mat(double* mat, int h, int w);
bool check_res(double* mat, int h, int w);
void write_mat_colmajor(double* mat, int h, int w);
