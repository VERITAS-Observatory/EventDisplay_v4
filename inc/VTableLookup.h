//! VTableLookup calculation of mean scaled variables and energies using MC filled tables

#ifndef VTABLELOOKUP
#define VTABLELOOKUP

#include "TDirectory.h"
#include "TError.h"
#include "TFile.h"
#include "TMath.h"
#include "TSystem.h"

#include "VStatistics.h"
#include "VTableLookupDataHandler.h"
#include "VTableLookupRunParameter.h"
#include "VTablesToRead.h"
#include "VTableCalculator.h"

#include <fstream>
#include <iostream>
#include <math.h>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class VTableLookup
{
    private:

        unsigned int fDebug;                     // print debug output
        bool fwrite;                        // true for table filling, false for table reading
        char freadwrite;                    // 'w' for table filling, 'r' for table reading
        VTableLookupDataHandler* fData;

        int fNumberOfIgnoredEvents;

        VTableLookupRunParameter* fTLRunParameter;

        int fNTel;

        vector< int > fTelComb;
        vector< int > fTelCombID;
        string fTelCombString;

        bool fUseMedianSizeforEnergyDetermination;

        TFile* fLookupTableFile;                  // root files with lookup tables
        TDirectory* fDirMSCW;
        TDirectory* fDirMSCL;
        TDirectory* fDirEnergySR;

        bool fWriteNoTriggerEvent;                // fill events with no triggers into the output tree
        bool fWrite1DHistograms;                  // write all 1D-histograms for median determination to disk

        string fMCtable;

        unsigned int fTableAzBins;
        vector< double > fTableAzLowEdge;
        vector< double > fTableAzUpEdge;

        int fnmscw;
        vector< bool > fTelToAnalyze;
        vector< double > fTableZe;      // [nze]
        vector< vector< double > > fTableZeOffset;  // [nze][nwoff]
        vector< vector< vector< vector< vector< double > > > > > fTableZeOffsetAzTelNoise;  // [nze][nwoff][naz][ntel][nnoise]

        // tables
        // fmscw[ze][woff][az][tel][noise]
        vector< vector< vector< vector< ULong64_t > > > > fTelType_tables;
        vector< vector< vector< vector< vector< VTableCalculator* > > > > > fmscw;
        vector< vector< vector< vector< vector< VTableCalculator* > > > > > fmscl;
        vector< vector< vector< vector< vector< VTableCalculator* > > > > > fenergySizevsRadius;

        // used for calculations
        VTableCalculator* f_calc_msc;
        VTableCalculator* f_calc_energySR;

        double fMeanNoiseLevel;
        vector< double > fNoiseLevel;             // pedestal variances per telescope from source file
        unsigned int fNNoiseLevelWarnings;

        void calculateMSFromTables( VTablesToRead* s, double esys );
        void configureTelescopeVector();
        bool cut( bool bWrite = false );  // apply cuts on successful reconstruction to input data
        void fillLookupTable();
        unsigned int  getAzBin( double az );
        void getIndexBoundary( unsigned int* ib, unsigned int* il, vector< double >& iV, double x );
        unsigned int  getNoiseBin( unsigned int ize, unsigned int iwoff, unsigned int iaz, unsigned int tel, double noise );
        vector< string > getSortedListOfDirectories( TDirectory* );
        void getTables( unsigned int inoise, unsigned int ize, unsigned int iwoff, unsigned int iaz, unsigned int tel, VTablesToRead* s );
        void interpolate( VTablesToRead* s1, double w1, VTablesToRead* s2, double w2, VTablesToRead* s, double w, bool iCos = false );
        void readLookupTable();
        void readNoiseLevel( bool bWriteToRunPara = true ); // read noise level from pedvar histograms of data files
        bool sanityCheckLookupTableFile( bool iPrint = false );

    public:
        VTableLookup( char readwrite, unsigned int iDebug = 0 );
        ~VTableLookup() {}
        double getMaxTotalTime()
        {
            return fData->getMaxTotalTime();
        }
        Long64_t  getNEntries()
        {
            if( fData )
            {
                return fData->getNEntries();
            }
            else
            {
                return 0;
            }
        }
        int    getNTel()
        {
            return fNTel;
        }
        bool   initialize( VTableLookupRunParameter* );
        void   loop();                              // loop over all events
        bool   setInputFiles( vector< string > iInputFiles ); // set input files from evndisp
        void   setMCTableFiles( string, string, string ); // set MC table file names (reading tables)
        void   setMCTableFiles( string, double, int, vector< double >, string, string, bool ); // set MC table file names (writing tables)
        void   setNEntries( int iN )
        {
            if( fData )
            {
                fData->setNEntries( iN );
            }
        }
        void   setNoTriggerEvents( bool iB )
        {
            fWriteNoTriggerEvent = iB;    // fill no trigger events into tree
        }
        void   setSpectralIndex( double iS );
        void   setTelescopeCombination( string iS )
        {
            fTelCombString = iS;
        }
        void   setUseMedianForEnergyDetermination( bool iB )
        {
            fUseMedianSizeforEnergyDetermination = iB;
        }
        void   setOutputFile( string outputfile, string writeoption, string tablefile ); // set file for output tree with mscw, energy, etc.
        void   setWrite1DHistograms( bool iB )
        {
            fWrite1DHistograms = iB;
        }
        void   terminate();

};
#endif
