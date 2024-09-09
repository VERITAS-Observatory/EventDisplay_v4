//! VLightCurveData  data class for light curve calculations

#ifndef VLightCurveData_H
#define VLightCurveData_H

#include <string>
#include <vector>

#include <TObject.h>

#include "VFluxCalculation.h"

using namespace std;


class VLightCurveData : public TObject
{
    private:
    
        bool   bIsZombie;
        
    public:
    
        string fName;
        string fDataFileName;
        
        double fMJD_min;
        double fMJD_max;
        
        double fEnergy_min_TeV;
        double fEnergy_max_TeV;
        double fMinEnergy;
        double fMaxEnergy;
        double fE0;
        double fAlpha;
        
        vector< double > fRunList;
        double fMJD_Data_min;
        double fMJD_Data_max;
        double fPhase_Data_min;
        double fPhase_Data_max;
        double fRunTime;
        double fRunElevation;
        double fNon;
        double fNoff;
        double fNoffAlpha;
        double fSignificance;
        double fFlux;
        double fFluxErrorUp;
        double fFluxErrorDown;
        double fUpperFluxLimit;
        double fRunFluxCI_lo_1sigma;
        double fRunFluxCI_up_1sigma;
        double fRunFluxCI_lo_3sigma;
        double fRunFluxCI_up_3sigma;
        string fFluxState;
        
        
        VLightCurveData( string iName = "lightcurvedata" );
        VLightCurveData( const VLightCurveData& );
        ~VLightCurveData() {}
        bool   fillTeVEvndispData( string iAnaSumFile, double iThresholdSignificance = -99999., double iMinEvents = -9999.,
                                   double iUpperLimit = 0.99, int iUpperlimitMethod = 5, int iLiMaEqu = 17, double iMinEnergy = 0.,
                                   double E0 = 1., double alpha = -2.5, bool i_bRolke = false );
        double getFluxError();
        double getFluxErrorDown()
        {
            return fFluxErrorDown;
        }
        double getFluxErrorUp()
        {
            return fFluxErrorUp;
        }
        double getMJD();
        double getMJDError();
        double getPhase();
        double getPhaseError();
        bool   isZombie()
        {
            return bIsZombie;
        }
        void   setFluxCalculationEnergyInterval( double iEnergy_min_TeV = 1., double iEnergy_max_TeV = -1. );
        void   setFluxError( double iL = 0. );
        void   setMJDInterval( double iMJD_min, double iMJD_max )
        {
            fMJD_min = iMJD_min;
            fMJD_max = iMJD_max;
        }
        
        ClassDef( VLightCurveData, 6 );
};

class VLightCurveDataLessThan
{
    public:
    
        bool operator()( const VLightCurveData* a, const VLightCurveData* b )
        {
            if(!a || !b )
            {
                return false;
            }
            
            if( a->fMJD_Data_max != b->fMJD_Data_max )
            {
                return a->fMJD_Data_max < b->fMJD_Data_min;
            }
            
            return false;
        }
        VLightCurveDataLessThan() {}
        ~VLightCurveDataLessThan() {}
};

#endif
