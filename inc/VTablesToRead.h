//! VTablesToRead service class for lookup tables
// Revision $Id: VTablesToRead.h,v 1.1.2.1.2.1.12.2 2010/09/21 10:20:32 gmaier Exp $

#ifndef VTABLEREAD_H
#define VTABLEREAD_H

#include "TH2F.h"
#include "TH2D.h"

#include <iostream>
#include <vector>

using namespace std;

class VTablesToRead
{
    public:

        unsigned int    fNTel;
        vector< TH2F* > hmscwMedian;
        vector< TH2F* > hmscwSigma;
        vector< TH2F* > hmsclMedian;
        vector< TH2F* > hmsclSigma;
        vector< TH2F* > henergyERMedian;
        vector< TH2F* > henergyERSigma;
        vector< TH2F* > henergySRMedian;
        vector< TH2F* > henergySRSigma;

        double mscw;
        double mscl;
        double energyER;
        double energyER_Chi2;
	double energyER_dE;
        double energySR;
        double energySR_Chi2;
	double energySR_dE;
        double* mscw_T;
        double* mscl_T;
        double* energyER_T;
        double* energySR_T;
        double* mscw_Tsigma;
        double* mscl_Tsigma;
        double* energySR_Tsigma;

        VTablesToRead( int nTel );
        ~VTablesToRead() {}
        void reset();
};
#endif
