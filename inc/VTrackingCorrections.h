//! VTrackingCorections read tracking corrections from DB and apply corrections to pointing
// Revision $Id: VTrackingCorrections.h,v 1.1.2.1.12.1.16.1.2.1 2010/03/08 08:01:35 gmaier Exp $

#ifndef VTRACKINGCORRECTIONS_H
#define VTRACKINGCORRECTIONS_H

#include <TMath.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TSQLServer.h>

#include <iostream>
#include <stdlib.h>
#include <string>

#include "CorrectionParameters.h"
#include "VGlobalRunParameter.h"
#include "VDB_Connection.h"

using namespace std;
using namespace SEphem;

class VTrackingCorrections : public VGlobalRunParameter
{
    private:

        bool fStatus;

        unsigned int fTelID;

        CorrectionParameters *correctionParameters;

    public:

        VTrackingCorrections( unsigned int iTelID );
        ~VTrackingCorrections() {};
        bool   isGood() { return fStatus; }
        bool   readTrackingCorrectionsFromDB( string iSQLDate );
        bool   applyTrackingCorrections( double iElRaw, double iAzRaw, double &iElCorr, double &iElAzCorr );
};
#endif
