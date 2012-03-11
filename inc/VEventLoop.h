//! VEventLoop main event loop
// Revision $Id: VEventLoop.h,v 1.19.6.1.2.1.10.3.6.1.4.1.2.1.4.1.2.1.2.2.2.1.8.1 2011/04/21 10:03:38 gmaier Exp $

#ifndef VEVENTLOOP_H
#define VEVENTLOOP_H

#include <TApplication.h>
#include <TGClient.h>
#include <TQObject.h>
#include <TSystem.h>
#include <TTree.h>

#include <VImageAnalyzer.h>
#include <VArrayAnalyzer.h>
#include <VCalibrator.h>
#include <VEvndispData.h>
#include <VDST.h>
#ifndef NOVBF
#include <VGPSDecoder.h>
#include <VRawDataReader.h>
#include <VBFDataReader.h>
#endif
#include <VPedestalCalculator.h>
#include <VPEReader.h>
#include <VEvndispRunParameter.h>

#ifndef NOGSL
#include <VFrogs.h>
#endif

#include <iostream>
#include <map>
#include <string>

using namespace std;

class VEventLoop : public VEvndispData
{
    private:
//! run mode (analysis, pedestal calculation, gain/toffset calculation)
        enum E_runmode {R_ANA, R_PED, R_GTO, R_BCK, R_DST, R_GTOLOW, R_PEDLOW };

        VCalibrator *fCalibrator;                 //!< default calibration class
        VPedestalCalculator *fPedestalCalculator; //!< default pedestal calculator
        VImageAnalyzer *fAnalyzer;                     //!< default analyzer class
        VArrayAnalyzer *fArrayAnalyzer;           //!< default array analyzer
        VDST *fDST;                               //!< data summarizer
#ifndef NOGSL
        VFrogs *fFrogs;                           //!< Frogs Analyzer
#endif
        string fFileGains;                        //!< file with gains

        E_runmode fRunMode;                       //!< run mode (analysis, pedestal calculation, gain/toffset calculation)

        bool fAnalyzeMode;                        //!< used for gotoEvent (go through file without analyse events)
        bool bCheckTelescopePositions;
	vector< bool > fBoolPrintSample;

        bool fCutTelescope;                       //!< cuts apply only to one telescope
        int fNCutNArrayTrigger;                   //!< show only events with more than  fNCutNArrayTrigger triggered telescopes
        int fNCutArrayImages;                     //!< show only events with more than fNCutArrayImages images
        vector< int > fNCutNTrigger;              //!< show only events with more than fNCutNTrigger triggered pixels
        vector< string > fStringCut;              //!< string with cut options, TTree selection style
        bool fChangedCut;                         //!< check if any cut is applied

        int fBoolSumWindowChangeWarning;

        bool fNextEventStatus;                    //!< for stopping event loop from display

        int      analyzeEvent();                  //!< analyze current event
        int      checkArrayCuts();                //!< check cuts (see tab cut option) for current event
        int      checkCuts();                     //!< check cuts (see tab cut option) for current event
                                                  //!< check if T1 is on the right place
        void     checkTelescopePositions( int iMJD );
        void     fillTriggerVectors();
        void     setEventTimeFromReader();        //! calculate event time in appropriate format
        void     printRunInfos();                 //!< print some informations about current run
        void     terminate( int );

    public:
        VEventLoop( VEvndispRunParameter* );
        virtual ~VEventLoop();                    //!< destructor
        VImageAnalyzer*  getAnalyzer() { return fAnalyzer; }
        VArrayAnalyzer* getArrayAnalyzer() { return fArrayAnalyzer; }
        int         getCutNArrayTrigger() { return fNCutNArrayTrigger; }
        int         getCutNArrayImages() { return fNCutArrayImages; }
        int         getCutNTrigger() { return fNCutNTrigger[getTelID()]; }
        string      getCutString() { return fStringCut[getTelID()]; }
        string      getDataFileName() { return fRunPar->fsourcefile; }
        bool        getNextEventStatus() { return fNextEventStatus; }
        VVirtualDataReader* getReader() { return fReader; }
        void        gotoEvent( int iNumber );     //!< goto event iNumber
        bool        initEventLoop();              //!< init values for file names and analyzer
                                                  //!< init values for file names
        bool        initEventLoop( string iDataFile );
        void        initializeAnalyzers();        //!< initialize analyzers (call at first event)
        bool        loop( int );                  //!< analyse certain number of events
        bool        nextEvent();                  //!< goto next event and analyze it
        void        previousEvent();              //!< goto previous event (doesn't work yet)
        void        resetRunOptions();            //!< reset options to standard values
        void        setCutString( string );       //!< set cut string (from display)
        void        setCutNArrayTrigger( int );   //!< set minimal number of trigged telescopes
        void        setCutNArrayImages( int );    //!< set minimal number of images
        void        setCutNTrigger( int );        //!< set minimal number of trigged tubes
                                                  //!< cuts apply only to one telescope
        void        setCutSingleTelescope( bool ic )
        {
            fCutTelescope = ic;
        }
        void        setNextEventStatus( bool i_stat ) { fNextEventStatus = i_stat; }
        void        shutdown();                   //!< write out all results to root files
};
#endif                                            // VEVENTLOOP_H
