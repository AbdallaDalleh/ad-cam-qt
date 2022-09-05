#ifndef CURVE_FITTING
#define CURVE_FITTING

#include <vector>
#include <cmath>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlinear.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

struct data
{
    double* t;
    double* y;
    size_t n;
};

/* model function: a * exp( -1/2 * [ (t - b) / c ]^2 ) */
double gaussian (const double a, const double b, const double c, const double d, const double x);
int    func_f   (const gsl_vector* x, void* params, gsl_vector* f);
int    func_df  (const gsl_vector* x, void* params, gsl_matrix* J);
int    func_fvv (const gsl_vector* x, const gsl_vector* v, void* params, gsl_vector* fvv);
void   callback (const size_t iter, void* params, const gsl_multifit_nlinear_workspace* w);

void   solve_system(gsl_vector* x, gsl_multifit_nlinear_fdf* fdf, gsl_multifit_nlinear_parameters* params);

std::vector<double> curve_fit(std::vector<double> t, std::vector<double> y, std::vector<double> initial_parameters);

#endif
