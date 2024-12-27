//! Mean scaled variables (e.g., mscw) calculations

#ifndef VMEANSCALEDVARIABLED
#define VMEANSCALEDVARIABLED

using namespace std;

namespace VMeanScaledVariables
{
    double mean_reduced_scaled_variable(unsigned int ntel, float *data, float *mc_value, float *mc_sigma);
    double mean_reduced_scaled_variable(unsigned int ntel, float *data, double *mc_value, double *mc_sigma);

    double mean_scaled_variable(unsigned int ntel, float *data, float *size, float *mc_value );
    double mean_scaled_variable(unsigned int ntel, float *data, float *size, double *mc_value );
}

#endif
