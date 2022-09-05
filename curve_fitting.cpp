
#include "curve_fitting.h"

std::vector<double> curve_fit(std::vector<double> t, std::vector<double> y, std::vector<double> initial_parameters)
{
    std::vector<double> parameters;

    size_t n = (size_t) y.size();     /* number of data points to fit */
    size_t p = 4;                     /* number of model parameters */
    double a = initial_parameters[0]; /* amplitude */
    double b = initial_parameters[1]; /* center */
    double c = initial_parameters[2]; /* width */
    double d = initial_parameters[3]; /* offset */

    // gsl_vector* f = gsl_vector_alloc(n);
    gsl_vector* x = gsl_vector_alloc(p);
    gsl_multifit_nlinear_fdf fdf;
    gsl_multifit_nlinear_parameters fdf_params = gsl_multifit_nlinear_default_parameters();
    struct data fit_data;

    fit_data.t = t.data();
    fit_data.y = y.data();
    fit_data.n = n;

    /* define function to be minimized */
    fdf.f      = func_f;
    fdf.df     = func_df;
    fdf.fvv    = func_fvv;
    fdf.n      = n;
    fdf.p      = p;
    fdf.params = &fit_data;

    /* starting point */
    gsl_vector_set(x, 0, a);
    gsl_vector_set(x, 1, b);
    gsl_vector_set(x, 2, c);
    gsl_vector_set(x, 3, d);
    fdf_params.trs = gsl_multifit_nlinear_trs_lmaccel;
    solve_system(x, &fdf, &fdf_params);

    parameters.push_back(gsl_vector_get(x, 0));
    parameters.push_back(gsl_vector_get(x, 1));
    parameters.push_back(gsl_vector_get(x, 2));
    parameters.push_back(gsl_vector_get(x, 3));
    return parameters;
}

/* model function: a * exp( -1 * [ (t - b) ]^2 / c ) + d */
double gaussian(const double a, const double b, const double c, const double d, const double x)
{
    const double z = (x - b);
    return a * exp(-1 * z * z) / c + d;
}

int func_f(const gsl_vector * x, void *params, gsl_vector * f)
{
    struct data *data = (struct data *) params;
    double a = gsl_vector_get(x, 0);
    double b = gsl_vector_get(x, 1);
    double c = gsl_vector_get(x, 2);
    double d = gsl_vector_get(x, 3);
    size_t i;

    for (i = 0; i < data->n; ++i)
    {
        double ti = data->t[i];
        double yi = data->y[i];
        double y = gaussian(a, b, c, d, ti);

        gsl_vector_set(f, i, yi - y);
    }

    return GSL_SUCCESS;
}

int func_df(const gsl_vector * x, void *params, gsl_matrix * J)
{
    struct data *data = (struct data *) params;
    double a = gsl_vector_get(x, 0);
    double b = gsl_vector_get(x, 1);
    double c = gsl_vector_get(x, 2);
    // double d = gsl_vector_get(x, 3);
    size_t i;

    for (i = 0; i < data->n; ++i)
    {
        double ti = data->t[i];
        double zi = (ti - b);
        double ei = exp(-1 * zi * zi) / c;

        gsl_matrix_set(J, i, 0, -ei);
        gsl_matrix_set(J, i, 1, -(a / c) * ei * zi);
        gsl_matrix_set(J, i, 2, -(a / c) * ei * zi * zi);
    }

    return GSL_SUCCESS;
}

int func_fvv (const gsl_vector* x, const gsl_vector* v, void* params, gsl_vector* fvv)
{
    struct data *d = (struct data *) params;
    double a = gsl_vector_get(x, 0);
    double b = gsl_vector_get(x, 1);
    double c = gsl_vector_get(x, 2);
    double va = gsl_vector_get(v, 0);
    double vb = gsl_vector_get(v, 1);
    double vc = gsl_vector_get(v, 2);
    size_t i;

    for (i = 0; i < d->n; ++i)
    {
        double ti = d->t[i];
        double zi = (ti - b);
        double ei = exp(-1 * zi * zi) / c;
        double Dab = -zi * ei / c;
        double Dac = -zi * zi * ei / c;
        double Dbb = a * ei / (c * c) * (1.0 - zi*zi);
        double Dbc = a * zi * ei / (c * c) * (2.0 - zi*zi);
        double Dcc = a * zi * zi * ei / (c * c) * (3.0 - zi*zi);
        double sum;

        sum = 2.0 * va * vb * Dab +
              2.0 * va * vc * Dac +
                    vb * vb * Dbb +
              2.0 * vb * vc * Dbc +
                    vc * vc * Dcc;

        gsl_vector_set(fvv, i, sum);
    }

    return GSL_SUCCESS;
}

void callback(const size_t iter, void *params, const gsl_multifit_nlinear_workspace *w)
{
    gsl_vector *f = gsl_multifit_nlinear_residual(w);
    gsl_vector *x = gsl_multifit_nlinear_position(w);
    double avratio = gsl_multifit_nlinear_avratio(w);
    double rcond;

    (void) params; /* not used */

    /* compute reciprocal condition number of J(x) */
    gsl_multifit_nlinear_rcond(&rcond, w);

    fprintf(stderr, "iter %2zu: a = %.4f, b = %.4f, c = %.4f, |a|/|v| = %.4f cond(J) = %8.4f, |f(x)| = %.4f\n",
            iter,
            gsl_vector_get(x, 0),
            gsl_vector_get(x, 1),
            gsl_vector_get(x, 2),
            avratio,
            1.0 / rcond,
            gsl_blas_dnrm2(f));
}

void solve_system(gsl_vector* x, gsl_multifit_nlinear_fdf* fdf, gsl_multifit_nlinear_parameters* params)
{
    const gsl_multifit_nlinear_type *T = gsl_multifit_nlinear_trust;
    const size_t max_iter = 200;
    const double xtol = 1.0e-8;
    const double gtol = 1.0e-8;
    const double ftol = 1.0e-8;
    const size_t n = fdf->n;
    const size_t p = fdf->p;
    gsl_multifit_nlinear_workspace *work =
    gsl_multifit_nlinear_alloc(T, params, n, p);
    gsl_vector * f = gsl_multifit_nlinear_residual(work);
    gsl_vector * y = gsl_multifit_nlinear_position(work);
    int info;
    double chisq0, chisq, rcond;

    /* initialize solver */
    gsl_multifit_nlinear_init(x, fdf, work);

    /* store initial cost */
    gsl_blas_ddot(f, f, &chisq0);

    /* iterate until convergence */
    gsl_multifit_nlinear_driver(max_iter, xtol, gtol, ftol, callback, NULL, &info, work);

    /* store final cost */
    gsl_blas_ddot(f, f, &chisq);

    /* store cond(J(x)) */
    gsl_multifit_nlinear_rcond(&rcond, work);

    gsl_vector_memcpy(x, y);

    /* print summary */

    fprintf(stderr, "NITER         = %zu\n", gsl_multifit_nlinear_niter(work));
    fprintf(stderr, "NFEV          = %zu\n", fdf->nevalf);
    fprintf(stderr, "NJEV          = %zu\n", fdf->nevaldf);
    fprintf(stderr, "NAEV          = %zu\n", fdf->nevalfvv);
    fprintf(stderr, "initial cost  = %.12e\n", chisq0);
    fprintf(stderr, "final cost    = %.12e\n", chisq);
    fprintf(stderr, "final x       = (%.12e, %.12e, %12e)\n",
            gsl_vector_get(x, 0), gsl_vector_get(x, 1), gsl_vector_get(x, 2));
    fprintf(stderr, "final cond(J) = %.12e\n", 1.0 / rcond);

    gsl_multifit_nlinear_free(work);
}
