//! VTableLookup calculation of mean scaled variables and energies using MC filled tables
// Revision $Id: VTableLookup.h,v 1.20.2.10.4.2.12.8.4.3.2.1.2.3.6.1.2.1.2.8.4.2 2011/03/29 12:33:57 gmaier Exp $

#ifndef VTABLELOOKUP
#define VTABLELOOKUP

#include "TDirectory.h"
#include "TError.h"
#include "TFile.h"
#include "TMath.h"
#include "TSystem.h"

#include "VTableLookupDataHandler.h"
#include "VTableLookupRunParameter.h"
#include "VTablesToRead.h"
#include "VTableCalculator.h"
#include "VTableEnergyCalculator.h"

#include <fstream>
#include <iostream>
#include <math.h>
#include <set>
#include <string>

using namespace std;

class VTableLookup
{
    private:
        unsigned int fDebug;                     // print debug output
        bool fDetailedDebug;
        bool fwrite;                        // true for table filling, false for table reading
        char freadwrite;                    // 'w' for table filling, 'r' for table reading
        VTableLookupDataHandler *fData;

        int fNumberOfIgnoredeEvents;

        VTableLookupRunParameter *fTLRunParameter;

        int fevent;

        int fNTel;

        vector< int > fTelComb;
        vector< int > fTelCombID;
        string fTelCombString;

        bool fUseMedianSizeforEnergyDetermination;

        TFile *fLookupTableFile;                  // root files with lookup tables
        TDirectory *fDirMSCW;
        TDirectory *fDirMSCL;
        TDirectory *fDirEnergyER;
        TDirectory *fDirEnergySR;

        bool fWriteNoTriggerEvent;                // fill events with no triggers into the output tree
        bool fWrite1DHistograms;                  // write all 1D-histograms for median determination to disk

        string fMCtable;

        unsigned int fTableAzBins;
        vector< double > fTableAzLowEdge;
        vector< double > fTableAzUpEdge;

        bool bEcorr;                              // do energy correction for height of shower maximum
        int fnmscw;
        vector< bool > fTelToAnalyze;
        vector< double > fTableNoiseLevel;
        vector< vector< double > > fTableZe;      // fZe[nnoiselevel][nze]
                                                  // fDirectionOffset[nnoiselevel][nze][nwoff]
        vector< vector< vector< double > > > fTableDirectionOffset;

// tables
// fmscw[noise][ze][woff][az][tel]
	vector< vector< vector< vector< vector< ULong64_t > > > > > fTelType_tables;
        vector< vector< vector< vector< vector< VTableCalculator* > > > > > fmscw;
        vector< vector< vector< vector< vector< VTableCalculator* > > > > > fmscl;
        vector< vector< vector< vector< vector< VTableEnergyCalculator* > > > > > fenergyEnergyvsRadius;;
        vector< vector< vector< vector< vector< VTableCalculator* > > > > > fenergySizevsRadius;

// used for calculations
        VTableCalculator *f_calc_msc;
        VTableCalculator *f_calc_energySR;
        VTableEnergyCalculator *f_calc_energy;

        double fMaxDistance;
        double fMinSize;
        double fMaxLocalDistance;
        double fMinAngle;
        double fMaxCoreError;

        double fMeanNoiseLevel;
        vector< double > fNoiseLevel;             // pedestal variances per telescope from source file
        unsigned int fNNoiseLevelWarnings;

        VTablesToRead *s_NupZupWup;
        VTablesToRead *s_NupZupWlow;
        VTablesToRead *s_NupZup;
        VTablesToRead *s_NupZlowWup;
        VTablesToRead *s_NupZlowWlow;
        VTablesToRead *s_NupZlow;
        VTablesToRead *s_Nup;
        VTablesToRead *s_NlowZupWup;
        VTablesToRead *s_NlowZupWlow;
        VTablesToRead *s_NlowZup;
        VTablesToRead *s_NlowZlowWup;
        VTablesToRead *s_NlowZlowWlow;
        VTablesToRead *s_NlowZlow;
        VTablesToRead *s_Nlow;
        VTablesToRead *s_N;

        double degrad;                            // degree to rad
        double raddeg;

        void calculateMSFromTables( VTablesToRead *s, double esys );
        void configureTelescopeVector();
        bool cut()                                // apply cuts on successfull reconstruction to input data
        {
            return cut( false );
        }
        bool cut( bool bWrite );
        int getAzBin( double az );
        void getIndexBoundary( unsigned int *ib, unsigned int *il, vector< double >& iV, double x );
        void getTables( unsigned int inoise, unsigned int ize, unsigned int iwoff, unsigned int iaz, unsigned int tel, VTablesToRead *s );
        void interpolate( VTablesToRead *s1, double w1, VTablesToRead *s2, double w2, VTablesToRead *s, double w, bool iCos = false );
        double interpolate( double w1, double ze1, double w2, double ze2, double ze, bool iCos = false, double iLimitforInterpolation = 0.5 );
        bool isGamma();
        vector< string > getSortedListOfDirectories( TDirectory* );
	bool sanityCheckLookupTableFile( bool iPrint = false);       

    public:
        VTableLookup( char readwrite, unsigned int iDebug = 0 );
        ~VTableLookup() {}
        double getMinAngle() { return fMinAngle; }
        double getMaxCoreError() { if( fData ) return fData->getMaxCoreError(); else return 0.; }
        double getMaxTotalTime() { return fData->getMaxTotalTime(); }
        int  getNEntries() { if( fData ) return fData->getNEntries(); else return 0; }
	int  getNTel() { return fNTel; }
//      double interpolate_WL( double, double, double, double, double );           // interpolate with cos Ze weights between two values
        bool initialize( VTableLookupRunParameter* );
        void loop();                              // loop over all events
                                                  // read noise level from pedvar histograms of data files
        void readNoiseLevel( bool bWriteToRunPara = true );
        void setArrayReconstruction( int method );// set array reconstruction methods
        bool setInputFiles( string iInputFiles ); // set input files from evndisp
        void setEnergyCorrection( bool iEcorr )   // apply energy correction for shower maximum
        {
            bEcorr = iEcorr;
        }
        void setMaxDistance( double iD ) { fMaxDistance = iD; }
        void setMaxWobbleOffset( double iD ) { if( fData ) fData->setMaxWobbleOffset( iD ); }
        void setMaxLocalDistance( double iL ) { fMaxLocalDistance = iL; }
        void setMaxTotalTime( double iT, int iN )
        {
            if( fData )
            {
                fData->setMaxTotalTime( iT ); fData->setMaxEntries( iN );
            }
        }
        void setMinSize( double iD ) { fMinSize = iD; }
                                                  // set MC table file names (reading tables)
        void setMCTableFiles( string, string, string );
                                                  // set MC table file names (writing tables)
        void setMCTableFiles( string, double, int, int, string, string, bool );
        void setMCTree( bool iB ) { fData->setWriteMCTree( iB ); }
        void setMinAngle( double iA ) { fMinAngle = iA; }
        void setMaxCoreError( double iR ) { if( fData ) fData->setMaxCoreError( iR ); }
        void setNEntries( int iN ) { if( fData ) fData->setNEntries( iN ); }
        void setNoTriggerEvents( bool iB )        // fill no trigger events into tree
        {
            fWriteNoTriggerEvent = iB;
        }
        void setSelectRandom( double iX, int iS );
        void setShortTree( bool iB )              // write only a short version of the output tree to disk
        {
            fData->setShortTree( iB );
        }
        void setSpectralIndex( double iS );
        void setTelescopeCombination( string iS ) { fTelCombString = iS; }
        void setUseMedianForEnergyDetermination( bool iB ) { fUseMedianSizeforEnergyDetermination = iB; }
                                                  // set file for output tree with mscw, energy, etc.
        void setOutputFile( string outputfile, string writeoption, string tablefile );
        void setWrite1DHistograms( bool iB ) { fWrite1DHistograms = iB; }
        void terminate();

};
#endif
