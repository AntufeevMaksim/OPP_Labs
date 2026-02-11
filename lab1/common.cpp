#include "common.hpp"
#include "matrix.hpp"
#include "first_matrix.hpp"
#include "heat_matrix.hpp"

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

std::vector<double> generateB(int Nx, int Ny, int sourcesCount)
{
    int N = Nx * Ny;
    std::vector<double> b(N, 0.0);

    std::mt19937 gen(42);

    std::uniform_int_distribution<> indexDist(0, N - 1);
    std::uniform_real_distribution<> valueDist(-50.0, 50.0);

    for (int i = 0; i < sourcesCount; ++i)
    {
        int idx = indexDist(gen);
        b[idx] = valueDist(gen);
    }

    return b;
}

bool is_number(const std::string &str)
{
    return !str.empty() && std::all_of(str.begin(), str.end(), [](unsigned char c)
                                       { return std::isdigit(c); });
}

void init_mat_vec1(Matrix &A, std::vector<double> &b, char *arg, int N, int start_h, int h, int rows)
{
    if (!std::strcmp(arg, "mat1"))
    {
        FirstMatrix mat_getter;
        A = Matrix(start_h, N, h, mat_getter);
        b = std::vector<double>(N, N + 1.0);
    }
    else if (!std::strcmp(arg, "math"))
    {
        auto [Nx, Ny] = computeGridSize(N);
        HeatMatrix mat_getter(Nx, Ny);
        A = Matrix(start_h, N, h, mat_getter);
        b = generateB(Nx, Ny);
    }
    else
    {
        std::cerr << "Wrong matrix name: " << arg << std::endl;
    }
}

void init_mat_vec2(Matrix &A, std::vector<double> &b, char *arg, int N, int start_h, int h, int rows)
{
    if (!std::strcmp(arg, "mat1"))
    {
        FirstMatrix mat_getter;
        A = Matrix(start_h, N, h, mat_getter);
        b = std::vector<double>(rows + 1, N + 1.0);
    }
    else if (!std::strcmp(arg, "math"))
    {
        auto [Nx, Ny] = computeGridSize(N);
        HeatMatrix mat_getter(Nx, Ny);
        A = Matrix(start_h, N, h, mat_getter);
        std::vector<double> tmp = generateB(Nx, Ny);
        b.assign(tmp.begin() + start_h, tmp.begin() + start_h + rows + 1);
    }
    else
    {
        std::cerr << "Wrong matrix name: " << arg << std::endl;
    }
}


void init_N(int &N, char* arg)
{
    try
    {
        std::string s_str(arg);
        N = std::stoi(s_str);
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
    }
}

void write_info(int iter, double time, int var, int num_proc)
{
    std::fstream out_file("info.txt", std::ios::out | std::ios::trunc);

    out_file << "Variant: " << var << " Num processes: " << num_proc << std::endl;
    out_file << "Iterations: " << iter << std::endl;
    out_file << "Time: " << time << std::endl;
    out_file.close();    
}