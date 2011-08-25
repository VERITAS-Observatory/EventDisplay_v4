/*! \class VImageAnalyzerHistograms
    \brief  histogramming of run parameter, etc.

    \attention

    number of telescopes hardwired in history min/max for FADC stop channels in fillDiagnosticTree()
    number of channels hardwirded for telescope binning in init()

    Revision $Id: VImageAnalyzerHistograms.cpp,v 1.13.20.1.8.1.8.1.4.1 2010/03/08 07:39:50 gmaier Exp $

    \author
       Gernot Maier
*/

#include "VImageAnalyzerHistograms.h"

/*!
    \param iTel Telescope number
*/
VImageAnalyzerHistograms::VImageAnalyzerHistograms( unsigned int iTel )
{
    fTelescopeID = iTel;
    fNChannels = 499;

    fRunLength = 28. * 60.;                       // assume run length of 28 min

    hisList = new TList();
}


VImageAnalyzerHistograms::~VImageAnalyzerHistograms()
{
}


void VImageAnalyzerHistograms::init()
{

    char hisname[800];
    char histitle[800];

    sprintf( hisname, "fHimage" );
    sprintf( histitle, "image pixel (telescope %d)", fTelescopeID+1 );
    fHis_image = new TH2D( hisname, histitle, fNChannels, 0., (double)fNChannels, (int)(fRunLength/60.), 0., fRunLength );
    fHis_image->SetXTitle( "pixel #" );
    fHis_image->SetYTitle( "time since run start [s]" );
    hisList->Add( fHis_image );

    sprintf( hisname, "fHborder" );
    sprintf( histitle, "border pixel (telescope %d)", fTelescopeID+1 );
    fHis_border = new TH2D( hisname, histitle, fNChannels, 0., (double)fNChannels, (int)(fRunLength/60.), 0., fRunLength );
    fHis_border->SetXTitle( "pixel #" );
    fHis_border->SetYTitle( "time since run start [s]" );
    hisList->Add( fHis_border );

    sprintf( hisname, "fHSums" );
    sprintf( histitle, "pixel sums (telescope %d)", fTelescopeID+1 );
    fHis_sums = new TH1F( hisname, histitle, 1000, 0., 1000 );
    fHis_sums->SetXTitle( "pixel sum" );
    fHis_sums->SetYTitle( "events" );
    hisList->Add( fHis_sums );

    sprintf( hisname, "fHiLo" );
    sprintf( histitle, "high/low gain (telescope %d)", fTelescopeID+1 );
    fHis_hilo = new TH2D( hisname, histitle, fNChannels, 0., (double)fNChannels, (int)(fRunLength/60.), 0., fRunLength );
    fHis_hilo->SetXTitle( "pixel #" );
    fHis_hilo->SetYTitle( "time since run start [s]" );
    hisList->Add( fHis_hilo );

    sprintf( hisname, "fdiagno" );
    sprintf( histitle, "diagnostic tree (telescope %d)", fTelescopeID+1 );
    fdiagno = new TTree( hisname, histitle );
    fdiagno->SetAutoSave(100000000);

    fdiagno->Branch("runNumber", &runNumber, "runNumber/I" );
    fdiagno->Branch("eventNumber",  &eventNumber,  "eventNumber/I");
    fdiagno->Branch("MJD",  &MJD,  "MJD/I");
    fdiagno->Branch("Time",  &time,  "time/D");
    fdiagno->Branch("FADCstopTZero", fFADCstopTZero, "fFADCstopTZero[4]/D" );
    fdiagno->Branch("FADCstopSum", fFADCstopSum, "fFADCstopSum[4]/D" );
    hisList->Add( fdiagno );

    sprintf( hisname, "fDeadChannels" );
    sprintf( histitle, "dead channels (telescope %d)", fTelescopeID+1 );
    fDeadChannels = new TTree( hisname, histitle );
    fDeadChannels->SetAutoSave(100000000);

    fDeadChannels->Branch( "channel", &channel, "channel/I" );
    fDeadChannels->Branch( "dead", &dead, "dead/I" );
    fDeadChannels->Branch( "deadLow", &deadLow, "deadLow/I" );
    hisList->Add( fDeadChannels );
}


/*!
   \param iImage   image vector
   \param iBorder  border vector
   \param iSums     sum vector
   \param iTZeros   tzero vector
*/
void VImageAnalyzerHistograms::fill( vector< bool >& iImage, vector<bool>& iBorder, valarray<double>& iSums, vector<bool>& iHiLo, double itime )
{
    unsigned int n_s = 0;
    n_s = iImage.size();
    for( unsigned int i = 0; i < n_s; i++ )
    {
        if( iImage[i] )       fHis_image->Fill( i + 0.5, itime );
        else if( iBorder[i] ) fHis_border->Fill( i + 0.5, itime );
        if( iHiLo[i] )   fHis_hilo->Fill( i + 0.5, itime );
        if( iSums[i] )   fHis_sums->Fill( iSums[i] );
    }
}

/*
    fill diagnostic trees

    expect for FADC stop channels not more than 4 telescopes

*/
void VImageAnalyzerHistograms::fillDiagnosticTree( int rN, int eN, int iMJD, double it, vector< double >& iTZero, vector< double >& iFSum )
{
    runNumber = rN;
    eventNumber = eN;
    MJD = iMJD;
    time = it;

    if( iTZero.size() == 4 )
    {
        for( unsigned int i = 0; i < 4; i++ ) fFADCstopTZero[i] = iTZero[i];
    }
    else for( unsigned int i = 0; i < 4; i++ ) fFADCstopTZero[i] = 0.;
    if( iFSum.size() == 4 )
    {
        for( unsigned int i = 0; i < 4; i++ ) fFADCstopSum[i] = iFSum[i];
    }
    else for( unsigned int i = 0; i < 4; i++ ) fFADCstopSum[i] = 0.;

    if( fdiagno ) fdiagno->Fill();
}


/*!
   writing all histograms to disk

   outputfile is tree outputfile from VAnalyzer
*/
void VImageAnalyzerHistograms::terminate( TFile *outputfile )
{
    if( outputfile == 0 ) return;
    TDirectory *iDir = gDirectory;

// make histo directory
    if( !gDirectory->FindObject( "histograms" ) ) gDirectory->mkdir( "histograms" )->cd();

    hisList->Write();

    outputfile->cd();
    iDir->cd();
}


void VImageAnalyzerHistograms::fillDeadChannelTree( vector< unsigned int >&  iDead, vector< unsigned int >& iDeadLow )
{
    if( fDeadChannels == 0 ) return;

    for( unsigned int i = 0; i < iDead.size(); i++ )
    {
        channel = (int)i;
        dead = (int)iDead[i];
        deadLow = (int)iDeadLow[i];

        fDeadChannels->Fill();
    }
}
