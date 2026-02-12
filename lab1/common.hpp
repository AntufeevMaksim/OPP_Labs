#pragma once

#include "matrix.hpp"

#include <vector>
#include <string>
std::vector<double> generateB(int Nx, int Ny, int sourcesCount = 10);
bool is_number(const std::string& str);
void init_mat_vec2(Matrix &A, std::vector<double> &b, char *arg, int N, int start_h, int h, int rows);
void init_mat_vec1(Matrix &A, std::vector<double> &b, char *arg, int N, int start_h, int h, int rows);
void init_N(int& N, char* arg);
void write_info(int iter, double time, int var, int num_proc, long total_mem);