/*! \class VCalibrationData
     \brief all data concerning the calibration is stored here

     \date 19/08/04

     Revision $Id: VCalibrationData.cpp,v 1.14.8.2.10.4.2.7.4.9.2.2.2.2.4.4.2.2.2.5.2.5.2.4.2.3 2011/02/11 22:58:50 gmaier Exp $

     \author Gernot Maier
*/

#include <VCalibrationData.h>

VCalibrationData::VCalibrationData( unsigned int iTel, string iDir, string iPedfile, string iGainfile, string iTofffile,
                                    string iPedLowGainfile, string iGainLowGainfile, string iToffLowGainfile,
				    string iLowGainMultfile, string iTzerofile, string iTzeroLowGainfile )
{
    fUsePedestalsInTimeSlices = true;
    fLowGainUsePedestalsInTimeSlices = true;

    fTelID = iTel;
    fCalDirName = iDir;

    fFileName.push_back( iPedfile );                fHistoName.push_back( "pedestal" );
    fFileName.push_back( iGainfile );               fHistoName.push_back( "gain" );
    fFileName.push_back( iTofffile );               fHistoName.push_back( "toff" );
    fFileName.push_back( iPedLowGainfile );         fHistoName.push_back( "pedestal_lowGain" );
    fFileName.push_back( iGainLowGainfile );        fHistoName.push_back( "gain_lowGain" );
    fFileName.push_back( iToffLowGainfile );        fHistoName.push_back( "toff_lowGain" );
    fFileName.push_back( iLowGainMultfile );        fHistoName.push_back( "lowgain_mult" );
    fFileName.push_back( iTzerofile );              fHistoName.push_back( "tzero" );
    fFileName.push_back( iTzeroLowGainfile );       fHistoName.push_back( "tzero_lowGain" );

    for( unsigned int i = 0; i < fFileName.size(); i++ )
    {
       fFile.push_back( 0 );
    }


    fPedFromPLine = false;

    fReader = 0;

    fBoolLowGainPedestals = false;
    fBoolLowGainGains = false;
    fBoolLowGainTOff = false;

    fSumWindow = 0;
    setLowGainMultiplierFixedSummationWindow();

    fTS_ped_temp_time = 0.;

    setAverageTZero( 0., true );
    setAverageTZero( 0., false );

// summary histograms

    hisList = new TList();

    char hname[200];
    char hname_var[200];
    for( unsigned int i = 0; i < fFile.size(); i++ )
    {
       sprintf( hname, "h%s_%d", fHistoName[i].c_str(), iTel+1 );
       sprintf( hname_var, "h%s_var_%d", fHistoName[i].c_str(), iTel+1 );
       if( i == C_PED || i == C_PEDLOW )
       {
          fHisto_mean.push_back( new TH1F( hname, "", 200, 0., 50. ) );
	  fHisto_mean.back()->SetXTitle( "mean pedestal [dc]" );
	  fHisto_variance.push_back( new TH1F( hname_var, "", 200, 0., 50. ) );
	  fHisto_variance.back()->SetXTitle( "mean pedestal [dc]" );
       }
       else if( i == C_TOFF || i == C_TOFFLOW )
       {
          fHisto_mean.push_back( new TH1F( hname, "", 200, -20., 20. ) );
	  fHisto_mean.back()->SetXTitle( "time offset [sample]" );
	  fHisto_variance.push_back( new TH1F( hname_var, "", 200, 0., 10. ) );
	  fHisto_variance.back()->SetXTitle( "time offset variance [sample]" );
       }
       else if( i == C_GAIN || i == C_GAINLOW )
       {
          fHisto_mean.push_back( new TH1F( hname, "", 200, 0., 3. ) );
	  fHisto_mean.back()->SetXTitle( "relative gain" );
	  fHisto_variance.push_back( new TH1F( hname_var, "", 200, 0., 2. ) );
	  fHisto_variance.back()->SetXTitle( "relative gain variance" );
       }
       else if( i == C_LOWGAIN )
       {
          fHisto_mean.push_back( new TH1F( hname, "", 200, 0., 20. ) );
	  fHisto_mean.back()->SetXTitle( "low gain multiplier" );
	  fHisto_variance.push_back( new TH1F( hname_var, "", 200, 0., 20. ) );
	  fHisto_variance.back()->SetXTitle( "low gain multiplier variance" );
       }
       else if( i == C_TZERO || i == C_TZEROLOW )
       {
          fHisto_mean.push_back( new TH1F( hname, "", 200, 0., 20. ) );
	  fHisto_mean.back()->SetXTitle( "average pulse time t_{0} [sample]" );
	  fHisto_variance.push_back( new TH1F( hname_var, "", 200, 0., 20. ) );
	  fHisto_variance.back()->SetXTitle( "average pulse time t_{0} (variance) [sample]" );
       }

       hisList->Add( fHisto_mean.back() );
       hisList->Add( fHisto_variance.back() );
    }


}


void VCalibrationData::initialize( unsigned int i_channel, unsigned int nSamples, bool iUsePedestalsInTimeSlices, bool iLowGainUsePedestalsInTimeSlices, bool iDebug )
{
    if( iDebug ) cout << "VCalibrationData::initialize " << i_channel << "\t" << fTelID << endl;

    fUsePedestalsInTimeSlices = iUsePedestalsInTimeSlices;
    fLowGainUsePedestalsInTimeSlices = iLowGainUsePedestalsInTimeSlices;

    for( unsigned int i = 0; i < fHisto_mean.size(); i++ ) 
    {
       if( fHisto_mean[i] ) fHisto_mean[i]->Reset();
    }
    for( unsigned int i = 0; i < fHisto_variance.size(); i++ ) 
    {
       if( fHisto_variance[i] ) fHisto_variance[i]->Reset();
    }

    gErrorIgnoreLevel = 50000;
    char c_name[800];
    for( unsigned int i = 0; i < fFileName.size(); i++ )
    {
       if( fFileName[i].size() > 0 )
       {
	  sprintf( c_name, "%s.root", fFileName[i].c_str() );
	  fFile[i] = new TFile( c_name, "READ" );
	  if( fFile[i]->IsZombie() ) fFile[i] = 0;
       }
       else fFile[i] = 0;
    }

    gErrorIgnoreLevel = 0;

    fFADCStopOffsets.resize( i_channel, 0. );
    fChannelStatus.resize( i_channel, 1 );

    valarray< double > itemp_ped;
    valarray< valarray< double > > itemp_pedvars;
    itemp_pedvars.resize( nSamples+1, itemp_ped );

// (time dependent pedestal vectors are initialzed in VCalibrator)

// high gain channels
    fPeds.resize( i_channel, 20. );
    fVPedvars.resize( nSamples+1, sqrt( fPeds ) );
    fPedrms.resize( i_channel, 0. );
    fVmeanPedvars.resize( nSamples+1, 0. );
    fVmeanRMSPedvars.resize( nSamples+1, 0. );

    fLowGainPedsrms.resize( i_channel, 0. );

    fGains.resize( i_channel, 0. );
    fGains_DefaultSetting.resize( i_channel, true );
    fGainvars.resize( i_channel, 0. );

    fTOffsets.resize( i_channel, 0. );
    fTOffsetvars.resize( i_channel, 0. );

    fAverageTzero.resize( i_channel, 0. );
    fAverageTzerovars.resize( i_channel, 0. );

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
    fLowGainAverageTzero.resize( i_channel, 0. );
    fLowGainAverageTzerovars.resize( i_channel, 0. );

// low gain multiplier settings (per summation window)
    itemp_ped.resize( i_channel, 1. );
    fLowGainMultiplier.resize( nSamples+1, itemp_ped );
    itemp_ped = 0.;
    fLowGainMultiplierError.resize( nSamples+1, itemp_ped );
    fLowGainMultiplier_Mean.resize( nSamples+1, 1. );
    fLowGainMultiplier_RMS.resize( nSamples+1, 0. );

    if( iDebug ) cout << "END VCalibrationData::initialize " << endl;
}

TH1F* VCalibrationData::getHistogram( unsigned int iTel, unsigned int iChannel, unsigned int iWindowSize, VCalibrationData::E_PEDTYPE iType, ULong64_t iTelType )
{
    char iHName[200];
    std::ostringstream iSname;
    
    if( iType == C_PED || iType == C_PEDLOW )
    {
       sprintf( iHName, "distributions/hped_%d_%d_%d", iTel, iWindowSize, iChannel );
       if( iType < (int)fFile.size() && fFile[iType] )
       {
	 if( !fFile[iType]->Get( iHName ) )
	 {
	   sprintf( iHName, "distributions_%d/hped_%d_%d_%d", iTel+1, iTel, iWindowSize, iChannel );
         }
	 if( !fFile[iType]->Get( iHName ) && iTelType != 99999 )
	 {
	   iSname << "distributions_" << iTelType << "/hped_" << iTelType << "_" << iWindowSize << "_" << iChannel;
	   sprintf( iHName, "%s", iSname.str().c_str() );
         }
       }
    }
    else if( iType == C_TOFF || iType == C_TOFFLOW )
    {
       sprintf( iHName, "htoff_%d", iChannel );
    }
    else if( iType == C_TZERO || iType == C_TZEROLOW )
    {
       sprintf( iHName, "htzero_%d_%d", iTel+1, iChannel );
    }
    else if( iType == C_GAIN || iType == C_GAINLOW )
    {
       sprintf( iHName, "hgain_%d", iChannel );
    }
    if( iType < (int)fFile.size() && fFile[iType] )
    {
       return (TH1F*)fFile[iType]->Get( iHName );
    }

    return 0;
}


TH1F* VCalibrationData::getHistoPed( unsigned int iTel, unsigned int iChannel, unsigned int iWindowSize, bool iLowGain, ULong64_t iTelType )
{
    if( fReader && fPedFromPLine )
    {
        if( fReader->getDataFormat() == "grisu" ) return fReader->getPedHisto( iTel, iChannel );
        else return 0;
    }
    if( iLowGain ) return getHistogram( iTel, iChannel, iWindowSize, C_PEDLOW, iTelType );

    return getHistogram( iTel, iChannel, iWindowSize, C_PED, iTelType );
}


TH1F* VCalibrationData::getHistoGain( unsigned int iTel, unsigned int iChannel, bool iLowGain )
{
    if( iLowGain ) return getHistogram( iTel, iChannel, 0, C_GAINLOW );

    return getHistogram( iTel, iChannel, 0, C_GAIN );
}


TH1F* VCalibrationData::getHistoToff( unsigned int iTel, unsigned int iChannel, bool iLowGain )
{
    if( iLowGain ) return getHistogram( iTel, iChannel, 0, C_TOFFLOW );

    return getHistogram( iTel, iChannel, 0, C_TOFF );
}

TH1F* VCalibrationData::getHistoAverageTzero( unsigned int iTel, unsigned int iChannel, bool iLowGain )
{
    if( iLowGain ) return getHistogram( iTel, iChannel, 0, C_TZEROLOW );

    return getHistogram( iTel, iChannel, 0, C_TZERO );
}

TH1F*  VCalibrationData::getHistoDist( int iType, bool iDist )
{
   if( !iDist )
   {
      if( iType < (int)fHisto_variance.size() ) return fHisto_variance[iType];
   }
   else
   {
      if( iType < (int)fHisto_mean.size() ) return fHisto_mean[iType];
   }
   return 0;
}

double VCalibrationData::getAverageTZero( bool iLowGain )
{
    if( iLowGain ) return fAverageTZero_lowgain;

    return fAverageTZero_highgain;
}

void  VCalibrationData::setAverageTZero( double iAverageTzero, bool iLowGain )
{
   if( iLowGain ) fAverageTZero_lowgain = iAverageTzero;

   fAverageTZero_highgain = iAverageTzero;
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
       const unsigned int iMAXSUMWINDOWS = 1000;
       char hname[200];
       char htitle[200];
       sprintf( hname, "calib_%d", fTelID+1 );
       sprintf( htitle, "calibration data (Telescope %d)", fTelID+1 );
       int ipix = 0;
       double iped = 0.;
       double ipedvar = 0.;
       double igain = 0.;
       double igainvar = 0.;
       double itoff = 0.;
       double itoffvar = 0.;
       double itzero = 0.;
       double itzerovar = 0.;
       double ipedlowgain = 0.;
       double ipedvarlowgain = 0.;
       double igainMult[iMAXSUMWINDOWS];
       double igainMultE[iMAXSUMWINDOWS];
       double igainlowgain = 0.; 
       double igainvarlowgain = 0.;
       double itofflowgain = 0.;
       double itoffvarlowgain = 0.;
       double itzerolowgain = 0.;
       double itzerovarlowgain = 0.;
       unsigned int nsumwindows = 0;
       unsigned int sumwindow = 0;
       int istat = 0;
       int istatLow = 0;
       double pedvarV[iMAXSUMWINDOWS];
       double pedvarLowGainV[iMAXSUMWINDOWS];
       for( unsigned int i = 0; i < iMAXSUMWINDOWS; i++ )
       {
          pedvarV[i] = 0.;
	  pedvarLowGainV[i] = 0.;
	  igainMult[i] = 0.;
	  igainMultE[i] = 0.;
       }
       TTree fCalibrationTree( hname, htitle );
       fCalibrationTree.Branch( "pix", &ipix, "pix/I" );
       fCalibrationTree.Branch( "ped", &iped, "ped/D" );
       fCalibrationTree.Branch( "state", &istat, "state/I" );
       fCalibrationTree.Branch( "stateLow", &istatLow, "stateLow/I" );
       fCalibrationTree.Branch( "sumwindow", &sumwindow, "sumwindow/i" );
       fCalibrationTree.Branch( "pedvar", &ipedvar, "pedvar/D" );
       fCalibrationTree.Branch( "nsumwindows", &nsumwindows, "nsumwindows/i" );
       fCalibrationTree.Branch( "pedvarV", pedvarV, "pedvarV[nsumwindows]/D" );
       fCalibrationTree.Branch( "gain", &igain, "gain/D" );
       fCalibrationTree.Branch( "gainvar", &igainvar, "gainvar/D" );
       fCalibrationTree.Branch( "toff", &itoff, "toff/D" );
       fCalibrationTree.Branch( "toffvar", &itoffvar, "toffvar/D" );
       fCalibrationTree.Branch( "tzero", &itzero, "tzero/D" );
       fCalibrationTree.Branch( "tzerovar", &itzerovar, "tzerovar/D" );
       fCalibrationTree.Branch( "pedLowGain", &ipedlowgain, "pedLowGain/D" );
       fCalibrationTree.Branch( "pedvarLowGain", &ipedvarlowgain, "pedvarLowGain/D" );
       fCalibrationTree.Branch( "pedvarVLowGain", pedvarLowGainV, "pedvarVLowGain[nsumwindows]/D" );
       fCalibrationTree.Branch( "gainLowGain", &igainlowgain, "gainLowGain/D" );
       fCalibrationTree.Branch( "gainvarLowGain", &igainvarlowgain, "gainvarLowGain/D" );
       fCalibrationTree.Branch( "toffLowGain", &itofflowgain, "toffLowGain/D" );
       fCalibrationTree.Branch( "toffvarLowGain", &itoffvarlowgain, "toffvarLowGain/D" );
       fCalibrationTree.Branch( "tzeroLowGain", &itzerolowgain, "tzeroLowGain/D" );
       fCalibrationTree.Branch( "tzerovarLowGain", &itzerovarlowgain, "tzerovarLowGain/D" );
       fCalibrationTree.Branch( "gainMult", igainMult, "gainMult[nsumwindows]/D" );
       fCalibrationTree.Branch( "gainMultE", igainMultE, "gainMultE[nsumwindows]/D" );

       if( fPeds.size() == fPedrms.size() && fPeds.size() == fGains.size() && fPeds.size() == fGainvars.size()
        && fPeds.size() == fTOffsets.size() && fPeds.size() == fTOffsetvars.size() && fPeds.size() == fLowGainPeds.size() )
       {
	   for( unsigned int i = 0; i < fPeds.size(); i++ )
	   {
	       ipix = (int)i;
	       iped = fPeds[i];
	       if( i < iDead.size() ) istat = iDead[i];
	       if( i < iDeadLow.size() ) istatLow = iDeadLow[i];
	       sumwindow = fSumWindow;
	       if( sumwindow < fVPedvars.size() ) ipedvar = fVPedvars[sumwindow][i];
	       else                                    ipedvar = 0.;

	       nsumwindows = fVPedvars.size();
	       if( nsumwindows < iMAXSUMWINDOWS )
	       {
		   for( unsigned int s = 0; s < fVPedvars.size(); s++ ) pedvarV[s] = fVPedvars[s][i];
	       }  
	       igain = fGains[i];
	       fGains_DefaultSetting[i] = false;
	       igainvar = fGainvars[i];
	       itoff = fTOffsets[i];
	       itoffvar = fTOffsetvars[i];
	       itzero = fAverageTzero[i];
	       itzerovar = fAverageTzerovars[i];
	       ipedlowgain = fLowGainPeds[i]; 
	       if( sumwindow < fVLowGainPedvars.size() )      ipedvarlowgain = fVLowGainPedvars[sumwindow][i];
	       else                                           ipedvarlowgain = 0.;
	       if( nsumwindows < iMAXSUMWINDOWS )
	       {
		   for( unsigned int s = 0; s < fVLowGainPedvars.size(); s++ ) pedvarLowGainV[s] = fVLowGainPedvars[s][i];
	       } 
	       if( nsumwindows < iMAXSUMWINDOWS )
	       {
	          for( unsigned int s = 0; s < fLowGainMultiplier.size(); s++ )
		  {
		     if( i < fLowGainMultiplier[s].size() )      igainMult[s]  = fLowGainMultiplier[s][i];
	             if( i < fLowGainMultiplierError[s].size() ) igainMultE[s] = fLowGainMultiplierError[s][i];
                  }
               }
	       igainlowgain = fLowGainGains[i];
	       fLowGainGains_DefaultSetting[i] = false;
	       igainvarlowgain = fLowGainGainvars[i];
	       itofflowgain = fLowGainTOffsets[i];
	       itoffvarlowgain = fLowGainTOffsetvars[i];  
	       itzerolowgain = fLowGainAverageTzero[i];
	       itzerovarlowgain = fLowGainAverageTzerovars[i];

               fCalibrationTree.Fill();
           }
       }
       fCalibrationTree.Write();
// do not write histograms -> same information as in fCalibrationTree
//       hisList->Write();
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


valarray<double>& VCalibrationData::getPedvars( bool iLowGain, unsigned int iSW, double iTime )
{
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
// most DST styles are without time-dependent pedestals -> ped/pedvars vector have length 1
    if( iLowGain )
    {
        if( iSW < fVLowGainPedvars.size() )    return fVLowGainPedvars[iSW];
	else if( fVLowGainPedvars.size() > 0 ) return fVLowGainPedvars[0];
    }
    else
    {
       if( iSW < fVPedvars.size() )     return fVPedvars[iSW];
       else if( fVPedvars.size() > 0 ) return fVPedvars[0];
    }

// should never arrive here
    return fValArrayDouble;
}


void VCalibrationData::setPeds( unsigned int iChannel, double iPed, bool iLowGain, double iTime )
{
    if( iLowGain && iChannel < fLowGainPeds.size() ) fLowGainPeds[iChannel] = iPed;

    if( iChannel < fPeds.size() ) fPeds[iChannel] = iPed;

}


double VCalibrationData::getmeanPedvars( bool iLowGain, unsigned int iSW )
{
    if( !iLowGain && iSW < fVmeanPedvars.size() ) return fVmeanPedvars[iSW];

    if( iLowGain && iSW < fVmeanLowGainPedvars.size() ) return fVmeanLowGainPedvars[iSW];

    return 0.;
}


double VCalibrationData::getmeanRMSPedvars( bool iLowGain, unsigned int iSW )
{
    if( !iLowGain && iSW < fVmeanRMSPedvars.size() ) return fVmeanRMSPedvars[iSW];

    if( iLowGain && iSW < fVmeanRMSLowGainPedvars.size() ) return fVmeanRMSLowGainPedvars[iSW];

    return 0.;
}


void VCalibrationData::getmeanPedvars( double &imean, double &irms, bool iLowGain, unsigned int iSW, double iTime )
{
    imean = 0.;
    irms  = 0.;

    double its_n = 0.;
    double its_sum = 0.;
    double its_sum2 = 0.;

    double iMinPedVars = 2.5;
    if( iSW < 8 ) iMinPedVars = 0.5;

    valarray< double > ipedvar = getPedvars( iLowGain, iSW, iTime );

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

unsigned int VCalibrationData::setSummationWindow( unsigned int iSumWindow )
{
   if( fLowGainMultiplier_FixedSummationWindow == 9999 ) return iSumWindow;

   return fLowGainMultiplier_FixedSummationWindow;
}


bool VCalibrationData::setLowGainMultiplier( double iV, unsigned int iChannel, unsigned int iSumWindow )
{
    if( iSumWindow == 9999 )
    {
       for( unsigned int i = 0; i < fLowGainMultiplier.size(); i++ ) 
       {
          fLowGainMultiplier[i][iChannel] = iV;
       }
       return true;
    }
    iSumWindow = setSummationWindow( iSumWindow );
    if( iSumWindow < fLowGainMultiplier.size() && iChannel < fLowGainMultiplier[iSumWindow].size() )
    {
        fLowGainMultiplier[iSumWindow][iChannel]  = iV;
	return true;
    }
    return false;
}

bool VCalibrationData::setRMSLowGainMultiplier( double g, unsigned int iSumWindow )
{
   iSumWindow = setSummationWindow( iSumWindow );
   if( iSumWindow < fLowGainMultiplier_RMS.size() )
   {
      fLowGainMultiplier_RMS[iSumWindow] = g;
      return true;
   }
   else if( iSumWindow == 9999 )
   {
      for( unsigned int i = 0; i < fLowGainMultiplier_RMS.size(); i++ ) fLowGainMultiplier_RMS[i] = g;
      return true;
   }
   return false;
}

bool VCalibrationData::setMeanLowGainMultiplier( double g, unsigned int iSumWindow )
{
   iSumWindow = setSummationWindow( iSumWindow );
   if( iSumWindow < fLowGainMultiplier_Mean.size() )
   {
      fLowGainMultiplier_Mean[iSumWindow] = g;
      return true;
   }
   else if( iSumWindow == 9999 )
   {
      for( unsigned int i = 0; i < fLowGainMultiplier_Mean.size(); i++ ) fLowGainMultiplier_Mean[i] = g;
      return true;
   }
   return false;
}

double VCalibrationData::getMeanLowGainMultiplier( unsigned int iSumWindow )
{
   iSumWindow = setSummationWindow( iSumWindow );
   if( iSumWindow < fLowGainMultiplier_Mean.size() ) return fLowGainMultiplier_Mean[iSumWindow];

   return -1.;
}

double VCalibrationData::getRMSLowGainMultiplier( unsigned int iSumWindow )
{
   iSumWindow = setSummationWindow( iSumWindow );
   if( iSumWindow < fLowGainMultiplier_RMS.size() ) return fLowGainMultiplier_RMS[iSumWindow];

   return -1.;
}

valarray< double >& VCalibrationData::getLowGainMultiplier( unsigned int iSumWindow )
{
   iSumWindow = setSummationWindow( iSumWindow );
   if( iSumWindow < fLowGainMultiplier.size() ) return fLowGainMultiplier[iSumWindow];

   return fValArrayDouble;
}

valarray< double >& VCalibrationData::getLowGainMultiplierError( unsigned int iSumWindow )
{
   iSumWindow = setSummationWindow( iSumWindow );
   if( iSumWindow < fLowGainMultiplierError.size() ) return fLowGainMultiplierError[iSumWindow];

   return fValArrayDouble;
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

bool VCalibrationData::setLowGainMultiplierFixedSummationWindow( unsigned int iSumWindow )
{
   fLowGainMultiplier_FixedSummationWindow = iSumWindow;

   return true;
}
