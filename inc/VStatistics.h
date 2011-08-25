/*! LiAndMa
    \brief collection of useful analysis functions for significance and upper limit calculations
    \author
      Jamie Holder
      Gernot Maier

      Revision $Id: VStatistics.h,v 1.1.2.2.8.1.2.1 2010/03/08 07:49:52 gmaier Exp $
*/

#ifndef LIANDMA_C
#define LIANDMA_C

#include "TF1.h"
#include "TFeldmanCousins.h"
#include "TMath.h"
#include "TRolke.h"

#include "TCanvas.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TLegend.h"

#include <iostream>
#include <cmath>

using namespace std;

namespace VStatistics
{

    inline void liandma(double Non, double Noff, double alpha, double& Nsig, double& Sig5, double& Sig9, double& Sig17)
    {
        double alphasq;
        double oneplusalpha;
        double oneplusalphaoveralpha;

        double Ntot;

        if( alpha == 0. )
        {
            Sig17 = 0.;
            return;
        }

        alphasq=alpha*alpha;
        oneplusalpha=1.0+alpha;
        oneplusalphaoveralpha=oneplusalpha/alpha;

        Nsig   = Non - alpha*Noff;
        Ntot   = Non + Noff;

        if( Non+alphasq*Noff > 0. ) Sig5 = Nsig/sqrt(Non+alphasq*Noff);
        else                        Sig5 = 0.;
        if( alpha*Ntot > 0. )       Sig9 = Nsig/sqrt(alpha*Ntot);
        else                        Sig9 = 0.;
        if( Ntot == 0.)
        {
            Sig17=0.;
        }
        else if( Non == 0 && Noff != 0. )
        {
            Sig17 = sqrt(2.*( Noff*log(oneplusalpha*(Noff/Ntot)) ) );
        }
        else if( Non != 0 && Noff == 0. )
        {
            Sig17 = sqrt(2.*( Non *log(oneplusalphaoveralpha*(Non/Ntot)) ) );
        }
        else
        {
            Sig17 = 2.*( Non *log(oneplusalphaoveralpha*(Non/Ntot)) + Noff*log(oneplusalpha*(Noff/Ntot)) );
// value in brackets can be a small negative number
            if( TMath::Abs( Sig17) < 1.e-5 ) Sig17 = 0.;
            else                             Sig17 = sqrt( Sig17 );
        }
        if( Nsig < 0 ) Sig17=-Sig17;

        return;
    }

    inline double calcSignificance(double nOn, double nOff, double norm, int iLiMaForm = 17 )
    {
        double nSig = 0.;
        double limaSig5 = 0.;
        double limaSig9 = 0.;
        double limaSig17 = 0.;

        if( fabs( nOn ) < 1.e-5 && fabs( nOff ) < 1.e-5 ) return 0.;

        liandma(nOn,nOff,norm,
            nSig,limaSig5,limaSig9,limaSig17);
//  if( !isnormal( limaSig17 ) ) return 0.;

        if( iLiMaForm == 5 ) return limaSig5;
        if( iLiMaForm == 9 ) return limaSig9;

        return limaSig17;
    }

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
// upper limit calculations
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/*
    Gaussian approximation, equ 10 and following paragraph of Helene (1983)
*/
    inline double Helene( double nOn, double nOff, double alpha, double CL )
    {
        double nDiff = nOn - alpha * nOff;
        double sigma = sqrt( nOn + alpha * alpha * nOff );

        double ulim = 0.;

        if( sigma > 0. ) ulim = TMath::ErfInverse( 1. - ( (1.-CL) * TMath::Erfc( -1. * nDiff / sigma / sqrt( 2. ) ) ) ) * sigma * sqrt( 2. ) + nDiff;

        return ulim;
    }

    inline Double_t funcg(Double_t *x, Double_t *par)
    {
        double a=x[0];                            //!< upper limit
        double c = par[0];                        //!< on source counts
        double b = par[1]*par[2];                 //!< off source counts * ratio

        double mean = c-b;
        double g = 0.;
                                                  //!< Helene (6) Gaussian aprox.
        if( par[4] == 2 )      g = TMath::Exp(-((a-mean)*(a-mean))/(2.*(c+b)));
                                                  //!< Helene(2)
        else if( par[4] == 1 ) g = TMath::Exp(-(a+b))*TMath::Power((a+b),c);

        return g;
    }

    inline Double_t funcf(Double_t *x, Double_t *par)
    {
        double n1 = 1.;                           //!< Normalisation
        double p = 0.;                            //!< Probability
        double eps = 1E-5;                        //!< Accuracy
        double ul=x[0];                           //!< Upper limit
        double c=par[0];                          //<! Counts in the Peak Region
        double b=par[1]*par[2];                   //<! Background Events * ratio
        double alpha=par[3];                      //<! Confidence Level

        double big= 10.*sqrt(b+c) +fabs(c-b);     //! 10 sigma noise
        TF1 g("myfuncg",funcg,0.0,big,5);
        g.SetParameters(par);
        n1=g.Integral(0.,big,par,eps);
        if( n1 > 0. ) p=g.Integral(ul,big,par,eps)/n1;
        else
        {
            p = 0.;
        }

        double f   = alpha - p;
        return f;
    }

/*
    calculate upper limit using Helene or Feldman & Cousins

    iMethod == 0: Helene
    iMethod == 1: Helene eq 2
    iMethod == 2: Helene eq 6
    iMethod == 3: Feldman & Cousins
    iMethod == 4: Rolke Model 3
    iMethod == 5: Rolke Model 4
*/
    inline double calcUpperLimit(double nOn, double nOff,double ratio, double CL, int iMethod = 0, double method4_em = 1., double method4_sdem = 0.3, double method5_e = 1. )
    {
// Helene taking ratio into account
        if( iMethod == 0 ) return Helene( nOn, nOff, ratio, CL );
// Helene, equ (2) or Helene equ (6, Gaussian approximation)
        else if( iMethod == 1 || iMethod == 2 )
        {
            double mypars[5];
            mypars[0]=nOn;
            mypars[1]=nOff;
            mypars[2]=ratio;
            mypars[3]=1.-CL;
            mypars[4]=(double)iMethod;

//! ICP and JB agree this is correct:
            if (nOn<nOff) nOn=nOff;

            TF1 f( "myfuncf", funcf, 0.0, 100000, 5);
            f.SetParameters( mypars );
            return f.GetX( 0.0 ,0.0, 100000 );
        }
// Feldman & Cousins
        else if( iMethod == 3 )
        {
            TFeldmanCousins i_FeldmanCousins( CL );
            if( nOn > 20 || ratio * nOff > 20 ) i_FeldmanCousins.SetMuMax( 100. );
            if( nOn > 60 || ratio * nOff > 60 ) i_FeldmanCousins.SetMuMax( 100. );
            if( nOn > 40 ) i_FeldmanCousins.SetMuStep( 0.5 );
            return i_FeldmanCousins.CalculateUpperLimit( nOn, ratio * nOff );
        }
// Rolke Model 3 Background - Gaussian, Efficiency - Gaussian
//
// method4_em    efficiency
// method4_sdem  standard deviation of efficiency
        else if( iMethod == 4 )
        {
            TRolke i_Rolke;
            i_Rolke.SetCL( CL );

            double sdb = ratio*sqrt( nOff );
            return i_Rolke.CalculateInterval((int)nOn, 0, 0, ratio*nOff, method4_em, 0., 3, method4_sdem, sdb, 0., 0., 0 );
        }
// Rolke Model 4 Background - Poisson, Efficiency - known
// method5_e    efficiency
        else if( iMethod == 5 )
        {
            TRolke i_Rolke;
            i_Rolke.SetCL( CL );

            return i_Rolke.CalculateInterval((int)nOn, (int)nOff, 0, 0., 0., method5_e, 4, 0., 0., ratio, 0., 0 );
        }
        else
        {
            cout << "unknown upper limit method: " << iMethod << endl;
            return 0;
        }
        return 0.;
    }
}
#endif
