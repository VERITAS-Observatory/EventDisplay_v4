// Anasum run summary tree definition.

#ifndef CRunSummary_h
#define CRunSummary_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

class CRunSummary : public TObject
{
    public :
        TTree*          fChain;                   //!pointer to the analyzed TTree or TChain
        Int_t           fCurrent;                 //!current Tree number in a TChain

        // Declaration of leave types
        Int_t           runOn;
        Int_t           runOff;
        Double_t        MJDOn;
        Double_t        MJDOff;
        Double_t        MJDrunstart;
        Double_t        MJDrunstop;
        Char_t          TargetName[300];
        Double_t        TargetRA;
        Double_t        TargetDec;
        Double_t        TargetRAJ2000;
        Double_t        TargetDecJ2000;
        Double_t        SkyMapCentreRAJ2000;
        Double_t        SkyMapCentreDecJ2000;
        Double_t        TargetShiftRAJ2000;
        Double_t        TargetShiftDecJ2000;
        Double_t        TargetShiftWest;
        Double_t        TargetShiftNorth;
        Double_t        WobbleNorth;
        Double_t        WobbleWest;
        UInt_t          NTel;
        Char_t          TelList[300];
        Double_t        tOn;
        Double_t        tOff;
        Double_t        elevationOn;
        Double_t        azimuthOn;
        Double_t        elevationOff;
        Double_t        azimuthOff;
        Double_t        Theta2Max;
        Double_t        RawRateOn;
        Double_t        RawRateOff;
        Double_t        pedvarsOn;
        Double_t        pedvarsOff;
        Double_t        NOn;
        Double_t        NOff;
        Double_t        NOffNorm;
        Double_t        OffNorm;
        Double_t        Signi;
        Double_t        Rate;
        Double_t        RateE;
        Double_t        RateOff;
        Double_t        RateOffE;
        Double_t        DeadTimeFracOn;
        Double_t        DeadTimeFracOff;
        Double_t        MaxSigni;
        Double_t        MaxSigniX;
        Double_t        MaxSigniY;

        // List of branches
        TBranch*        b_runOn;                  //!
        TBranch*        b_runOff;                 //!
        TBranch*        b_MJDOn;                  //!
        TBranch*        b_MJDOff;                 //!
        TBranch*        b_MJDrunstart;   //!
        TBranch*        b_MJDrunstop;   //!
        TBranch*        b_TargetName;   //!
        TBranch*        b_TargetRA;               //!
        TBranch*        b_TargetDec;              //!
        TBranch*        b_TargetRAJ2000;          //!
        TBranch*        b_TargetDecJ2000;         //!
        TBranch*        b_SkyMapCentreRAJ2000;    //!
        TBranch*        b_SkyMapCentreDecJ2000;   //!
        TBranch*        b_TargetShiftRAJ2000;     //!
        TBranch*        b_TargetShiftDecJ2000;    //!
        TBranch*        b_TargetShiftWest;        //!
        TBranch*        b_TargetShiftNorth;       //!
        TBranch*        b_WobbleNorth;            //!
        TBranch*        b_WobbleWest;             //!
        TBranch*        b_NTel;                   //!
        TBranch*        b_TelList;   //!
        TBranch*        b_tOn;                    //!
        TBranch*        b_tOff;                   //!
        TBranch*        b_elevationOn;            //!
        TBranch*        b_azimuthOn;              //!
        TBranch*        b_elevationOff;           //!
        TBranch*        b_azimuthOff;             //!
        TBranch*        b_Theta2Max;   //!
        TBranch*        b_RawRateOn;              //!
        TBranch*        b_RawRateOff;             //!
        TBranch*        b_pedvarsOn;              //!
        TBranch*        b_pedvarsOff;             //!
        TBranch*        b_NOn;                    //!
        TBranch*        b_NOff;                   //!
        TBranch*        b_NOffNorm;               //!
        TBranch*        b_OffNorm;                //!
        TBranch*        b_Signi;                  //!
        TBranch*        b_Rate;                   //!
        TBranch*        b_RateE;                  //!
        TBranch*        b_RateOff;                //!
        TBranch*        b_RateOffE;               //!
        TBranch*        b_DeadTimeFracOn;         //!
        TBranch*        b_DeadTimeFracOff;        //!
        TBranch*        b_MaxSigni;               //!
        TBranch*        b_MaxSigniX;              //!
        TBranch*        b_MaxSigniY;              //!

        CRunSummary( TTree* tree = 0 );
        virtual ~CRunSummary();
        virtual Int_t    GetEntry( Long64_t entry );
        virtual Long64_t LoadTree( Long64_t entry );
        virtual void     Init( TTree* tree );
        virtual Bool_t   Notify();

        ClassDef( CRunSummary, 1 );
};
#endif
