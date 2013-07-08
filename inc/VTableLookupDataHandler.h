//! VTableLookupDataHandler data class for mscw and energy reconstruction

#ifndef VTableLookupDataHandler_H
#define VTableLookupDataHandler_H

#include "VEmissionHeightCalculator.h"
#include "VDeadTime.h"
#include "VEffectiveAreaCalculatorMCHistograms.h"
#include "VMonteCarloRunHeader.h"
#include "VTableLookupRunParameter.h"
#include "VUtilities.h"

#include "TChain.h"
#include "TChainElement.h"
#include "TDirectory.h"
#include "TError.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TList.h"
#include "TMath.h"
#include "TObjArray.h"
#include "TRandom3.h"

#include "Cshowerpars.h"
#include "Ctelconfig.h"
#include "Ctpars.h"

#include <bitset>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

class VTableLookupDataHandler
{
    private:
        unsigned int fDebug;  

	VTableLookupRunParameter *fTLRunParameter;  //! lookup parameters

        int fNMethods;                            //!< number of direction and core reconstruction methods in evndisp
        vector< string > finputfile;                        //!< input file name
        string foutputfile;                       //!< output file name
        TFile *fOutFile;                          //!< point to output file
        bool   fwrite;                            //!< true for table filling

        unsigned int fNTel;                       //!< number of telescopes
        unsigned int fNTelComb;                   //!< number of telescope combinations
        int fNEntries;                            //!< total number of events in input tree
        int fEventCounter;                        //!< event counter for input tree
        int fMethod;                              //!< which direction and core reconstruction method data should be used

        double fMaxTotalTime;                     //!< time to analyse in a run (in [s])
        double fTotalTime;                        //!< time passed in analysing the run (in [s])
        double fTotalTime0;                       //!< time of first event (in [s])

        VEmissionHeightCalculator *fEmissionHeightCalculator;

        double fSelectRandom;
        int fSelectRandomSeed;
        TRandom3 *fRandom;

// MC parameter
	unsigned int fMinImages;
	double fMC_distance_to_cameracenter_min;
	double fMC_distance_to_cameracenter_max;

        double fMCSpectralIndex;
        double fMCMinEnergy;
        double fMCMaxEnergy;
        double fSpectralIndex;

        double degrad;
        double raddeg;

        vector< double > fNoiseLevel;
        vector< double > fCurrentNoiseLevel;

// input trees
        int fEventDisplayFileFormat;
        TChain* fTshowerpars;
        Cshowerpars* fshowerpars;
        TChain* fTtelconfig;
        Ctelconfig* ftelconfig;
        vector< TChain* > fTtpars;
        vector< Ctpars* > ftpars;

        double fEventWeight;

// MC energy histograms
        TH1D* hE0mc;
        TH2D *hXYmc;
        TH2D *hDE0mc;
        TH2D *hWE0mc;
        TH1D *hZe;
        TH1D* hE0trig;
        TH2D *hXYtrig;
        TH2D *hDE0trig;
        TH2D *hWE0trig;
        TList *hisList;

        TH1D *hTrigPattern;                       //!< trigger pattern
        TH1D *hImagePattern;                      //!< pattern of telescopes with more than 4 pixels
        vector< string > sTrigPattern;            //!< label for trigger pattern histogram

// telescope positions from fTtelconfig
        double fTelX[VDST_MAXTELESCOPES];
        double fTelY[VDST_MAXTELESCOPES];
        double fTelZ[VDST_MAXTELESCOPES];
	double fFocalLength[VDST_MAXTELESCOPES];
        ULong64_t fTel_type[VDST_MAXTELESCOPES];
	map<ULong64_t, unsigned int > fList_of_Tel_type;
	map<ULong64_t, unsigned int >::iterator fList_of_Tel_type_iterator;
// telescope pointing
        double fTelElevation[VDST_MAXTELESCOPES];
        double fTelAzimuth[VDST_MAXTELESCOPES];
        double fTelDec[VDST_MAXTELESCOPES];
        double fTelRA[VDST_MAXTELESCOPES];
// target positions
        double fTargetElev;
        double fTargetAz;
        double fTargetDec;
        double fTargetRA;
        double fWobbleN;
        double fWobbleE;

// output trees
        TTree *fOTree;
        bool fShortTree;                          //!< use short version of output tree
        bool bWriteMCPars;

// cut statistics
	bool fEventStatus;
        unsigned int fNStats_All;
        unsigned int fNStats_Rec;
        unsigned int fNStats_NImagesCut;
        unsigned int fNStats_Chi2Cut;
        unsigned int fNStats_CoreErrorCut;
        unsigned int fNStats_WobbleCut;
        unsigned int fNStats_WobbleMinCut;
        unsigned int fNStats_WobbleMaxCut;

// dead time handler
        VDeadTime *fDeadTime;

        void   calcDistances( int );                //!< calculate distances between telescopes and shower core
        void   calcEmissionHeights();
        double calculateMeanNoiseLevel( bool bCurrentNoiseLevel = false );
	bool   checkIfFilesInChainAreRecovered( TChain *c );
	void   copyMCHistograms();
	bool   copyMCRunheader();
        void   copyMCTree();
	void   copy_telconfig();
        bool   fillNextEvent( bool bShort );
	void   printCutStatistics();
        bool   randomSelected();
        void   resetImageParameters();
        void   resetImageParameters( unsigned int i );
        void   setEventWeightfromMCSpectrum();
        void   setSelectRandom( double iX, int iS );
	void   writeDeadTimeHistograms();

    public:

//  data written to output file

        int runNumber;
        int eventNumber;
        int MJD;
        double time;

        bool fIsMC;                               //!< data is MC
	int    fMCPrimary;
        double fMCEnergy;                         //!< MC energy
        double fMCxcore;
        double fMCycore;
        double fMCxcore_SC;
        double fMCycore_SC;
        double fMCxcos;
        double fMCycos;
        double fMCaz;
        double fMCze;
        double fMCxoff;
        double fMCyoff;

        unsigned int LTrigS;
        ULong64_t LTrig;
        unsigned int fNTrig;
        int fNImages;
        unsigned int fImgSelS;
        ULong64_t fImgSel;
        bool fImgSel_list[VDST_MAXTELESCOPES];
	unsigned int fImgSel_list_short[VDST_MAXTELESCOPES];
	int fNTelTypes;
	unsigned int NImages_Ttype[VDST_MAXTELESCOPES];
        double fimg2_ang;
        double fZe;                               //!< zenith angle
        double fAz;
        double fRA;
        double fDec;
        double fXoff;
        double fYoff;
	double fXoff_derot;
	double fYoff_derot;
        double fstdS;
        double ftheta2;
        double fXcore;
        double fYcore;
        double fXcore_SC;
        double fYcore_SC;
        double fstdP;
        double fchi2;                             //!< chi2 from array reconstruction
//
// {1}
        double fMCEnergyArray [VDST_MAXTELESCOPES];
        float  fmeanPedvar_ImageT[VDST_MAXTELESCOPES];
        float  fmeanPedvar_Image;
        double fdist     [VDST_MAXTELESCOPES];
	double fdist_telType[VDST_MAXTELESCOPES];
        double fsize     [VDST_MAXTELESCOPES];
        double fsize2    [VDST_MAXTELESCOPES];
        double fsizeCorr [VDST_MAXTELESCOPES];
	double fsize_telType[VDST_MAXTELESCOPES];
        double floss     [VDST_MAXTELESCOPES];
        double fmax1     [VDST_MAXTELESCOPES];
        double fmax2     [VDST_MAXTELESCOPES];
        double fmax3     [VDST_MAXTELESCOPES];
        int    fmaxindex1     [VDST_MAXTELESCOPES];
        int    fmaxindex2     [VDST_MAXTELESCOPES];
        int    fmaxindex3     [VDST_MAXTELESCOPES];
        double fwidth    [VDST_MAXTELESCOPES];
	double fwidth_telType[VDST_MAXTELESCOPES];
        double flength   [VDST_MAXTELESCOPES];
	double flength_telType[VDST_MAXTELESCOPES];
        int    fntubes   [VDST_MAXTELESCOPES];
        unsigned short int fnsat[VDST_MAXTELESCOPES];
        unsigned short int fnlowgain[VDST_MAXTELESCOPES];
        double falpha    [VDST_MAXTELESCOPES];
        double flos      [VDST_MAXTELESCOPES];
        double fasym     [VDST_MAXTELESCOPES];
        double fcen_x    [VDST_MAXTELESCOPES];
        double fcen_y    [VDST_MAXTELESCOPES];
        double fcosphi   [VDST_MAXTELESCOPES];
        double fsinphi   [VDST_MAXTELESCOPES];
        double ftgrad_x  [VDST_MAXTELESCOPES];
        double ftchisq_x [VDST_MAXTELESCOPES];
        double fweight   [VDST_MAXTELESCOPES];    //!< always 1.
// {-1}
        double fR        [VDST_MAXTELESCOPES];    //!< distance from each telescope to reconstructed shower core
	double fR_telType[VDST_MAXTELESCOPES];    //!< distance from each telescope to reconstructed shower core (depending on tel type)
//        double fMCR        [VDST_MAXTELESCOPES];  //!< distance from each telescope to reconstructed shower core
//        double fMCR_telType[VDST_MAXTELESCOPES];  //!< distance from each telescope to reconstructed shower core
        double fE        [VDST_MAXTELESCOPES];    //!< energy assigned to each telescope (method 0)
        double fES       [VDST_MAXTELESCOPES];    //!< energy assigned to each telescope (method 1)
        double ftmscw    [VDST_MAXTELESCOPES];    //!< mscw assigned to each telescope
        double ftmscl    [VDST_MAXTELESCOPES];    //!< mscl assigned to each telescope
        float  ftmscw_sigma[VDST_MAXTELESCOPES];  //!< mscw  sigma  assigned to each telescope
        float  ftmscl_sigma[VDST_MAXTELESCOPES];  //!< mscl  sigma  assigned to each telescope

        int    fnmscw;                            //!< number of images used for mscw/mscl/energy calculation
        double fmscw;                             //!< mean scaled width
        double fmscl;                             //!< mean scaled length
        float  fmwr;                              //!< mean width ratio
        float  fmlr;                              //!< mean length ratio
        double fenergy;                           //!< reconstructed primary energy
        double fechi2;                            //!< chi2 from reconstructed primary energy
        double fdE;                               //!< dE from reconstructed primary energy
        float  fesys;                             //!< energy correction
        float  fesysVar;                          //!< energy correction (variance)
        float  fesysDist;                         //!< mean distance to maximum in emission height
        double fenergyS;                          //!< reconstructed primary energy
        double fechi2S;                           //!< chi2 from reconstructed primary energy
        double fdES;                              //!< dE from reconstructed primary energy

// emission height
        unsigned int fNTelPairs;
        float  fEmissionHeightMean;
        float  fEmissionHeightChi2;
                                                  //!< (note that maximum array length should be larger than MaxNbrTel
        float  fEmissionHeightT[VDST_MAXTELESCOPES];

        double fSizeSecondMax;                    //AMC 09102009
        double ftheta2_All[25];                   //AMC 10122009

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        VTableLookupDataHandler( bool iWrite, VTableLookupRunParameter* iT = 0 );
        ~VTableLookupDataHandler() {}

        bool cut()                                //!< apply cuts on successfull reconstruction to input data
        {
            return cut( false );
        }
        bool cut( bool bWrite );
        void fill();                              //!< fill output tree
        void fillMChistograms();
        void fillTables( bool ib ) { fwrite = ib; }
        int getArrayReconstructionMethod() { return fMethod; }
        double* getDistance() { return fdist; }
	double* getDistance( ULong64_t iTelType );
        double* getDistanceToCore() { return fR; }
	double* getDistanceToCore( ULong64_t iTelType );
        int    getEventNumber() { return eventNumber; }
	bool   getEventStatus() { return fEventStatus; }
        double getEventWeight() { return fEventWeight; }
        double getMCAz() { return fMCaz; }
        double getMCZe() { return fMCze; }
        double getAz() { return fAz; };
        double getMCDistance();
        double* getMCEnergyArray();
        double getMCEnergy() { return fMCEnergy; }
        double getMCWobbleOffset() { return sqrt( fMCxoff*fMCxoff + fMCyoff*fMCyoff ); }
        double* getE() { return fE; }
        int    getEventCounter() { return fEventCounter; }
        double *getLength() { return flength; }
	double *getLength( ULong64_t iTelType );
	map<ULong64_t, unsigned int > getList_of_Tel_type() { return fList_of_Tel_type; }
        unsigned int getNTel_type( ULong64_t t ) { if( fList_of_Tel_type.find(t) != fList_of_Tel_type.end() ) return fList_of_Tel_type[t]; else return 0; }
        unsigned int getMaxNbrTel() const { return VDST_MAXTELESCOPES; }
        int* getNtubes() { return fntubes; }
        double* getMSCWtel() { return ftmscw; }
        double* getMSCLtel() { return ftmscl; }
        double getMCMinEnergy() { return fMCMinEnergy; }
        double getMCMaxEnergy() { return fMCMaxEnergy; }
        double getMCSpectralIndex() { return fMCSpectralIndex; }
        double getSpectralIndex() { return fSpectralIndex; }
        int  getNEntries() { return fNEntries; }
        int  getNEvents() { if( fTshowerpars ) return fTshowerpars->GetEntries(); else return 0; }
        bool getNextEvent();                      //!< get next event from evndisp tree
        bool getNextEvent( bool bShort );         //!< get next event from evndisp tree
        double getMeanNoiseLevel( bool bCurrentNoiseLevel = false ) { if( !bCurrentNoiseLevel ) return calculateMeanNoiseLevel( false ); else return (double)fmeanPedvar_Image; }
        vector< double > getNoiseLevel( bool bCurrentNoiseLevel = false ) { if( !bCurrentNoiseLevel ) return fNoiseLevel; else return fCurrentNoiseLevel; }
        unsigned int getNTel() { return fNTel; }
        unsigned int getNTelTypes() { return fNTelTypes; }
        TFile* getOutputFile() { return fOutFile; }
        double* getSize( double iSizeCorrection = 1., bool iSelectedImagesOnly = false, bool iSize2 = false );
	double* getSize( double iSizeCorrection, ULong64_t iTelType, bool iSelectedImagesOnly, bool iSize2 = false );
        double* getSize2( double iSizeCorrection = 1., bool iSelectedImagesOnly = false ) { return getSize( iSizeCorrection, iSelectedImagesOnly, true ); }
	double* getSize2( double iSizeCorrection, ULong64_t iTelType, bool iSelectedImagesOnly ) 
	                   { return getSize( iSizeCorrection, iTelType, iSelectedImagesOnly, true ); }
        double *getWeight() { return fweight; }
        double *getWidth() { return fwidth; }
	double *getWidth( ULong64_t iTelType );
        double getMaxTotalTime() { return fMaxTotalTime; }
        double getTelElevation( unsigned int iTelID ) { return fTelElevation[iTelID]; }
        double getTelElevation();
	ULong64_t getTelType( unsigned int iTelID ) { return fTel_type[iTelID]; }
	unsigned int getTelType_arraycounter( unsigned int iTelID );      // return position of tel type for this telescope in array counter
        double getZe();
        double getTheta2() { return ftheta2; }
        double getWobbleOffset() { return sqrt( fXoff*fXoff + fYoff*fYoff ); }
        bool isReconstructed();
        bool readRunParameter();
        void reset();                             //!< reset a few output variables
        void resetAll();
        void setChi2( double iChi2, double iChi2S ) { fechi2 = iChi2; fechi2S = iChi2S; }
        void setdE( double idE, double idES ) { fdE = idE ; fdES = idES; }
                                                  //!< no check of boundaries!!
        void setDistanceToCore( int itel, double iR )
        {
            fR[itel] = iR;
        }
        void setEnergy( double iE, double iES ) { fenergy = iE; fenergyS = iES; }
        void setEnergyT( int i, double iET, double iETS ) { fE[i] = iET; fES[i] = iETS; }
        bool setInputFile( vector< string > );              //!< set input file
        void setMCMinEnergy( double iB ) { fMCMinEnergy = iB; }
        void setMCMaxEnergy( double iB ) { fMCMaxEnergy = iB; }
        void setMCSpectralIndex( double iB ) { fMCSpectralIndex = iB; }
        void setSpectralIndex( double iB ) { fSpectralIndex = iB; }
        void setMethod( int );                    //!< data from which evndisp reconstruction method should be used?
        void setMSCL( double iMSLC ) { fmscl = iMSLC; }
        void setMSCLT( int i, double iMSLC, float iMSLC_T = -99. ) { ftmscl[i] = iMSLC; ftmscl_sigma[i] = iMSLC_T; }
        void setNMSCW( int in ) { fnmscw = in; }
        void setMSCW( double iMSWC ) { fmscw = iMSWC; }
        void setMSCWT( int i, double iMSWC, float iMSWC_T = -99. ) { ftmscw[i] = iMSWC; ftmscw_sigma[i] = iMSWC_T; }
        void setMWR( double iM ) { fmwr = iM; }
        void setMLR( double iM ) { fmlr = iM; }
	void setMCDistanceToCameraCenter( double iMin, double iMax ) { fMC_distance_to_cameracenter_min = iMin; fMC_distance_to_cameracenter_max = iMax; }
        void setNEntries( int iN );
	void setDebug( bool iD ) { fDebug = iD; }
        bool setOutputFile( string, string, string );
        bool terminate( TNamed* );                //!< write everything to disk
};
#endif
