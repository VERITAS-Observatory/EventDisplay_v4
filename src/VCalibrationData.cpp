/*! \class VCalibrationData
     \brief all data concerning the calibration is stored here

     \date 19/08/04

     Revision $Id: VCalibrationData.cpp,v 1.14.8.2.10.4.2.7.4.9.2.2.2.2.4.4.2.2.2.5.2.5.2.4.2.3 2011/02/11 22:58:50 gmaier Exp $

     \author Gernot Maier
*/

#include <VCalibrationData.h>

VCalibrationData::VCalibrationData( unsigned int iTel, string iDir, string iPedfile, string iGainfile, string iTofffile, string iPedLowGainfile, string iGainLowGainFile, string iToffLowGainfile, string iLowGainMultFile )
{
    fUsePedestalsInTimeSlices = true;
    fLowGainUsePedestalsInTimeSlices = true;

    fTelID = iTel;
    fCalDirName = iDir;
    fPedFileName = iPedfile;
    fGainFileName = iGainfile;
    fToffFileName = iTofffile;

    fLowGainPedFileName = iPedLowGainfile;
    fLowgainGainFileName = iPedLowGainfile;
    fLowGainToffFileName = iGainLowGainFile;
    fLowGainMultFileName = iLowGainMultFile;

    fPedFromPLine = false;

    fReader = 0;

    fPedFile = 0;
    fGainFile = 0;
    fToffFile = 0;
    fLowGainPedFile = 0;
    fLowGainGainFile = 0;
    fLowGainToffFile = 0;
    fLowGainMultFile = 0;

    fBoolLowGainPedestals = false;
    fBoolLowGainGains = false;
    fBoolLowGainTOff = false;

    fSumWindow = 0;
    fSumWindowSmall = 0;

    fTS_ped_temp_time = 0.;

// summary histograms

    hisList = new TList();

    char hname[200];
    sprintf( hname, "hPeds_%d", iTel+1 );
    fPedDistribution = new TH1F( hname, "", 200, 0., 50. );
    fPedDistribution->SetXTitle( "mean pedestal [dc]" );
    hisList->Add( fPedDistribution );

    sprintf( hname, "hPedsLowGain_%d", iTel+1 );
    fLowGainPedDistribution = new TH1F( hname, "", 200, 0., 50. );
    fLowGainPedDistribution->SetXTitle( "mean pedestal [dc]" );
    hisList->Add( fLowGainPedDistribution );

    sprintf( hname, "hPedvars_%d", iTel+1 );
    fPedvarsDistribution = new TH1F( hname, "", 200, 0., 50. );
    fPedvarsDistribution->SetXTitle( "pedestal variance [int dc]" );
    hisList->Add( fPedvarsDistribution );

    sprintf( hname, "hPedvarsLowGain_%d", iTel+1 );
    fLowGainPedvarDistribution = new TH1F( hname, "", 200, 0., 50. );
    fLowGainPedvarDistribution->SetXTitle( "pedestal variance [int dc]" );
    hisList->Add( fLowGainPedvarDistribution );

    sprintf( hname, "hToff_%d", iTel+1 );
    fTOffsetsDistribution = new TH1F( hname, "", 200, -20., 20. );
    fTOffsetsDistribution->SetXTitle( "time offset [sample]" );
    hisList->Add( fTOffsetsDistribution );

    sprintf( hname, "hToffvars_%d", iTel+1 );
    fTOffsetVarsDistribution = new TH1F( hname, "", 200, 0., 10. );
    fTOffsetVarsDistribution->SetXTitle( "time offset variance [sample]" );
    hisList->Add( fTOffsetVarsDistribution );

    sprintf( hname, "hGains_%d", iTel+1 );
    fGainsDistribution = new TH1F( hname, "", 200, 0., 3. );
    fGainsDistribution->SetXTitle( "relative gain" );
    hisList->Add( fGainsDistribution );

    sprintf( hname, "hGainvars_%d", iTel+1 );
    fGainVarsDistribution = new TH1F( hname, "", 200, 0., 2. );
    fGainVarsDistribution->SetXTitle( "relative gain variance" );
    hisList->Add( fGainVarsDistribution );

    sprintf( hname, "hLowGainToff_%d", iTel+1 );
    fLowGainTOffsetsDistribution = new TH1F( hname, "", 200, -20., 20. );
    fLowGainTOffsetsDistribution->SetXTitle( "time offset [sample]" );
    hisList->Add( fLowGainTOffsetsDistribution );

    sprintf( hname, "hLowGainToffvars_%d", iTel+1 );
    fLowGainTOffsetVarsDistribution = new TH1F( hname, "", 200, 0., 10. );
    fLowGainTOffsetVarsDistribution->SetXTitle( "time offset variance [sample]" );
    hisList->Add( fLowGainTOffsetVarsDistribution );

    sprintf( hname, "hLowGainGains_%d", iTel+1 );
    fLowGainGainsDistribution = new TH1F( hname, "", 200, 0., 3. );
    fLowGainGainsDistribution->SetXTitle( "relative gain" );
    hisList->Add( fLowGainGainsDistribution );

    sprintf( hname, "hLowGainGainvars_%d", iTel+1 );
    fLowGainGainVarsDistribution = new TH1F( hname, "", 200, 0., 2. );
    fLowGainGainVarsDistribution->SetXTitle( "relative gain variance" );
    hisList->Add( fLowGainGainVarsDistribution );

    sprintf( hname, "hLowGainmult_%d", iTel+1 );
    fLowGainMultDistribution = new TH1F( hname, "", 200, 0., 20. );
    fLowGainMultDistribution->SetXTitle( "low gain multiplier" );
    hisList->Add( fLowGainMultDistribution );

    sprintf( hname, "hLowGainmultSmall_%d", iTel+1 );
    fLowGainMultDistributionSmall = new TH1F( hname, "", 200, 0., 20. );
    fLowGainMultDistributionSmall->SetXTitle( "low gain multiplier" );
    hisList->Add( fLowGainMultDistributionSmall );

//   fCalibrationTree = 0;
}


void VCalibrationData::initialize( unsigned int i_channel, unsigned int nSamples, bool iUsePedestalsInTimeSlices, bool iLowGainUsePedestalsInTimeSlices, bool iDebug )
{
    if( iDebug ) cout << "VCalibrationData::initialize " << i_channel << "\t" << fTelID << endl;

    fUsePedestalsInTimeSlices = iUsePedestalsInTimeSlices;
    fLowGainUsePedestalsInTimeSlices = iLowGainUsePedestalsInTimeSlices;

    fPedDistribution->Reset();
    fPedvarsDistribution->Reset();
    fTOffsetsDistribution->Reset();
    fTOffsetVarsDistribution->Reset();
    fGainsDistribution->Reset();
    fGainVarsDistribution->Reset();

    fLowGainPedDistribution->Reset();
    fLowGainPedvarDistribution->Reset();
    fLowGainTOffsetsDistribution->Reset();
    fLowGainTOffsetVarsDistribution->Reset();
    fLowGainGainsDistribution->Reset();
    fLowGainGainVarsDistribution->Reset();

    fLowGainMultDistribution->Reset();
    fLowGainMultDistributionSmall->Reset();

    gErrorIgnoreLevel = 50000;
    char c_name[800];
    if( fPedFileName.size() > 0 )
    {
        sprintf( c_name, "%s.root", fPedFileName.c_str() );
        fPedFile = new TFile( c_name, "READ" );
        if( fPedFile->IsZombie() ) fPedFile = 0;
    }
    else fPedFile = 0;

    if( fLowGainPedFileName.size() > 0 )
    {
	sprintf( c_name, "%s.root", fLowGainPedFileName.c_str() );
        fLowGainPedFile = new TFile( c_name, "READ" );
        if( fLowGainPedFile->IsZombie() ) fLowGainPedFile = 0;
    }
    else fLowGainPedFile = 0;

    if( fGainFileName.size() > 0 )
    {
	sprintf( c_name, "%s.root", fGainFileName.c_str() );
        fGainFile = new TFile( c_name, "READ" );
        if( fGainFile->IsZombie() ) fGainFile = 0;
    }
    else fGainFile = 0;

    if( fLowgainGainFileName.size() > 0 )
    {
	sprintf( c_name, "%s.root", fGainFileName.c_str() );
        fLowGainGainFile = new TFile( c_name, "READ" );
        if( fLowGainGainFile->IsZombie() ) fLowGainGainFile = 0;
    }
    else fLowGainGainFile = 0;

    if( fToffFileName.size() > 0 )
    {
	sprintf( c_name, "%s.root", fToffFileName.c_str() );
        fToffFile = new TFile( c_name, "READ" );
        if( fToffFile->IsZombie() ) fToffFile = 0;
    }
    else fToffFile = 0;

    if( fLowGainToffFileName.size() > 0 )
    {
	sprintf( c_name, "%s.root", fLowGainToffFileName.c_str() );
        fLowGainToffFile = new TFile( c_name, "READ" );
        if( fLowGainToffFile->IsZombie() ) fLowGainToffFile = 0;
    }

    if( fLowGainMultFileName.size() > 0 )
    {
	sprintf( c_name, "%s.root", fLowGainMultFileName.c_str() );
        fLowGainMultFile = new TFile( c_name, "READ" );
        if( fLowGainMultFile->IsZombie() ) fLowGainMultFile = 0;
    }
    else fLowGainMultFile = 0;

    gErrorIgnoreLevel = 0;

    fFADCStopOffsets.resize( i_channel, 0. );
    fChannelStatus.resize( i_channel, 1 );

    valarray< double > itemp_ped;
    valarray< valarray< double > > itemp_pedvars;
    itemp_pedvars.resize( nSamples+1, itemp_ped );

// (time dependant pedestal vectors are initialzed in VCalibrator)

// high gain channels
    fPeds.resize( i_channel, 20. );
    fVPedvars.resize( nSamples+1, sqrt( fPeds ) );
    fPedrms.resize( i_channel, 0. );
    fVmeanPedvars.resize( nSamples+1, 0. );
    fVmeanRMSPedvars.resize( nSamples+1, 0. );

    fPedvarsPadded.resize( i_channel, 0. );
    fLowGainPedsrms.resize( i_channel, 0. );
    fPedvarsPaddedSmall.resize( i_channel, 0. );

    fGains.resize( i_channel, 0. );
    fGains_DefaultSetting.resize( i_channel, true );
    fGainvars.resize( i_channel, 0. );

    fTOffsets.resize( i_channel, 0. );
    fTOffsetvars.resize( i_channel, 0. );

// low gain channels
    fLowGainPeds.resize( i_channel, 20. );
    fVLowGainPedvars.resize( nSamples+1, fLowGainPeds );
    fVmeanLowGainPedvars.resize( nSamples+1, 0. );
    fVmeanRMSLowGainPedvars.resize( nSamples+1, 0. );

    fLowGainGains.resize( i_channel, 0. );
    fLowGainGains_DefaultSetting.resize( i_channel, 0. );
    fLowGainGainvars.resize( i_channel, 0. );

    fLowGainTOffsets.resize( i_channel, 0. );
    fLowGainTOffsetvars.resize( i_channel, 0. );

    fLowGainMult.resize( i_channel, 6. );
    fLowGainMultError.resize( i_channel, 0. );
    fmeanLowGainMult = 0.;
    frmsLowGainMult = 0.;
    fLowGainMultSmall.resize( i_channel, 6. );
    fLowGainMultErrorSmall.resize( i_channel, 0. );
    fmeanLowGainMultSmall = 0.;
    frmsLowGainMultSmall = 0.;

}


TH1F* VCalibrationData::getHistoPed( unsigned int iTel, unsigned int iChannel, unsigned int iWindowSize, bool iLowGain )
{
    char iHName[200];
    sprintf( iHName, "distributions/hped_%d_%d_%d", iTel, iWindowSize, iChannel );

    if( fReader && fPedFromPLine )
    {
        if( fReader->getDataFormat() == "grisu" ) return fReader->getPedHisto( iTel, iChannel );
        else return 0;
    }

    TFile *iFile = 0;
    if( !iLowGain ) iFile = fPedFile;
    else            iFile = fLowGainPedFile;

// no calibration file open
    if( !iFile ) return 0;
// get histogram from calibration file
    return (TH1F*)iFile->Get( iHName );
}


TH1F* VCalibrationData::getHistoGain( unsigned int iTel, unsigned int iChannel, bool iLowGain )
{
    TFile *iFile = 0;
    if( !iLowGain ) iFile = fGainFile;
    else            iFile = fLowGainGainFile;

    if( !iFile ) return 0;
    char iHName[200];
    sprintf( iHName, "hgain_%d", iChannel );
    return (TH1F*)iFile->Get( iHName );
}


TH1F* VCalibrationData::getHistoToff( unsigned int iTel, unsigned int iChannel, bool iLowGain )
{
    TFile *iFile = 0;
    if( !iLowGain ) iFile = fToffFile;
    else            iFile = fLowGainToffFile;

    if( !iFile ) return 0;
    char iHName[200];
    sprintf( iHName, "htoff_%d", iChannel );
    return (TH1F*)iFile->Get( iHName );
}


bool VCalibrationData::terminate( vector< unsigned int > iDead, vector< unsigned int > iDeadLow, bool iDST )
{
    TDirectory *iDir = gDirectory;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// no DST analysis
    if( !iDST )
    {
       if( !gDirectory->FindObject( "calibration" ) )
       {
	   TDirectory *iNewDir = gDirectory->mkdir( "calibration" );
	   if( iNewDir ) iNewDir->cd();
	   else
	   {
	       cout << "VCalibrationData::terminate: Error in creating calibration directory in: " << iDir->GetName() << endl;
	       iDir->cd();
	       return false;
	   }
       }

// fill tree with used calibration data
       char hname[200];
       char htitle[200];
       sprintf( hname, "calib_%d", fTelID+1 );
       sprintf( htitle, "calibration data (Telescope %d)", fTelID+1 );
       int ipix = 0;
       double iped = 0.;
       double ipedvar = 0.;
       double ipedvarsmall = 0.;
       double igain = 0.;
       double igainvar = 0.;
       double itoff = 0.;
       double itoffvar = 0.;
       double ipedlowgain = 0.;
       double ipedvarlowgain = 0.;
       double ipedvarlowgainsmall = 0.;
       double igainMult = 0.;
       double igainMultE = 0.;
       double igainMultSmall = 0.;
       double igainMultESmall = 0.;
       double igainlowgain = 0.; 
       double igainvarlowgain = 0.;
       double itofflowgain = 0.;
       double itoffvarlowgain = 0.;
       unsigned int sumwindow = 0;
       unsigned int sumwindowSmall = 0;
       int nsumwindows = 0;
       int istat = 0;
       int istatLow = 0;
       double pedvarV[1000];
       double pedvarLowGainV[1000];
       for( unsigned int i = 0; i < 1000; i++ ) pedvarV[i] = 0.;
       for( unsigned int i = 0; i < 1000; i++ ) pedvarLowGainV[i] = 0.;
       TTree fCalibrationTree( hname, htitle );
       fCalibrationTree.Branch( "pix", &ipix, "pix/I" );
       fCalibrationTree.Branch( "ped", &iped, "ped/D" );
       fCalibrationTree.Branch( "state", &istat, "state/I" );
       fCalibrationTree.Branch( "stateLow", &istatLow, "stateLow/I" );
       fCalibrationTree.Branch( "sumwindow", &sumwindow, "sumwindow/i" );
       fCalibrationTree.Branch( "sumwindowSmall", &sumwindowSmall, "sumwindowSmall/i" );
       fCalibrationTree.Branch( "pedvar", &ipedvar, "pedvar/D" );
       fCalibrationTree.Branch( "pedvarSmall", &ipedvarsmall, "pedvarSmall/D" );
       fCalibrationTree.Branch( "nsumwindows", &nsumwindows, "nsumwindows/I" );
       fCalibrationTree.Branch( "pedvarV", pedvarV, "pedvarV[nsumwindows]/D" );
       fCalibrationTree.Branch( "gain", &igain, "gain/D" );
       fCalibrationTree.Branch( "gainvar", &igainvar, "gainvar/D" );
       fCalibrationTree.Branch( "toff", &itoff, "toff/D" );
       fCalibrationTree.Branch( "toffvar", &itoffvar, "toffvar/D" );
       fCalibrationTree.Branch( "pedLowGain", &ipedlowgain, "pedLowGain/D" );
       fCalibrationTree.Branch( "pedvarLowGain", &ipedvarlowgain, "pedvarLowGain/D" );
       fCalibrationTree.Branch( "pedvarLowGainSmall", &ipedvarlowgainsmall, "pedvarLowGainSmall/D" );
       fCalibrationTree.Branch( "pedvarVLowGain", pedvarLowGainV, "pedvarVLowGain[nsumwindows]/D" );
       fCalibrationTree.Branch( "gainLowGain", &igainlowgain, "gainLowGain/D" );
       fCalibrationTree.Branch( "gainvarLowGain", &igainvarlowgain, "gainvarLowGain/D" );
       fCalibrationTree.Branch( "toffLowGain", &itofflowgain, "toffLowGain/D" );
       fCalibrationTree.Branch( "toffvarLowGain", &itoffvarlowgain, "toffvarLowGain/D" );
       fCalibrationTree.Branch( "gainMult", &igainMult, "gainMult/D" );
       fCalibrationTree.Branch( "gainMultE", &igainMultE, "gainMultE/D" );
       fCalibrationTree.Branch( "gainMultSmall", &igainMultSmall, "gainMultSmall/D" );
       fCalibrationTree.Branch( "gainMultESmall", &igainMultESmall, "gainMultESmall/D" );

       if( fPeds.size() == fPedrms.size() && fPeds.size() == fGains.size() && fPeds.size() == fGainvars.size() && fPeds.size() == fTOffsets.size() && fPeds.size() == fTOffsetvars.size() && fPeds.size() == fLowGainPeds.size() && fPeds.size() == fLowGainMult.size() && fPeds.size() == fLowGainMultError.size() && fPeds.size() == fLowGainMultSmall.size() && fPeds.size() == fLowGainMultErrorSmall.size() )
       {
	   for( unsigned int i = 0; i < fPeds.size(); i++ )
	   {
	       ipix = (int)i;
	       iped = fPeds[i];
	       if( i < iDead.size() ) istat = iDead[i];
	       if( i < iDeadLow.size() ) istatLow = iDeadLow[i];
	       sumwindow = fSumWindow;
	       sumwindowSmall = fSumWindowSmall;
	       if( sumwindow < fVPedvars.size() ) ipedvar = fVPedvars[sumwindow][i];
	       else                                    ipedvar = 0.;
	       if( sumwindowSmall < fVPedvars.size() ) ipedvarsmall = fVPedvars[sumwindowSmall][i];
	       else                                         ipedvarsmall = 0.; 

	       nsumwindows = (int)fVPedvars.size();
	       if( nsumwindows < 1000 )
	       {
		   for( unsigned int s = 0; s < fVPedvars.size(); s++ ) pedvarV[s] = fVPedvars[s][i];
	       }  
	       igain = fGains[i];
	       fGains_DefaultSetting[i] = false;
	       igainvar = fGainvars[i];
	       itoff = fTOffsets[i];
	       itoffvar = fTOffsetvars[i];
	       ipedlowgain = fLowGainPeds[i]; 
	       if( sumwindow < fVLowGainPedvars.size() )      ipedvarlowgain = fVLowGainPedvars[sumwindow][i];
	       else                                           ipedvarlowgain = 0.;
	       if( sumwindowSmall < fVLowGainPedvars.size() ) ipedvarlowgainsmall = fVLowGainPedvars[sumwindowSmall][i];
	       else                                           ipedvarlowgainsmall = 0.;
	       if( nsumwindows < 1000 )
	       {
		   for( unsigned int s = 0; s < fVLowGainPedvars.size(); s++ ) pedvarLowGainV[s] = fVLowGainPedvars[s][i];
	       } 
	       igainMult = fLowGainMult[i];
	       igainMultE = fLowGainMultError[i];
	       igainMultSmall = fLowGainMultSmall[i];
	       igainMultESmall = fLowGainMultErrorSmall[i];
	       igainlowgain = fLowGainGains[i];
	       fLowGainGains_DefaultSetting[i] = false;
	       igainvarlowgain = fLowGainGainvars[i];
	       itofflowgain = fLowGainTOffsets[i];
	       itoffvarlowgain = fLowGainTOffsetvars[i];  

               fCalibrationTree.Fill();
           }
       }
       fCalibrationTree.Write();
       hisList->Write();
    }

    iDir->cd();

    return true;
}


unsigned int VCalibrationData::getTSTimeIndex( double iTime, unsigned int &i1, unsigned int& i2, double& ifrac1, double& ifrac2 )
{
    unsigned iTS_size = getTimeTS_vector().size();

    i1 = 0;
    i2 = 0;
    ifrac1 = 1.0;
    ifrac2 = 0.;

    if( iTS_size > 0 )
    {
// time is before first time point
        if( iTime < getTimeTS_vector()[0] )
        {
            i1 = 0;
            i2 = 0;
            ifrac1 = 1.;
            ifrac2 = 0.;
        }
// time is after first time point
        else if( iTime > getTimeTS_vector()[iTS_size-1] )
        {
            i1 = iTS_size-1;
            i2 = iTS_size-1;
            ifrac1 = 0.;
            ifrac2 = 1.;
        }
// time is in between
        else
        {
// get index for closest time point
            unsigned int iIndex = 0;
            double iMaxDiff = 1.e15;
// time is smaller than closest time point
            for( unsigned int i = 0; i < iTS_size; i++ )
            {
                if( fabs( getTimeTS_vector()[i] - iTime ) < iMaxDiff )
                {
                    iMaxDiff = fabs( getTimeTS_vector()[i] - iTime );
                    iIndex = i;
                }
            }
// get indexes and fractions
            i1 = iIndex;
            if( iTime - getTimeTS_vector()[iIndex] > 0 )
            {
                if( iIndex < getTimeTS_vector().size() - 1 ) i2 = iIndex + 1;
                else                                         i2 = getTimeTS_vector().size() - 1;
            }
            else
            {
                if( iIndex > 0 )  i2 = iIndex - 1;
                else              i2 = 0;
            }

            double id = getTimeTS_vector()[i1] - getTimeTS_vector()[i2];
            if( id != 0. )
            {
                ifrac1 = 1. - ( getTimeTS_vector()[i1] - iTime ) / id;
                ifrac2 = 1. - ( iTime - getTimeTS_vector()[i2] ) / id;
            }
            else
            {
                ifrac1 = 1.;
                ifrac2 = 0.;
            }
        }
    }
    return 0;
}


valarray<double>& VCalibrationData::getPeds( bool iLowGain, double iTime )
{
// return time dependent pedestals
    if( usePedestalsInTimeSlices( iLowGain ) && iTime > 0. && getPedsTS_vector( iLowGain ).size() > 0. )
    {
        if( fabs( iTime - fTS_ped_temp_time ) < 1.e-3 )
        {
            fTS_ped_temp_time = iTime;
            return fTS_ped_temp;
        }
        fTS_ped_temp_time = iTime;

        unsigned int i1 = 0;
        unsigned int i2 = 0;
        double ifrac1 = 0.;
        double ifrac2 = 0.;
        getTSTimeIndex( iTime, i1, i2, ifrac1, ifrac2 );

        if( fTS_ped_temp.size() != getPedsTS_vector( iLowGain )[i1].size() ) fTS_ped_temp.resize( getPedsTS_vector( iLowGain )[i1].size(), 0. );

        for( unsigned int i = 0; i < fTS_ped_temp.size(); i++ )
        {
            fTS_ped_temp[i] = (ifrac1 * getPedsTS_vector( iLowGain )[i1][i] + ifrac2 * getPedsTS_vector( iLowGain )[i2][i] );
        }

        return fTS_ped_temp;
    }

    if( iLowGain ) return fLowGainPeds;

    return fPeds;
}


valarray<double>& VCalibrationData::getPedvars( bool iLowGain, unsigned int iSW, bool iSumWindowSmall, double iTime )
{
// no summation window given -> take global defined ones
    if( iSW == 0 && !iSumWindowSmall )    iSW = fSumWindow;
    else if( iSW ==0 && iSumWindowSmall ) iSW = fSumWindowSmall;

// most DST styles are without time-dependent pedestals -> ped/pedvars vector have length 1
    if( iLowGain )
    {
        if( iSW < fVLowGainPedvars.size() )      return fVLowGainPedvars[iSW];
	else if(  fVLowGainPedvars.size() == 1 ) return fVLowGainPedvars[0];
    }
    else
    {
       if( fVPedvars.size() == 1 ) return fVPedvars[0];
    }

//////////////////////////////////////////////////////////
// pedvars in time slices
//////////////////////////////////////////////////////////
    if( usePedestalsInTimeSlices( iLowGain ) && iTime > 0. && getPedvarsVTS_vector( iLowGain ).size() > 0. )
    {
// check validity of summation window
        if( iSW > 0 && iSW-1 < getPedvarsVTS_vector( iLowGain )[0].size() )
        {
// pedvars is available for this time (maybe filled by a previous call for another channel)
            if( fTS_pedvar_temp_time.find( iSW ) != fTS_pedvar_temp_time.end() && fabs( iTime - fTS_pedvar_temp_time[iSW] ) < 1.e-3 )
            {
                fTS_pedvar_temp_time[iSW] = iTime;
                return fTS_pedvar_temp[iSW];
            }
// no search for a pedvars for given time
            fTS_pedvar_temp_time[iSW] = iTime;

// get time index
            unsigned int i1 = 0;
            unsigned int i2 = 0;
            double ifrac1 = 0.;
            double ifrac2 = 0.;
            getTSTimeIndex( iTime, i1, i2, ifrac1, ifrac2 );

            if( fTS_pedvar_temp[iSW].size() != getPedvarsVTS_vector( iLowGain )[i1][iSW-1].size() ) fTS_pedvar_temp[iSW].resize( getPedvarsVTS_vector( iLowGain )[i1][iSW-1].size(), 0. );

// loop over all channels and calculate pedvars for this time (weighted mean between time bins)
            for( unsigned int i = 0; i < fTS_pedvar_temp[iSW].size(); i++ )
            {
                fTS_pedvar_temp[iSW][i] = (ifrac1 * getPedvarsVTS_vector( iLowGain )[i1][iSW-1][i] + ifrac2 * getPedvarsVTS_vector( iLowGain )[i2][iSW-1][i]);
            }

            return fTS_pedvar_temp[iSW];
        }
        else
        {
            cout << "VCalibrationData::getPedvars: invalid summation window for pedestal variations in time slices: " << iSW << " ";
            if( getPedvarsVTS_vector( iLowGain ).size() > 0 ) cout << getPedvarsVTS_vector( iLowGain )[0].size();
            cout << endl;
            exit( 0 );
        }
    }
//////////////////////////////////////////////////////////
// time independent pedestal variations
// low gain pedvars for summation window iSW
    if( iLowGain ) return fVLowGainPedvars[iSW]; 

// high gain pedvars for summation window iSW
    return fVPedvars[iSW];
}


void VCalibrationData::setPeds( unsigned int iChannel, double iPed, bool iLowGain, double iTime )
{
    if( iLowGain && iChannel < fLowGainPeds.size() ) fLowGainPeds[iChannel] = iPed;

    if( iChannel < fPeds.size() ) fPeds[iChannel] = iPed;

}


double VCalibrationData::getmeanPedvars( bool iLowGain, unsigned int iSW, bool iSumWindowSmall, double iTime )
{
    if( iSW == 0 && !iSumWindowSmall ) iSW = fSumWindow;
    if( iSW == 0 &&  iSumWindowSmall ) iSW = fSumWindowSmall;

    if( !iLowGain && iSW < fVmeanPedvars.size() ) return fVmeanPedvars[iSW];

    if( iLowGain && iSW < fVmeanLowGainPedvars.size() ) return fVmeanLowGainPedvars[iSW];

    return 0.;
}


double VCalibrationData::getmeanRMSPedvars( bool iLowGain, unsigned int iSW, bool iSumWindowSmall, double iTime )
{
    if( iSW == 0 && !iSumWindowSmall ) iSW = fSumWindow;
    if( iSW == 0 &&  iSumWindowSmall ) iSW = fSumWindowSmall;

    if( !iLowGain && iSW < fVmeanRMSPedvars.size() ) return fVmeanRMSPedvars[iSW];

    if( iLowGain && iSW < fVmeanRMSLowGainPedvars.size() ) return fVmeanRMSLowGainPedvars[iSW];

    return 0.;
}


void VCalibrationData::getmeanPedvars( double &imean, double &irms, bool iLowGain, unsigned int iSW, bool iSumWindowSmall, double iTime )
{
    imean = 0.;
    irms  = 0.;

    double its_n = 0.;
    double its_sum = 0.;
    double its_sum2 = 0.;

    double iMinPedVars = 2.5;
    if( iSumWindowSmall ) iMinPedVars = 0.5;

    valarray< double > ipedvar = getPedvars( iLowGain, iSW, iSumWindowSmall, iTime );

    for( unsigned int i = 0; i < ipedvar.size(); i++ )
    {
        if( ipedvar[i] > iMinPedVars )
        {
            its_n++;
            its_sum += ipedvar[i];
            its_sum2 += ipedvar[i] * ipedvar[i];
        }
    }
    if( its_n > 0. ) imean = its_sum / its_n;
    if( its_n > 1. ) irms = sqrt( 1./(its_n-1.) * ( its_sum2 - 1./its_n * its_sum * its_sum ) );
}


void   VCalibrationData::setLowGainMultiplier( double iV, unsigned int iChannel, bool iSmall )
{
    if( iSmall )
    {
        if( iChannel < fLowGainMultSmall.size() ) fLowGainMultSmall[iChannel]  = iV;
    }
    else
    {
        if( iChannel < fLowGainMult.size() ) fLowGainMult[iChannel]  = iV;
    }
}

/*!
    get smallest non-zero pedestal value
*/
double VCalibrationData::getPed_min( bool iLowGain )
{
    double i_min = 1.e6;
    for( unsigned int i = 0; i < getPeds( iLowGain ).size(); i++ )
    {
       if( getPeds( iLowGain )[i] > 0. && getPeds( iLowGain )[i] < i_min ) i_min = getPeds( iLowGain )[i];
    }
    return i_min;
}

/*!
    get largest non-zero pedestal value
*/
double VCalibrationData::getPed_max( bool iLowGain )
{
    double i_max = 0.;
    for( unsigned int i = 0; i < getPeds( iLowGain ).size(); i++ )
    {
       if( getPeds( iLowGain ) [i] > 0. && getPeds( iLowGain )[i] > i_max ) i_max = getPeds( iLowGain )[i];
    }
    return i_max;
}

void VCalibrationData::recoverLowGainPedestals()
{
   for( unsigned int i = 0; i < getNSummationWindows(); i++ )
   {
      double iM = 0.;
      double iN = 0.;
      for( unsigned int j = 0; j < getPedvars( true, i ).size(); j++ )
      {
         if( getPedvars( true, i )[j] > 0.1 ) 
	 {
	    iM += getPedvars( true, i )[j];
	    iN++;
         }
      }
// recover pedvars
      if( iN > 0. ) iM /= iN;
      if( iM > 0. )
      {
         for( unsigned int j = 0; j < getPedvars( true, i ).size(); j++ )
	 {
	    if( getPedvars( true, i )[j] < 0.1 )
	    {
	       getPedvars( true, i )[j] = iM;
            }
         }
      }
   }
}
