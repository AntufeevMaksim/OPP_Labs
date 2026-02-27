#include "common.hpp"

#include <fstream>
#include <cstring>
#include <iostream>
#include <random>
#include <algorithm>

std::pair<int, int> computeGridSize(int N)
{
    int Ny = static_cast<int>(std::sqrt(N));

    while (Ny > 0)
    {
        if (N % Ny == 0)
            break;
        --Ny;
    }

    int Nx = N / Ny;

    return {Nx, Ny};
}


bool is_number(const std::string &str)
{
    return !str.empty() && std::all_of(str.begin(), str.end(), [](unsigned char c)
                                       { return std::isdigit(c); });
}


int init_N(char* arg)
{
    int N;
    try
    {
        std::string s_str(arg);
        N = std::stoi(s_str);
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
    }
    return N;
}

void write_info(int iter, double time, int var, int num_proc, long total_mem)
{
    std::fstream out_file("info.txt", std::ios::out | std::ios::trunc);

    out_file << "Variant: " << var << " Num processes: " << num_proc << std::endl;
    out_file << "Iterations: " << iter << std::endl;
    out_file << "Time: " << time << std::endl;
    out_file << "Total mem:" << total_mem << std::endl;
    out_file.close();    
}

void write_mat(double* mat, int h, int w)
{
    std::fstream out_file("output.txt", std::ios::out | std::ios::trunc);

    for (int i = 0; i < h; ++i)
    {
        for (int j = 0; j < w; ++j)
        {
            out_file << mat[i * w + j] << " ";
        }
        out_file << std::endl;
    }
}

void write_mat_colmajor(double* mat, int h, int w)
{
    std::fstream out_file("output.txt", std::ios::out | std::ios::trunc);

    for (int row = 0; row < h; ++row)              // строки сверху вниз
    {
        for (int col = 0; col < w; ++col)          // столбцы слева направо
        {
            // В col-major: элемент (row, col) находится по индексу col * h + row
            out_file << mat[col * h + row] << " ";
        }
        out_file << std::endl;
    }
}
bool check_res(double* mat, int h, int w)
{
    for (int i = 0; i < h; ++i)
    {
        for (int j = 0; j < w; ++j)
        {
            if (mat[i * w + j] != (i+1) * (j+1) * w)
                return false;
        }
    }
    return true;
}