/*
 * Calculation of mean scaled and mean reduced scaled variables
 *
 */

#include "VMeanScaledVariables.h"

/*
 * Mean reduced scaled variable (e.g. mscw)
 *
*/
double VMeanScaledVariables::mean_reduced_scaled_variable(unsigned int ntel, float *data_value, float *mc_value, float *mc_sigma)
{
   double value = 0.;
   double weight = 0;

   if(ntel < 1)
   {
       return -9999.;
   }

    for( unsigned int t = 0; t < ntel; t++ )
    {
        if( mc_value[t] > -90. && mc_sigma[t] > -99. )
        {
            value += ( data_value[t] - mc_value[t] ) / mc_sigma[t] * (mc_value[t] * mc_value[t]) / ( mc_sigma[t] * mc_sigma[t] );
            weight += ( mc_value[t] * mc_value[t] ) / ( mc_sigma[t] * mc_sigma[t] );
        }
    }
    if( weight <= 0. )
    {
        return -9999.;
    }
    return value / weight;
}

double VMeanScaledVariables::mean_reduced_scaled_variable(unsigned int ntel, float *data_value, double *mc_value, double *mc_sigma)
{
    float f_mc_value[ntel];
    float f_mc_sigma[ntel];
    for(unsigned int t = 0; t < ntel; t++ )
    {
        f_mc_value[t] = (float)mc_value[t];
        f_mc_sigma[t] = (float)mc_sigma[t];
    }
    return VMeanScaledVariables::mean_reduced_scaled_variable(ntel, data_value, f_mc_value, f_mc_sigma );
}

/*
 * mean scaled variables (e.g. MWR)
 *
*/
double VMeanScaledVariables::mean_scaled_variable(unsigned int ntel, float *data, float *size, float *mc_value )
{
    double imr = 0.;
    double inr = 0.;
    // require size > 0 (to use only selected images for the MWR/MWL calculation)
    for( unsigned int t = 0; t < ntel; t++ )
    {
        if( data[t] > 0. && mc_value[t] && size[t] > 0. )
        {
            imr += data[t] / mc_value[t];
            inr++;
        }
    }
    if( inr > 0. )
    {
        return imr / inr;
    }
    return -9999.;
}

double VMeanScaledVariables::mean_scaled_variable(unsigned int ntel, float *data, float *size, double *mc_value )
{
    float f_mc_value[ntel];
    for(unsigned int t = 0; t < ntel; t++ )
    {
        f_mc_value[t] = (float)mc_value[t];
    }
    return VMeanScaledVariables::mean_scaled_variable(ntel, data, size, f_mc_value );
}
