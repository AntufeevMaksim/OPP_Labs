#include "matrix_getter.hpp"

class FirstMatrix : public IMatrixGetter
{
public:
    virtual double get(int i, int j) override;
};