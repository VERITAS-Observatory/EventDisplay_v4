/*! \class VCalibrator
    \brief calibration class, calculation of pedestals, gains, ...

    \date 19/08/04

    Revision $Id: VCalibrator.cpp,v 1.45.2.2.2.1.2.6.10.13.2.12.4.8.4.5.2.2.2.3.2.4.2.3.2.10.2.10.2.5.2.1 2011/04/21 10:03:36 gmaier Exp $

    \author Jamie Holder
    \author Gernot Maier
*/

#include "VCalibrator.h"

VCalibrator::VCalibrator()
{
    setCalibrated( false );
    fRaw = true;

    fGainTree = 0;
    fToffTree = 0;

    fCalibrationfileVersion = 1;
}


/*!

 */
void VCalibrator::calculatePedestals( bool iLowGain )
{
    if( getDebugFlag() ) cout << "void VCalibrator::calculatePedestals()" << endl;

    string ioutfile;
    char ic[800];
// init histograms etc (only done before first event)
    if( fReader->getMaxChannels() && hped_vec.size() == 0 )
    {
// create histograms and  root output file for all telescopes
        if( hped_vec.size() == 0 )
        {
// create histogram, one for each telescope, channel and sumwindow
// loop over all telescopes
            int telID = 0;
            for( unsigned int t = 0; t < getNTel(); t++ )
            {
                telID = t+1;
                vector< vector<TH1F* > > iped_vec;
// loop over all sumwindows
                for( int i = 0; i < getRunParameter()->fCalibrationSumWindow; i++ )
                {
                    vector<TH1F* > ihped;
// loop over all channels
                    for( unsigned int j = 0; j < getNChannels(); j++ )
                    {
                        char pedkey[100];
                        sprintf(pedkey,"hped_%d_%d_%d",t,i+1,j);
                        double min=0.;
                        double max=50.*((double)i+1.);
                        sprintf( ic, "ped distribution (tel %d, channel %d, sumwindow %d)", telID, j, i+1 );
                        ihped.push_back( new TH1F(pedkey, ic,(int)(max-min),min,max) );
                    }
                    iped_vec.push_back( ihped );
                }
                hped_vec.push_back( iped_vec );
            }
// root output file = pedfilename.root
            for( unsigned int tel = 0; tel < getTeltoAna().size(); tel++ )
            {
		unsigned int i = getTeltoAna()[tel];
                if( !iLowGain ) ioutfile = fPedFileNameC[i] + ".root";
                else            ioutfile = fLowGainPedFileNameC[i] + ".root";

                fPedOutFile.push_back( new TFile( ioutfile.c_str(), "RECREATE") );
                if( fPedOutFile.back()->IsZombie() )
                {
                    cout << "VCalibrator::calculatePedestal error in creating pedestal file: ";
                    cout << ioutfile << endl;
                    exit( -1 );
                }
            }
        }
        setCalibrated();
    }
    resetAnaData();

    fillHiLo();

// calculate pedestals for current telescope
    fNumberPedestalEvents[fTelID]++;
// loop over all sumwindows
    for( unsigned int i = 0; i < hped_vec[fTelID].size(); i++ )
    {
        calcSums( fRunPar->fCalibrationSumFirst, fRunPar->fCalibrationSumFirst+(i+1), true, iLowGain );
// loop over all channels
        for(unsigned int j=0;j<hped_vec[fTelID][i].size();j++)
        {
// exclude low gain channels from pedestal calculation
            if( iLowGain && !getHiLo()[j] ) continue;
            else if( !iLowGain && getHiLo()[j] ) continue;
// calculate pedestals only for channels with valid hitbit
//   important if trying to generate pedestals for zero supressed
//   runs without injected pedestal events
            if( fRunPar->fRunIsZeroSuppressed )
            {
                if( fReader->getChannelHitIndex(j).first ) hped_vec[fTelID][i][j]->Fill( getSums()[j] );
            }
            else hped_vec[fTelID][i][j]->Fill( getSums()[j] );
        }
    }
}


void VCalibrator::writePeds( bool iLowGain )
{
    writePeds( iLowGain, 0 );
}


void VCalibrator::writePeds( bool iLowGain, VPedestalCalculator *iPedestalCalculator )
{
    if( getDebugFlag() ) cout << "void VCalibrator::writePeds()" << endl;
    if( fPedOutFile.size() < 1 || !fPedOutFile[0] )
    {
        cout << "void VCalibrator::writePeds() error: no output file defined" << endl;
        cout << "probably no events for pedestal calculation available" << endl;
        return;
    }
    string ioutfile;
// loop over all telescopes
    for( unsigned int tel = 0; tel < getTeltoAna().size(); tel++ )
    {
        unsigned int t = getTeltoAna()[tel];
// make one pedestal file per telescope
        if( !iLowGain ) ioutfile = fPedFileNameC[t];
        else            ioutfile = fLowGainPedFileNameC[t];
        ofstream os( ioutfile.c_str() );
        if( !os )
        {
            cout << "VCalibrator::writePeds(): ERROR, unable to write pedestals to " << ioutfile << " (" << iLowGain << ")" << endl;
            exit( 0 );
        }
        cout << "Telescope " << t+1 << endl;
        cout << "\t total number of pedestal events: " << fNumberPedestalEvents[t] << endl;
        cout << "\t writing ";
        if( iLowGain ) cout << " low ";
	else           cout << " high ";
	cout << " gain pedestals to " << endl;
        cout << "\t\t" << ioutfile << endl;
        cout << "\t\t" << fPedOutFile[tel]->GetName() << endl;
// loop over all channels
        for(unsigned int i=0;i<hped_vec[t][0].size();i++)
        {
// get pedestal and pedestal variances from pedestal histograms
            os   << t << " " << i << " " << hped_vec[t][fRunPar->fCalibrationSumWindow-1][i]->GetMean()/(double)fRunPar->fCalibrationSumWindow << " ";
// loop over all window sizes
            for(unsigned int j=0;j<hped_vec[t].size();j++)
            {
                os << hped_vec[t][j][i]->GetRMS() << " ";
            }
            os << endl;
        }
        os.close();
// write histograms to file
        if( !fPedOutFile[tel] || !fPedOutFile[tel]->cd() )
        {
            cout << "VCalibrator::writePeds(): error accessing pedestal output file " << t << endl;
            cout << "...exiting" << endl;
            exit( 0 );
        }
        if( usePedestalsInTimeSlices( iLowGain ) ) fillPedestalsInTimeSlices( tel, iPedestalCalculator );
// write 1D histograms
        TDirectory *i_dist = fPedOutFile[tel]->mkdir( "distributions" );
        if( i_dist->cd() )
        {
            i_dist->cd();
            for(unsigned int i=0;i<hped_vec[t].size();i++)
            {
                for(unsigned int j=0;j<hped_vec[t][i].size();j++)
                {
                    if( hped_vec[t][i][j] )
                    {
                        hped_vec[t][i][j]->Write();
                    }
                }
            }
        }
        fPedOutFile[tel]->Close();
    }                                             // end loop over all telescopes
}


bool VCalibrator::fillPedestalsInTimeSlices( unsigned int tel, VPedestalCalculator *iP )
{
    unsigned int t = 0;
    if( tel < getTeltoAna().size() ) t = getTeltoAna()[tel];
    else                             return false;

    if( t >= hped_vec.size() || tel >= getTeltoAna().size() )
    {
        cout << "VCalibrator::fillPedestalsInTimeSlices warning, telescope number out of range " << t << "\t" << hped_vec.size() << endl;
        return false;
    }
    if( !iP )
    {
       cout << "VCalibrator::fillPedestalsInTimeSlices error: no pedestal calculator given" << endl;
       return false;
    }

    char iname[800];
    char ititle[800];

    UInt_t ichannel = 0;
    UInt_t insumw = 0;
    const int nmax_sw = 500;                      //!< maximum number of summation windows: 5000
    Float_t isumw[nmax_sw];
    Float_t iped = 0.;
    Float_t ipedv[nmax_sw];
    for( int i = 0; i < nmax_sw; i++ )
    {
        isumw[i] = 0.;
        ipedv[i] = 0.;
    }
    UInt_t inevents = 0;
// pedestals in time slices
    const Int_t nSlices = 500;
    UInt_t TSnSlices = 0;
    if( iP && t < iP->v_MJD.size() ) TSnSlices = iP->v_MJD[t].size();

    cout << "\t number of time slices for pedestals in telescope " << t+1 << ": " << TSnSlices << endl;

    UInt_t TSMJD[nSlices];
    Double_t TStime[nSlices];
    UInt_t TSnevents[nSlices];
    Float_t TSpedmean[nSlices];
    for( int i = 0; i < nSlices; i++ )
    {
	TSMJD[i] = 0;
        TStime[i] = 0.;
        TSnevents[i] = 0;
        TSpedmean[i] = 0.;
    }
    vector< Float_t* > TSpedvar;

    sprintf( iname, "tPeds_%d", t+1 );
    sprintf( ititle, "pedestals, telescope %d", t+1 );
    TTree *iPedTree = new TTree( iname, ititle );
    iPedTree->Branch( "channel", &ichannel, "channel/i" );
    iPedTree->Branch( "nsumwindows", &insumw, "nsumwindow/i" );
    iPedTree->Branch( "sumwindow", isumw, "sumwindow[nsumwindow]/F" );
    iPedTree->Branch( "pedmean", &iped, "pedmean/F" );
    iPedTree->Branch( "pedvars", ipedv, "pedvars[nsumwindow]/F" );
    iPedTree->Branch( "nevents", &inevents, "nevents/i" );
    iPedTree->Branch( "TSnSlices", &TSnSlices, "TSnSlices/i" );
    iPedTree->Branch( "TSMJD", TSMJD, "TSMJD[TSnSlices]/i" );
    iPedTree->Branch( "TStime", TStime, "TStime[TSnSlices]/D" );
    iPedTree->Branch( "TSnevents", TSnevents, "TSnevents[TSnSlices]/i" );
    iPedTree->Branch( "TSpedmean", TSpedmean, "TSpedmean[TSnSlices]/F" );

    for( unsigned int i = 0; i < hped_vec[t].size(); i++ )
    {
        TSpedvar.push_back( new Float_t[nSlices] );
        sprintf( iname, "TSpedvar_sw%d[TSnSlices]/F", i+1 );
        sprintf( ititle, "TSpedvar_sw%d", i+1 );
        iPedTree->Branch( ititle, TSpedvar[i], iname );
    }

// loop over all channels
    for( unsigned int i=0; i < hped_vec[t][0].size(); i++ )
    {
        ichannel = (Int_t)i;
// get pedestal and pedestal variances from pedestal histograms
        if( fRunPar->fCalibrationSumWindow > 0 ) iped = hped_vec[t][fRunPar->fCalibrationSumWindow-1][i]->GetMean()/(double)fRunPar->fCalibrationSumWindow;
        else                               iped = 0.;
        inevents = (Int_t)hped_vec[t][fRunPar->fCalibrationSumWindow-1][i]->GetEntries();
// loop over all window sizes

        insumw = (UInt_t)hped_vec[t].size();
        for(unsigned int j=0;j<hped_vec[t].size();j++)
        {
            isumw[j] = (Float_t)j+1;
            ipedv[j] = hped_vec[t][j][i]->GetRMS();
        }
// pedestals in time slices
// loop over all time slices
        TSnSlices = (UInt_t)iP->v_MJD[tel].size();
        for( unsigned int ts = 0; ts < iP->v_MJD[tel].size(); ts++ )
        {
            TSMJD[ts] = iP->v_MJD[tel][ts];
            TStime[ts] = iP->v_time[tel][ts];
            if( i < iP->v_ped[tel][ts].size() && iP->v_ped[tel][ts][i].size() > 0 )
            {
                unsigned int iSW_mean = iP->v_ped[tel][ts][i].size()-1;
                if( iSW_mean > (unsigned int)getRunParameter()->fCalibrationSumWindow && getRunParameter()->fCalibrationSumWindow > 0 )
		{
		   iSW_mean = getRunParameter()->fCalibrationSumWindow - 1;
                }
                TSnevents[ts] = (UInt_t)iP->v_pedEntries[tel][ts][i][iSW_mean];
                TSpedmean[ts] = iP->v_ped[tel][ts][i][iSW_mean];
// check summation windows
                unsigned int iSW_temp = iP->v_pedvar[tel][ts][i].size();
                if( iSW_temp > TSpedvar.size() ) iSW_temp = TSpedvar.size();
                for( unsigned int w = 0; w < iSW_temp; w++ )
                {
                    TSpedvar[w][ts] = iP->v_pedvar[tel][ts][i][w];
                }
            }
        }

        iPedTree->Fill();
    }

    iPedTree->Write();

    return true;
}


void VCalibrator::calculateGainsAndTOffsets( bool iLowGain )
{
    if( getDebugFlag() )  cout << "VCalibrator::calculateGainsAndTOffsets()" << endl;
    if ( fReader->getMaxChannels() > 0 && hgain.size() == 0 )
    {
        findDeadChans( iLowGain );

// root output files are gain/toff file.root
        if( !iLowGain ) opfgain = new TFile( (fGainFileNameC[getTelID()]+".root").c_str(), "RECREATE");
	else            opfgain = new TFile( (fLowGainGainFileNameC[getTelID()]+".root").c_str(), "RECREATE");
        if( opfgain->IsZombie() )
        {
            cout << "calculateGainsAndTOffsets() error, can't open output file: " << opfgain->GetName() << endl;
            exit( -1 );
        }
        cout << "opened gain file: " << opfgain->GetName() << endl;
	cout << "calculate gains and toffsets with summation window " << fRunPar->fCalibrationSumWindow;
	cout << " (start at " << fRunPar->fCalibrationSumFirst << ")" << endl;

// create histograms
        for( unsigned int i = 0; i < getNChannels(); i++ )
        {
            char gainkey[100];
	    char ic[100];
            sprintf(gainkey,"hgain_%d",i);
            double imin=0.0;
            double imax=10.0;
            sprintf( ic, "gain distribution (tel %d, channel %d)", getTelID()+1, i );
            hgain.push_back( new TH1F(gainkey, ic,500,imin,imax) );

            char pulsekey[100];
            sprintf(pulsekey,"hpulse_%d",i);
            imin=0;
            imax=getNSamples();
            hpulse.push_back( new TProfile(pulsekey,"Mean pulse",(int)(imax-imin),imin,imax, -100., 10000.) );

            char spekey[100];
            sprintf( spekey, "htcpulse_%d", i );
            htcpulse.push_back( new TProfile( spekey, "time corrected mean pulse", 100, imin, imax ) );
        }
        if( !iLowGain ) opftoff = new TFile( (fToffFileNameC[getTelID()]+".root").c_str(), "RECREATE");
	else            opftoff = new TFile( (fLowGainToffFileNameC[getTelID()]+".root").c_str(), "RECREATE");
        if( opftoff->IsZombie() )
        {
            cout << "calculateGainsAndTOffsets() error, can't open output file: " << opftoff->GetName() << endl;
            exit( -1 );
        }
        for( unsigned int i = 0; i < getNChannels(); i++ )
        {
            char toff_vs_sumkey[100];
	    char ic[100];
            sprintf(toff_vs_sumkey,"htoff_vs_sum_%d",i);
            double imin=5;
            double imax=405;
            htoff_vs_sum.push_back( new TProfile(toff_vs_sumkey,"TOff vs Sum",20,imin,imax) );

            char toffkey[100];
            sprintf(toffkey,"htoff_%d",i);
            imin=-10;
            imax=10;
            sprintf( ic, "TOffset distribution (tel %d, channel %d)", getTelID()+1, i );
            htoff.push_back( new TH1F(toffkey,ic,500,imin,imax) );
        }
	setSpecialChannels();

        setCalibrated();
    }

    fillHiLo();

//! calculate sums and tzeros
    calcSums(   fRunPar->fCalibrationSumFirst, fRunPar->fCalibrationSumFirst+fRunPar->fCalibrationSumWindow, false );
    calcTZeros( fRunPar->fCalibrationSumFirst, fRunPar->fCalibrationSumFirst+fRunPar->fCalibrationSumWindow );

    if( fRunPar->fL2TimeCorrect ) FADCStopCorrect();

//! This will go when we get an event type
//////////////////////////////////////////
// test if this is a laser event (by sum of total charge in all channels )
    bool i_laser=false;
    double i_total=0;
    for(unsigned int i=0; i < getNChannels(); i++ ) i_total += getSums()[i];
    if (i_total > fRunPar->fLaserSumMin )
    {
        i_laser=true;
    }
    if(i_laser)
    {
        fNumberGainEvents[fTelID]++;
// (GM) write pulse to disk, pulse histograms for each event in one directory
        char i_name[200];
        char i_title[200];
        TDirectory *i_curDir = (TDirectory*)opfgain;
        TDirectory *i_pulseDir = 0;
        TH1D *i_pulse = 0;
        if( getRunParameter()->fwritepulses > 0 )
        {
            i_curDir->cd();
            sprintf( i_name, "zEvent_%d", getEventNumber() );
            i_pulseDir = gDirectory->mkdir( i_name, i_name);
        }

// need number of saturated PMTs for laser calibration
        int num_sat=0;
        for( unsigned int i = 0; i < fReader->getNumChannelsHit(); i++ )
        {
            fReader->selectHitChan(i);
            if( getHiLo()[i] != 0 && !getDead()[i] ) num_sat+=1;
        }
// don't do anything if there are not enough channels in low or high gain
        if( !iLowGain && num_sat > 10 ) return;
        if( iLowGain && num_sat < (int)(0.7*(double)getNChannels()) );

        int counttzero=0;
        int countsums=0;
        double m_tzero=0;
        double m_sums=0;
// calculate total sum
        for( unsigned int i = 0;i < getNChannels(); i++ )
        {
            if( iLowGain && !getHiLo()[i] ) continue;
            if( !iLowGain && getHiLo()[i] ) continue;

            if( (getSums()[i]>10 || fRunPar->fLaserSumMin < 0. )  && !getDead()[i] && !getMasked()[i] )
            {
                countsums++;
                m_sums  += getSums()[i];
                if(getTZeros()[i]>=0.)
                {
                    counttzero++;
                    m_tzero += getTZeros()[i];
                }
            }
        }
        if( countsums > 0 ) m_sums /= countsums;
        if( counttzero > 0 ) m_tzero /= counttzero;

// (GM) subtract pedestals
// (GM) set number of entries in hpulse to number of pulses added up
        double tcorr = 0.;
        for( unsigned int i = 0; i < getNChannels(); i++ )
        {
            int i_entries = (int)hpulse[i]->GetEntries();
            if( getRunParameter()->fwritepulses > 0 )
            {
                i_pulseDir->cd();
                sprintf( i_name, "h_%d", i );
                sprintf( i_title, "event %d, channel %d", getEventNumber(), i );
                i_pulse = new TH1D( i_name, i_title, hpulse[i]->GetNbinsX(), hpulse[i]->GetXaxis()->GetXmin(), hpulse[i]->GetXaxis()->GetXmax() );
            }

            if (m_sums>0.1 || fRunPar->fLaserSumMin < 0.)
            {
                double this_content = 0.;
                unsigned int chanID = 0;
                int this_bin = 0;
                for( unsigned int k = 0; k<fReader->getNumChannelsHit(); k++)
                {
                    try
                    {
                        chanID = fReader->getHitID(k);
                        if( chanID == i )
                        {
                            for( unsigned int j=0; j < (unsigned int)hpulse[i]->GetNbinsX(); j++ )
                            {
                                this_bin = (int)(j+fCalData[getTeltoAnaID()]->fFADCStopOffsets[i]+1);
                                if (this_bin > 0 && this_bin <= hpulse[i]->GetNbinsX())
                                {
				    this_content = fReader->getSample_double( chanID, this_bin, (this_bin==0) ) -  getPeds( iLowGain )[i];
                                    if( getRunParameter()->fwritepulses > 0 ) i_pulse->SetBinContent(this_bin,this_content);
                                    hpulse[i]->Fill( this_bin, this_content );
// time corrected pulse
                                    if( getTZeros()[i] > -99. ) tcorr = getTZeros()[i] - m_tzero;
                                    else                        tcorr = 0.;
                                    htcpulse[i]->Fill( (double)this_bin - tcorr, this_content );
                                }
                            }
                        }
                    }
                    catch(...)
                    {
                        cout << "VCalibrator::calculateGainsAndTOffsets() index out of range " << k << endl;
                    }
                }
                if( iLowGain && !getHiLo()[i] ) continue;
                if( !iLowGain && getHiLo()[i] ) continue;

                if( (getSums()[i]> 50 || fRunPar->fLaserSumMin < 0. ) && !getDead()[i] && !getMasked()[i] )
                {
                    hgain[i]->Fill((float)getSums()[i]/m_sums);
                    if(getTZeros()[i]>=0.)
                    {

                        htoff[i]->Fill((float)getTZeros()[i]-m_tzero);
                        htoff_vs_sum[i]->Fill((float)getSums()[i], (float)getTZeros()[i]-m_tzero);
                    }
                }
                if( getRunParameter()->fwritepulses > 0 )
                {
                    i_pulse->Write();
                }
            }
            if( i_entries != hpulse[i]->GetEntries() ) hpulse[i]->SetEntries( i_entries+1 );
            if( i_entries != htcpulse[i]->GetEntries() ) htcpulse[i]->SetEntries( i_entries+1 );
        }
        if( getRunParameter()->fwritepulses > 0 ) getRunParameter()->fwritepulses--;
        i_curDir->cd();
    }
}


void VCalibrator::writeGains( bool iLowGain )
{
    string iFile;
    for( unsigned int tel = 0; tel < getTeltoAna().size(); tel++ )
    {
        int t = getTeltoAna()[tel];
        setTelID( t );
        if( !iLowGain ) iFile = fGainFileNameC[t];
        else            iFile = fLowGainGainFileNameC[t];

        cout << "Telescope " << t+1 << endl;
        cout << "\t total number of laser events: " << fNumberGainEvents[t] << endl;
        if( !opfgain )
        {
            cout << "VCalibrator::writeGains error: no output file for gain values" << endl;
            continue;
        }
        cout << "\t writing gains to " << iFile << " and " << opfgain->GetName() << endl;
        ofstream os( iFile.c_str() );
        if( !os )
        {
            cout << "VCalibrator::writeGains(): ERROR unable to write to " << iFile << endl;
            exit( 0 );
        }
        else
        {
            for( unsigned int i = 0; i < getNChannels(); i++ )
            {
                os   << i << " " << hgain[i]->GetMean() << " " << hgain[i]->GetRMS() << endl;
            }
        }
        os.close();

        opfgain->cd();
        for( unsigned int i = 0; i < hgain.size(); i++ ) hgain[i]->Write();

        for( unsigned int i = 0; i < hpulse.size(); i++ )
        {
            hpulse[i]->Write();
            htcpulse[i]->Write();
        }
        TTree *iTG = fillGainTree( t );
        if( iTG ) iTG->Write();
        opfgain->Close();
    }
}


TTree *VCalibrator::fillGainTree( unsigned int itel )
{

    setTelID( itel );

    char iname[200];
    char ititle[200];

    if( fGainTree ) fGainTree->Delete();

    int ichannel;
    float igain;
    float igainvar;

    sprintf( iname, "tGains_%d", itel+1 );
    sprintf( ititle, "gains, telescope %d", itel+1 );
    fGainTree = new TTree( iname, ititle );
    fGainTree->Branch( "channel", &ichannel, "channel/I" );
    fGainTree->Branch( "gain", &igain, "gain/F" );
    fGainTree->Branch( "gainvar", &igainvar, "gainvar/F" );

    for( unsigned int i = 0; i < getNChannels(); i++ )
    {
        ichannel = (int)i;
        if( hgain[i] )
        {
            igain = hgain[i]->GetMean();
            igainvar = hgain[i]->GetRMS();
        }
        else
        {
            igain = 0.;
            igainvar = 0.;
        }
        fGainTree->Fill();
    }

    return fGainTree;
}


TTree *VCalibrator::fillToffTree( unsigned int itel )
{

    setTelID( itel );

    char iname[200];
    char ititle[200];

    if( fToffTree ) fToffTree->Delete();

    int ichannel;
    float itoff;
    float itoffvar;

    sprintf( iname, "tToffs_%d", itel+1 );
    sprintf( ititle, "time offsets, telescope %d", itel+1 );
    fToffTree = new TTree( iname, ititle );
    fToffTree->Branch( "channel", &ichannel, "channel/I" );
    fToffTree->Branch( "toff", &itoff, "toff/F" );
    fToffTree->Branch( "toffvar", &itoffvar, "toffvar/F" );

    for( unsigned int i = 0; i < getNChannels(); i++ )
    {
        ichannel = (int)i;
        if( htoff[i] )
        {
            itoff = htoff[i]->GetMean();
            itoffvar = htoff[i]->GetRMS();
        }
        else
        {
            itoff = 0.;
            itoffvar = 0.;
        }
        fToffTree->Fill();
    }

    return fToffTree;
}


void VCalibrator::writeTOffsets( bool iLowGain )
{
    string iFile;
    for( unsigned int tel = 0; tel < getTeltoAna().size(); tel++ )
    {
        int t = getTeltoAna()[tel];
        setTelID( t );
        if( !iLowGain ) iFile = fToffFileNameC[t];
        else            iFile = fLowGainToffFileNameC[t];
        if( !opftoff )
        {
            cout << "VCalibrator::writeTOffsets error writing time offsets to " << iFile << endl;
            continue;
        }
        cout << "\t writing time offsets to " << iFile << " and " << opftoff->GetName() << endl;

        ofstream os( iFile.c_str() );

        if( !os )
        {
            cout << "VCalibrator::writeTOffsets() ERROR: unable to write to " << iFile << endl;
            exit( 0 );
        }
        else for(unsigned int i=0;i<getNChannels();i++)
        {
            os << i << " " << htoff[i]->GetMean() << " " << htoff[i]->GetRMS() << endl;
        }
        os.close();

        opftoff->cd();
        for( unsigned int i = 0; i < htoff.size(); i++ ) htoff[i]->Write();
        for( unsigned int i = 0; i < htoff_vs_sum.size(); i++ )
        {
            htoff_vs_sum[i]->SetErrorOption("S");
            htoff_vs_sum[i]->Write();
        }
        TTree *iTT = fillToffTree( t );
        if( iTT ) iTT->Write();
        opftoff->Close();
    }
}


void VCalibrator::terminate( VPedestalCalculator* iP )
{
    if( fRunPar->frunmode == 1 || fRunPar->frunmode == 6 ) writePeds( fRunPar->frunmode == 6, iP );
    if( fRunPar->frunmode == 2 || fRunPar->frunmode == 5 )
    {
        writeGains( fRunPar->frunmode == 5 );
        writeTOffsets( fRunPar->frunmode == 5 );
    }
}


/*!

 */
void VCalibrator::readCalibrationData( bool iPeds, bool iGains )
{
    if( fDebug ) cout << "VCalibrator::readCalibrationData " << endl;

    if( !iPeds && !iGains ) return;

    for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
    {
        setTelID( getTeltoAna()[i] );

// read high gain gains
        if( getRunParameter()->frunmode != 2 && getRunParameter()->frunmode != 5 )
        {
            readGains( false );
// read low gain gains
            if( fLowGainGainFileNameC[getTeltoAna()[i]].size() > 0 )
            {
                readGains( true );
                setLowGainGains();
            }
// set low gain gains equal to high gain gains
            else getGains( true ) = getGains( false );
        }

//read high gain pedestals
        readPeds( fPedFileNameC[getTelID()], false, getSumWindow() );

// read low gain pedestals
        if( fLowGainPedFileNameC[getTeltoAna()[i]].size() > 0 )
        {
            readPeds( fLowGainPedFileNameC[getTeltoAna()[i]], true, getSumWindow() );
        }
// no low gain peds available -> set low gain peds to high gain peds (not sure if this is a good idea)
        else
        {
            getPedsLowGain() = getPeds();
            getPedvarsLowGain() = getPedvars();
            getPedvarsAllSumWindows( true ) = getPedvarsAllSumWindows( false );
            getmeanPedvarsAllSumWindow( true ) = getmeanPedvarsAllSumWindow( false );
            getmeanRMSPedvarsAllSumWindow( true ) = getmeanRMSPedvarsAllSumWindow( false );
        }

// read low gain multiplier
// (PRELI: for one summation window only)
        readLowGainMultiplier( getSumWindow_2() );

        if( getRunParameter()->frunmode != 2 && getRunParameter()->frunmode != 5 )
        {
// read high gain channel time offsets
            readTOffsets( false );
// read low gain channel time offsets
            if( fLowGainToffFileNameC.size() > 0 )
            {
                readTOffsets( true );
                setLowGainTOff();
            }
            else                                   getTOffsets( true ) = getTOffsets( false );
        }

// read pixel status
        readPixelstatus();

// read summation window dependend values for second summation window
	 readPeds( fPedFileNameC[getTeltoAna()[i]], false, getSumWindow_2() );

// read low gain pedestals
	 if( fLowGainPedFileNameC[getTeltoAna()[i]].size() > 0 )
	 {
	     readPeds( fLowGainPedFileNameC[getTeltoAna()[i]], true, getSumWindow_2() );
	 }
// (preli) fill high gain value if low value is not available
	 else
	 {
	    for( unsigned int g = 0; g < getPedsLowGain().size(); g++ )
	    {
		if( fabs( getPedsLowGain()[g] ) < 0.5 || fabs( getPedvarsLowGain()[g] ) < 0.5 )
		{
		    getPedsLowGain()[g] = getPeds()[g];
		}
	    }
	}
	if( fLowGainPedFileNameC[getTeltoAna()[i]].size() > 0 )
	{
	  getCalibrationData()->recoverLowGainPedestals();
        }

        setCalibrated();
    }                                             // loop over all telescopes

}

/*

 reading pedestal from root file

*/
bool VCalibrator::readPeds_from_rootfile( string iFile, bool iLowGain, unsigned int i_SumWindow )
{
  ifstream infile_root;
  infile_root.open( (iFile+".root").c_str(), ifstream::in);

// Temporary: do not read it for low gain channels
  if( infile_root && usePedestalsInTimeSlices( iLowGain ) )
  {
      infile_root.close();
      iFile += ".root";
      TFile iFPed( iFile.c_str() );
      char hname[20];
      sprintf( hname, "tPeds_%d", fTelID+1 );
      TTree *tPed = (TTree*)gDirectory->Get( hname );
      if( !tPed )
      {
	  cout << "VCalibrator::readPeds_from_rootfile WARNING:";
	  cout << "found root file with pedestals but no pedestal tree (telescope " << getTelID()+1 << "): " << iFile << endl;
	  return false;
      }
      cout << "Telescope " << getTelID()+1;
      if( !iLowGain ) cout << ": reading pedestals for high gain channels";
      else            cout << ": reading pedestals for low gain channels";
      cout << " and sumwindow " << i_SumWindow << " from :" << endl;
      cout << "Telescope " << getTelID()+1 << ": ";
      cout << iFile << endl;

      UInt_t ichannel = 0;
      UInt_t insumw = 0;
      Float_t isumw[VDST_MAXSUMWINDOW];
      for( unsigned int i = 0; i < VDST_MAXSUMWINDOW; i++ ) isumw[i] = 0.;
      Float_t iped = 0.;
      Float_t ipedv[VDST_MAXSUMWINDOW];
      for( unsigned int i = 0; i < VDST_MAXSUMWINDOW; i++ ) ipedv[i] = 0.;
      UInt_t iTSnSlices = 0;
      UInt_t iTSMJD[VDST_PEDTIMESLICES];
      for( unsigned int i = 0; i < VDST_PEDTIMESLICES; i++ ) iTSMJD[i] = 0;
      Double_t iTStime[VDST_PEDTIMESLICES];
      for( unsigned int i = 0; i < VDST_PEDTIMESLICES; i++ ) iTStime[i] = 0.;
      Float_t iTSpedmean[VDST_PEDTIMESLICES];
      for( unsigned int i = 0; i < VDST_PEDTIMESLICES; i++ ) iTSpedmean[i] = 0.;
      vector< Float_t* > iTSpedvars;
      for( unsigned int i = 0; i < getNSamples(); i++ ) iTSpedvars.push_back( new Float_t[VDST_PEDTIMESLICES] );

      tPed->SetBranchAddress( "channel", &ichannel );
      tPed->SetBranchAddress( "nsumwindows", &insumw );
      tPed->SetBranchAddress( "sumwindow", isumw );
      tPed->SetBranchAddress( "pedmean", &iped );
      tPed->SetBranchAddress( "pedvars", ipedv );
      tPed->SetBranchAddress( "TSnSlices", &iTSnSlices );
      tPed->SetBranchAddress( "TSMJD", iTSMJD );
      tPed->SetBranchAddress( "TStime", iTStime );
      tPed->SetBranchAddress( "TSpedmean", iTSpedmean );
      char i_branchname[100];
      for( unsigned int i = 0; i < getNSamples(); i++ )
      {
	sprintf( i_branchname, "TSpedvar_sw%d", i+1 );
	if( tPed->GetBranchStatus( i_branchname ) ) tPed->SetBranchAddress( i_branchname, iTSpedvars[i] );
      }
// initialize time dependant pedestal vectors
      if( tPed->GetEntries() > 0 )
      {
	valarray< double > i_ts_temp_peds;
	i_ts_temp_peds.resize( getPeds( false, -99. ).size(), 0. );
	valarray< valarray< double > > i_ts_temp_pedvar;
	i_ts_temp_pedvar.resize( getNSamples()+1, i_ts_temp_peds );

// for first event only, expect that all channels have same number of time slices
	tPed->GetEntry( 0 );

	if( iTSnSlices > 0 )
	{
// prepare time vectors
	  getCalData()->getMJDTS_vector().resize( iTSnSlices, 0 );
	  getCalData()->getTimeTS_vector().resize( iTSnSlices, 0. );

// prepare pedestal vector
	  getCalData()->getPedsTS_vector( iLowGain ).resize( iTSnSlices, i_ts_temp_peds );

// prepare pedestal variation vector
	 getCalData()->getPedvarsVTS_vector( iLowGain ).resize( iTSnSlices, i_ts_temp_pedvar );
       }
     }
/////////////////////////////////////////////////////////////////////////////////
// loop over tree entries and read pedestals and pedestal variations from tree
/////////////////////////////////////////////////////////////////////////////////

// loop over all channels
    for( int i = 0; i < tPed->GetEntries(); i++ )
    {
	tPed->GetEntry( i );

// fix summation window
	if( insumw > getLargestSumWindow() ) insumw = getLargestSumWindow();

	if ( ichannel >= getPeds( iLowGain, -99. ).size() ) continue;

// read pedestal and fill pedestal distributions
	getPeds( iLowGain, -99. )[ichannel] = iped;
	if( !iLowGain ) getPedDist()->Fill( iped );
	else            getPedLowGainDist()->Fill( iped );
// read pedestal variation as function of summation window
	for( unsigned int sw = 0; sw < insumw; sw++ )
	{
	    unsigned int icurrent_sw = (unsigned int)TMath::Nint( isumw[sw] );
	    if( ( isumw[sw] == i_SumWindow || (iLowGain && isumw[sw] == 23 ) ) && ichannel < getPedvars( iLowGain, i_SumWindow, -99. ).size() )
	    {
		getPedvars( iLowGain, i_SumWindow, -99. )[ichannel] = ipedv[sw];
		if( isumw[sw] == i_SumWindow )
		{
		    if( !iLowGain ) getPedvarsDist()->Fill( ipedv[sw] );
		    else            getPedvarsLowGainDist()->Fill( ipedv[sw] );
		}
	    }
	    if( icurrent_sw < getPedvarsAllSumWindows( iLowGain ).size() && ichannel < getPedvarsAllSumWindows( iLowGain )[sw].size() )
	    {
	        getPedvarsAllSumWindows( iLowGain )[icurrent_sw][ichannel] = ipedv[sw];
            }
	    else
	    {
		cout << "VCalibrator::readPeds error:";
		cout << " incompatible summation window from root file: ";
		cout << sw << "\t" << isumw[sw] << "\t" << getPedvarsAllSumWindows( iLowGain ).size() << endl;
	    }
	    if( sw == 1 && ichannel < getPedrms( iLowGain ).size() ) getPedrms( iLowGain )[ichannel] = ipedv[sw];
	}
	if( insumw > 0 && isumw[insumw-1] < i_SumWindow )
	{
	    cout << "VCalibrator::readPeds error:";
	    cout << "no pedestal found for this sumwindow : ";
	    cout << i_SumWindow << " (" << isumw[insumw-1] << "), tel, channel " << fTelID+1 << ", " << ichannel << endl;
	    if( iLowGain ) cout << "VCalibrator::readPeds: using pedestals for window " << isumw[insumw-1] << " for low gain channels" << endl;
	}
// read date for pedestals in time slices
	if( iTSnSlices > 0 )
	{
// fill vectors
	    for( unsigned int ts = 0; ts < iTSnSlices; ts++ )
	    {
// read time for first entry only
		if( i == 0 )
		{
		    getCalData()->getMJDTS_vector()[ts] = iTSMJD[ts];
		    getCalData()->getTimeTS_vector()[ts] = iTStime[ts];
		}
// pedestals and pedvars
		getCalData()->getPedsTS_vector( iLowGain )[ts][i] = iTSpedmean[ts];
		for( unsigned int sw = 0; sw < insumw; sw++ )
		{
		    if( sw < getCalData()->getPedvarsVTS_vector( iLowGain )[ts].size() ) getCalData()->getPedvarsVTS_vector( iLowGain )[ts][sw][i] = iTSpedvars[sw][ts];
		}
	    }                         // for( unsigned int ts = 0; ts < iTSnSlices; ts++ )
	}                             // if( iTSnSlices > 0 )
    }                                 // for( int i = 0; i < tPed->GetEntries(); i++ )
    tPed->ResetBranchAddresses();

/////////////////////////////////////////////////////////////////////////////////
// calculate mean and RMS pedvars for time slices
/////////////////////////////////////////////////////////////////////////////////
      valarray< double > its_mean( 0., getNSamples()+1 );
      valarray< double > its_rms( 0., getNSamples()+1 );
      getCalData()->getMeanPedvarsVTS_vector( iLowGain ).resize( getCalData()->getPedvarsVTS_vector( iLowGain ).size(), its_mean );
      getCalData()->getMeanRMSPedvarsVTS_vector( iLowGain ).resize( getCalData()->getPedvarsVTS_vector( iLowGain ).size(), its_rms );

      for( unsigned int ts = 0; ts < getCalData()->getPedvarsVTS_vector( iLowGain ).size(); ts++ )
      {
       its_mean.resize( getCalData()->getPedvarsVTS_vector( iLowGain )[ts].size(), 0. );
       its_rms.resize( getCalData()->getPedvarsVTS_vector( iLowGain )[ts].size(), 0. );

       for( unsigned int sw = 0; sw < getCalData()->getPedvarsVTS_vector( iLowGain )[ts].size(); sw++ )
       {
	    double its_n = 0.;
	    double its_sum = 0.;
	    double its_sum2 = 0.;
	    for( unsigned int ts_c = 0; ts_c < getCalData()->getPedvarsVTS_vector( iLowGain )[ts][sw].size(); ts_c++ )
	    {
	       if( getCalData()->getPedvarsVTS_vector( iLowGain )[ts][sw][ts_c] >= 1.0 )
	       {
		   its_n++;
		   its_sum  += getCalData()->getPedvarsVTS_vector( iLowGain )[ts][sw][ts_c];
		   its_sum2 += getCalData()->getPedvarsVTS_vector( iLowGain )[ts][sw][ts_c]*getCalData()->getPedvarsVTS_vector( iLowGain )[ts][sw][ts_c];
	       }
	    }
	    if( its_n > 1. )
	    {
	       its_mean[sw] = its_sum / its_n;
	       its_rms[sw]  = sqrt( 1./(its_n-1.) * ( its_sum2 - 1./its_n * its_sum * its_sum ) );
	    }
	    else
	    {
	       its_mean[sw] = 0.;
	       its_rms[sw]  = 0.;
	    }
	}
	getCalData()->getMeanPedvarsVTS_vector( iLowGain )[ts] = its_mean;
	getCalData()->getMeanRMSPedvarsVTS_vector( iLowGain )[ts] = its_rms;
// everything seemed to worked find
	return true;
      }
   }

   return false;
}


/*
   read pedestals from text file
*/
bool VCalibrator::readPeds_from_textfile( string iFile, bool iLowGain, unsigned int i_SumWindow )
{
   ifstream infile;
   infile.open( iFile.c_str(), ifstream::in);
   if( !infile )
   {
       cout << "VCalibrator::readPeds_from_textfile error: unable to open pedestal file " << iFile << endl;
       cout << "\t exiting..." << endl;
       exit( -1 );
   }

   cout << "Telescope " << getTelID()+1;
   if( !iLowGain ) cout << ": reading pedestals for high gain channels";
   else            cout << ": reading pedestals for low gain channels";
   cout << " and sumwindow " << i_SumWindow << " from: " << endl;
   cout << "Telescope " << getTelID()+1 << ": ";
   cout << iFile << endl;

// resetting all vectors
   getPeds( iLowGain, -99. ) = 1.0;
   getPedvars( iLowGain, i_SumWindow, -99. ) = 1.0;
   getPedrms( iLowGain ) = 1.0;
   unsigned int tel = 0;
   unsigned int ch = 0;
   float mean = 0.;
   float rms = 0.;
   int i_testCounter = 0;
// loop over pedestal file (text format)	    
   string i_Line;
   while( getline( infile, i_Line ) )
   {
       if( i_Line.size() > 0 )
       {
	   std::istringstream is_stream( i_Line );
// telescope number
	   is_stream >> tel;
	   if( tel == fTelID )
	   {
	       i_testCounter++;
// channel number
	       is_stream >> ch;
// pedestal
	       is_stream >> mean;
	       if ( ch < getPeds( iLowGain, -99. ).size() )
	       {
		   getPeds( iLowGain, -99. )[ch]=mean;
		   if( !iLowGain ) getPedDist()->Fill( mean );
		   else            getPedLowGainDist()->Fill( mean );
	       }
	       unsigned int count=0;
// pedestal variances
	       do
	       {
		   count+=1;
		   is_stream >> rms;
// ther might be more than NSamples values in the file; stop when reached NSamples
		   if( ( count==i_SumWindow || (iLowGain && count == getNSamples() ) )  && ch < getPedvars( iLowGain, i_SumWindow, -99. ).size() )
		   {
		       getPedvars( iLowGain, i_SumWindow, -99. )[ch] = rms;
		       if( count == i_SumWindow )
		       {
			   if( !iLowGain ) getPedvarsDist()->Fill( rms );
			   else getPedvarsLowGainDist()->Fill( rms );
			   
		       }
		   }
		   if( count <  getPedvarsAllSumWindows( iLowGain ).size() && ch < getPedvarsAllSumWindows( iLowGain )[count].size() )
		   {
		      getPedvarsAllSumWindows( iLowGain )[count][ch] = rms;
		   }
		   if( count == 1 && ch < getPedrms( iLowGain ).size() ) getPedrms( iLowGain )[ch] = rms;
	       } while( !is_stream.eof());
	       if( count < i_SumWindow )
	       {
		   cout << "VCalibrator::readPeds_from_textfile error:";
		   cout << "no pedestal found for this sumwindow : " << i_SumWindow << " (" << count << "), tel, channel " << tel+1 << ", " << ch << endl;
		   if( iLowGain ) cout << "VCalibrator::readPeds_from_textfile: using pedestals for window " << count << " for low gain channels" << endl;
	       }
	   }
       }
   }
   if( i_testCounter == 1 )
   {
       cout << "VCalibrator::readPeds: problem with reading pedestal file" << endl;
       cout << "\t probably old pedestal file format. Rerun pedestal calculation" << endl;
       cout << "\t exiting....." << endl;
       exit( 0 );
   }

   return true;
}

bool VCalibrator::readPeds_from_grisufile( string i_pedfile, bool iLowGain, unsigned int i_SumWindow )
{
   cout << "Telescope " << getTelID()+1 << ": reading peds from 'P' lines (sumwindow " << i_SumWindow;
   if( iLowGain ) cout << ", low gain)";
   else           cout << ", high gain)";
   cout << endl;

   setPedsFromPLine();
   fReader->setSumWindow( getTelID(), i_SumWindow );
   getPeds( iLowGain, -99. ) = fReader->getPeds();
   getPedvars( iLowGain, i_SumWindow, -99. ) = fReader->getPedvars();
   getPedvarsAllSumWindows( iLowGain ) = fReader->getPedvarsAllSumWindows();
   getPedrms( iLowGain ) = fReader->getPedRMS();

// fill pedestal distributions
  if( !iLowGain )
  {
      for( unsigned int i = 0; i < getPeds( iLowGain, -99. ).size(); i++ ) getPedDist()->Fill( getPeds( iLowGain, -99. )[i] );
      for( unsigned int i = 0; i < getPedvars( iLowGain, i_SumWindow, -99. ).size(); i++ )
      {
          getPedvarsDist()->Fill( getPedvars( iLowGain, i_SumWindow, -99. )[i] );
      }
  }
  else
  {
      for( unsigned int i = 0; i < getPeds( iLowGain, -99. ).size(); i++ ) getPedLowGainDist()->Fill( getPeds( iLowGain, -99. )[i] );
      for( unsigned int i = 0; i < getPedvars( iLowGain, i_SumWindow, -99. ).size(); i++ )
      {
         getPedvarsLowGainDist()->Fill( getPedvars( iLowGain, i_SumWindow, -99. )[i] );
      }
  }
  return true;
}

/*!

    read pedestals from root or text file or get if from the Monte Carlo file

*/
bool VCalibrator::readPeds( string i_pedfile, bool iLowGain, unsigned int i_SumWindow )
{
   if( getDebugFlag() ) cout << "VCalibrator::readPeds() " << getTelID()+1 << "\t" << i_pedfile << endl;

// set input file
   string iFile = i_pedfile;
//////////////////////////////////////////////
// reset histograms with pedestal distributions
   if( !iLowGain )
   {
      getPedvarsDist()->Reset();
      getPedDist()->Reset();
   }
   else
   {
      getPedvarsLowGainDist()->Reset();
      getPedLowGainDist()->Reset();
   }
   if( iFile.size() > 0 && getRunParameter()->fsimu_pedestalfile.size() == 0 )
   {
// read pedestals from root file
      if( !readPeds_from_rootfile( i_pedfile, iLowGain, i_SumWindow ) )
      {
          readPeds_from_textfile( i_pedfile, iLowGain, i_SumWindow );
      }
   }
////////////////////////////////////////////////////////////////
// getting pedestals directly from MC (grisu) file
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
   else if( fReader->getDataFormat() == "grisu" || (getRunParameter()->fsourcetype == 2 && getRunParameter()->fsimu_pedestalfile.size() > 0 ) )
   {
      readPeds_from_grisufile( i_pedfile, iLowGain, i_SumWindow );
   } 
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

// mean pedvar calculation
    if( getPedvarsAllSumWindows( iLowGain ).size() == getmeanPedvarsAllSumWindow( iLowGain ).size()  )
    {
        double i_n = 0.;
        double i_sum = 0.;
        double i_sum2 = 0.;
        for( unsigned int i = 0; i < getPedvarsAllSumWindows( iLowGain ).size(); i++ )
        {
            i_n = 0.;
            i_sum = 0.;
            i_sum2 = 0.;
            for( unsigned int s = 0; s < getPedvarsAllSumWindows( iLowGain )[i].size(); s++ )
            {
                if( getPedvarsAllSumWindows( iLowGain )[i][s] >= 1.0 )
                {
                    i_sum  += getPedvarsAllSumWindows( iLowGain )[i][s];
                    i_sum2 += getPedvarsAllSumWindows( iLowGain )[i][s] * getPedvarsAllSumWindows( iLowGain )[i][s];
                    i_n += 1.;
                }
            }
            if( i_n > 1. )
            {
                getmeanPedvarsAllSumWindow( iLowGain )[i] = i_sum / i_n;
                getmeanRMSPedvarsAllSumWindow( iLowGain )[i] = sqrt( 1/(i_n-1) * ( i_sum2 - 1./i_n * i_sum * i_sum ) );
            }
            else
            {
                getmeanPedvarsAllSumWindow( iLowGain )[i] = 0.;
                getmeanRMSPedvarsAllSumWindow( iLowGain )[i] = 0.;
            }
        }
    }

    if( iLowGain ) setLowGainPedestals();

    return true;
}


string VCalibrator::getCalibrationFileName( int iTel, int irun, string iSuffix )
{
   if( irun <= 0 ) return "";

   stringstream iFileStr;
   iFileStr << getRunParameter()->getDirectory_EVNDISPCalibrationData();
   iFileStr << "Tel_" << iTel+1 << "/";
   iFileStr << irun << ".";
   iFileStr << iSuffix;

   return iFileStr.str();
}

/*!

 */
void VCalibrator::readGains( bool iLowGain )
{
    if( fDebug ) 
    {
       cout << "VCalibrator::readGains() " << fReader->getDataFormat();
       cout << "\t low gain: " << iLowGain << endl;
       cout << endl;
    }

    string iFile = fGainFileNameC[getTelID()];
    if( iLowGain ) iFile = fLowGainGainFileNameC[getTelID()];

// don't read gains for runmode = 2
    if( iFile.size() > 0 && getRunParameter()->frunmode != 2 )
    {
        bool use_default=false;
        cout << "Telescope " << getTelID()+1 << ":";
        cout << " reading relative gains";
        if( iLowGain ) cout << " for low gain channels ";
        else           cout << " for high gain channels ";
        cout << "from: " << endl;
        cout << "Telescope " << getTelID()+1 << ": ";
	cout << iFile << endl;

        ifstream infile;
        infile.open( iFile.c_str(), ifstream::in );
        if (!infile)
        {
            cout << "VCalibrator::readGains() warning: Input file " << iFile << " cannot be opened.\n" ;
            cout << "VCalibrator::readGains() info: all gains set to 1.0\n" ;
            use_default=true;
	    if( getRunParameter()->fDBRunType == "flasher" || getRunParameter()->fDBRunType == "laser" ) setGains_DefaultValue( true, iLowGain );
	    else                                                                                         setGains_DefaultValue( false, iLowGain );
        }

        char buffer[100];
        setGains( 1.0, iLowGain );
        setGainvars( 1.0, iLowGain );
        if (!use_default)
        {
            while (!infile.eof() )
            {
                infile.getline(buffer,100);
                unsigned int ch;
                float mean,rms;
                sscanf(buffer,"%d %f %f",&ch,&mean,&rms);
                if ( ch < getGains().size() )
                {
                    setGains( ch, mean, iLowGain );
                    if( mean > 0. )
                    {
                        getGainDist( iLowGain )->Fill( mean );
                        getGainVarsDist( iLowGain )->Fill( rms );
                    }
                    setGainvars( ch, rms, iLowGain );
                }
                else
                {
                    cout << "VCalibrator::readGains(): channel out of range: " << ch << "\t" << getGains().size() << endl;
                }
            }
        }
    }
    else
    {
        setGains( 1., iLowGain );
        setGainvars( 1., iLowGain );
    }

// apply additional gain corrections
    getGains() /= getRunParameter()->fGainCorrection[getTelID()];

}


void VCalibrator::readTOffsets( bool iLowGain )
{
    if( fDebug ) cout << "VCalibrator::readTOffsets" << endl;

    string iFile = fToffFileNameC[getTelID()];
    if( iLowGain ) iFile = fLowGainToffFileNameC[getTelID()];

    if( iFile.size() > 0 && getRunParameter()->frunmode != 2 )
    {
        bool use_default=false;
        cout << "Telescope " << getTelID()+1 << ":";
        cout << " reading time offsets";
        if( iLowGain ) cout << " for low gain channels ";
        else           cout << " for high gain channels ";
        cout << "from: " << endl;
        cout << "Telescope " << getTelID()+1 << ": ";
	cout << iFile << endl;

        ifstream infile;
        infile.open( iFile.c_str(), ifstream::in );
        if( !infile )
        {
            cout << "VCalibrator::readTOffsets() warning: input file " << iFile << " cannot be opened.\n" ;
            cout << "VCalibrator::readTOffsets() info: all tOffsets set to 0.\n" ;
            use_default=true;
        }

        char buffer[100];
        setTOffsets( 0., iLowGain );
        setTOffsetvars( 5., iLowGain );
        if (!use_default)
        {
            while (!infile.eof() )
            {
                infile.getline (buffer,100);
                unsigned int ch;
                float mean,rms;
                sscanf(buffer,"%d %f %f",&ch,&mean,&rms);
                if (ch< getTOffsets().size())
                {
                    setTOffsets( ch, mean, iLowGain );
                    if( mean != 0. )
                    {
                        getToffsetDist( iLowGain )->Fill( mean );
                        getToffsetVarsDist( iLowGain )->Fill( rms );
                    }
                }
                if( ch < getTOffsetvars( iLowGain ).size()) setTOffsetvars( ch, rms, iLowGain );
            }
        }
    }
    else
    {
        setTOffsets( 0., iLowGain );
        setTOffsetvars( 0.1, iLowGain );
    }
}


void VCalibrator::initialize()
{
    if( fDebug ) cout << "VCalibrator::initialize()" << endl;

// set the data readers
    initializeDataReader();

    getCalibrationRunNumbers();

////////////////////////////////////////////////////////////////
// create the calibration data structures
    for( unsigned int i = 0; i < getNTel(); i++ )
    {
        setTelID( i );

	fCalData.push_back( new VCalibrationData( i, fRunPar->getDirectory_EVNDISPCalibrationData(), 
	                                          fPedFileNameC[i], fGainFileNameC[i], fToffFileNameC[i], fLowGainPedFileNameC[i] ));
	fCalData.back()->setSumWindows( getSumWindow( i ) );
// PRELI: low gain multiplier does not depend on summation window (yet)
	fCalData.back()->setLowGainMultiplierFixedSummationWindow( getSumWindow_2() );
        fNumberPedestalEvents.push_back( 0 );
        fNumberGainEvents.push_back( 0 );

        fCalData.back()->initialize( getNChannels(), getNSamples(), usePedestalsInTimeSlices( false ), usePedestalsInTimeSlices( true ), getDebugFlag() );
        if( fReader->getDataFormat() == "grisu" ) fCalData.back()->setReader( fReader );
    }

////////////////////////////////////////////////////////////////
// read the calibration files
    if( fRunPar->fsourcetype != 6 && fRunPar->fsourcetype != 7 && fRunPar->fsourcetype != 4 )
    {
        readCalibrationData( ( fRunPar->frunmode != 1 && fRunPar->frunmode != 6 ),
	                     ( fRunPar->frunmode != 2 && fRunPar->frunmode != 1 && fRunPar->frunmode != 6 ) );
    }
// PE mode: set gains to 1
    else
    {
        for( unsigned int tel = 0; tel < getTeltoAna().size(); tel++ )
        {
            int t = getTeltoAna()[tel];
            setTelID( t );
            readGains( false );
            readGains( true );
        }
// read peds from DST file (for DST MC source file)
	if( fRunPar->fsourcetype == 7 || fRunPar->fsourcetype == 4 ) readCalibrationData( fRunPar->fsourcefile );
    }

// initialize dead  channel finder
    initializeDeadChannelFinder();

}

void VCalibrator::setCalibrationFileNames()
{
   if( fDebug ) cout << "VCalibrator::setCalibrationFileNames()" << endl;

   for( unsigned int i = 0; i < getNTel(); i++ )
   {
      if( i < fPedFileNameC.size() )           fPedFileNameC[i] = getCalibrationFileName( i, getRunParameter()->fPedFileNumber[i], "ped" );
      else                                     fPedFileNameC.push_back( getCalibrationFileName( i, getRunParameter()->fPedFileNumber[i], "ped" ) );
      if( i < fGainFileNameC.size() )          fGainFileNameC[i] = getCalibrationFileName( i, getRunParameter()->fGainFileNumber[i], "gain" );
      else                                     fGainFileNameC.push_back( getCalibrationFileName( i, getRunParameter()->fGainFileNumber[i], "gain" ) );
      if( i < fToffFileNameC.size() )          fToffFileNameC[i] = getCalibrationFileName( i, getRunParameter()->fTOffFileNumber[i], "toff" );
      else                                     fToffFileNameC.push_back( getCalibrationFileName( i, getRunParameter()->fTOffFileNumber[i], "toff" ) );
      if( i < fPixFileNameC.size() )           fPixFileNameC[i] = getCalibrationFileName( i, getRunParameter()->fPixFileNumber[i], "pix" );
      else                                     fPixFileNameC.push_back( getCalibrationFileName( i, getRunParameter()->fPixFileNumber[i], "pix" ) );
      if( i >= fBlockTel.size()    )           fBlockTel.push_back( false );
      if( i < fLowGainPedFileNameC.size() )    fLowGainPedFileNameC[i] = getCalibrationFileName( i, getRunParameter()->fPedLowGainFileNumber[i], "lped" );
      else                                     fLowGainPedFileNameC.push_back( getCalibrationFileName( i, getRunParameter()->fPedLowGainFileNumber[i], "lped" ) );
      if( i < fLowGainMultiplierNameC.size() ) fLowGainMultiplierNameC[i] = getCalibrationFileName( i, getRunParameter()->fLowGainMultiplierFileNumber[i], "lmult" );
      else                                     fLowGainMultiplierNameC.push_back( getCalibrationFileName( i, getRunParameter()->fLowGainMultiplierFileNumber[i], "lmult" ) );
      if( i < fLowGainGainFileNameC.size() )   fLowGainGainFileNameC[i] = getCalibrationFileName( i, getRunParameter()->fGainLowGainFileNumber[i], "lgain" );
      else                                     fLowGainGainFileNameC.push_back( getCalibrationFileName( i, getRunParameter()->fGainLowGainFileNumber[i], "lgain" ) );
      if( i < fLowGainToffFileNameC.size() )   fLowGainToffFileNameC[i] = getCalibrationFileName( i, getRunParameter()->fTOffLowGainFileNumber[i], "ltoff" );
      else                                     fLowGainToffFileNameC.push_back( getCalibrationFileName( i, getRunParameter()->fTOffLowGainFileNumber[i], "ltoff" ) );
   }
}

/*

    getting calibration run numbers

    ped, gains, toffs (high and low gain)
*/
void VCalibrator::getCalibrationRunNumbers()
{
    if( fDebug ) cout << "VCalibrator::getCalibrationRunNumbers()" << endl;

// initialize vectors
    setCalibrationFileNames();

    if( getRunParameter()->fsimu_pedestalfile.size() > 0 )
    {
        cout << "VCalibrator::getCalibrationRunNumbers() info: taking calibration from grisu files" << endl;
        return;
    }

// get calibration run numbers from calibration text file
    int iCaliLines = 0;
    if( getRunParameter()->fcalibrationfile.size() > 0 )
    {
       iCaliLines = getCalibrationRunNumbers_fromCalibFile();
       if( iCaliLines > 0 ) setCalibrationFileNames();
    }
    int iLowGainCaliLines = 0;
    if( getRunParameter()->fLowGainCalibrationFile.size() > 0 && getRunParameter()->frunmode != 6 )
    {
       iLowGainCaliLines = readLowGainCalibrationValues_fromCalibFile( "LOWGAINPED" );
       if( iLowGainCaliLines > 0 ) setCalibrationFileNames();
    }

// take pedestals from grisu output file ('P'-lines), gains=1, and toff = 0.
    if( iCaliLines == 0 &&  (fReader->getDataFormat() == "grisu" || getRunParameter()->fsimu_pedestalfile.size() > 0 )  )
    {
        cout << "VCalibrator::getCalibrationRunNumbers() info: taking calibration from grisu files" << endl;
        return;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// no information found for this runnumber
    if( iCaliLines == 0 )
    {
// for dst, take dst.ped
        if( fReader->getDataFormat() == "DST" || fReader->getDataFormat() == "PE" )
        {
            for( unsigned int i = 0; i < getNTel(); i++ ) fPedFileNameC[i] = "dst";
        }
// plot raw: don't need any calibration data
        else if( getRunParameter()->fPlotRaw )
        {
            for( unsigned int i = 0; i < getNTel(); i++ ) fPedFileNameC[i] = "";
        }
// calibration run numbers are taken from DB
	else if( getRunParameter()->fsimu_pedestalfile.size() == 0 &&
	         getRunParameter()->fuseDB && getRunParameter()->frunmode != 2 &&
		 getRunParameter()->frunmode != 5 &&
		 getRunParameter()->frunmode != 1 &&
		 getRunParameter()->frunmode != 6 )
	{
            cout << "VCalibrator::getCalibrationRunNumbers() info: using laser file number from DB" << endl;
	}
// for laser files: take data file run number for pedestals and gains
        else if( getRunParameter()->fDBRunType == "flasher" )
	{
	    cout << "VCalibrator::getCalibrationRunNumbers() info: laser/flasher files, using run number for pedestals and gains" << endl;
	}
// this is an calibration run
	else if( getRunParameter()->fsimu_pedestalfile.size() == 0 && getRunParameter()->fcalibrationrun )
	{
	   cout << "VCalibrator::getCalibrationRunNumbers() info: calibration run - no calibration run numbers necessary" << endl;
        }
// no calibration data: problem
        else
        {
            cout << "VCalibrator::getCalibrationRunNumbers() error: no calibration information found for run ";
            cout << getRunNumber();
	    if( getRunParameter()->fcalibrationfile.size() > 0 ) cout << " in file " << getRunParameter()->fcalibrationfile <<  endl;
	    else                                                 cout << ": no file given " << endl;
	    cout << "(run type " << getRunParameter()->fDBRunType << ")" << endl;
            exit( -1 );
        }
    }

// append suffixes
    for( unsigned int i = 0; i < getNTel(); i++ )
    {
        if( fPixFileNameC[i].size() > 0 )  fPixFileNameC[i] += ".pix";
    }
    cout << endl;
    
    return;
}

int VCalibrator::readLowGainCalibrationValues_fromCalibFile( string iVariable, unsigned int iTelescopeSelect, int iSumWindow )
{
    int iLinesFound = 0;
// open file with calibration information
    string is_Temp;
    is_Temp = fRunPar->getDirectory_EVNDISPCalibrationData();
    is_Temp += getRunParameter()->fLowGainCalibrationFile;
    ifstream is;
    is.open( is_Temp.c_str(), ifstream::in);
    if( !is )
    {
        cout << "VCalibrator::readLowGainCalibrationValues_fromCalibFile: error, calibration data file not found: ";
        cout <<  is_Temp << endl;
        exit( -1 );
    }

    string is_line;
    int iRunMin = 0;
    int iRunMax = 0;
    int iTel = -1;

    string iLowGainPeds;
    for( unsigned int i = 0; i < fBlockTel.size(); i++ ) fBlockTel[i] = false;

    while( getline( is, is_line ) )
    {
        if(  is_line.size() > 0 )
        {
            istringstream is_stream( is_line );
// only line with '*' as first characters are valid lines
            is_stream >> is_Temp;
            if( is_Temp != "*" ) continue;

	    if( is_stream.eof() ) continue;
	    is_stream >> is_Temp;
	    if( is_Temp == iVariable )
	    {
	       if( is_stream.eof() ) continue;
	       is_stream >> iTel;
	       iTel--;              // internal counting starts at 0
	       if( is_stream.eof() ) continue;
	       is_stream >> iRunMin;
	       if( is_stream.eof() ) continue;
	       is_stream >> iRunMax;
// check run number range
               if( getRunNumber() >= iRunMin && getRunNumber() <= iRunMax )
	       {
	          if( iVariable == "LOWGAINPED" )
		  {
		     cout << "reading low-gain parameters for run range " << iRunMin << ", " << iRunMax << endl;
                  }
	          if( is_stream.eof() ) continue;
		  is_stream >> iLowGainPeds;
		  iLinesFound++;
		  if( iTel < 0 )
		  {
		     for( unsigned int i = 0; i < getNTel(); i++ )
		     {
		        if( !fBlockTel[i] )
			{
			   if( iVariable == "LOWGAINPED" && atoi( iLowGainPeds.c_str() ) > 0 )
			   {
			      getRunParameter()->fPedLowGainFileNumber[i] = atoi( iLowGainPeds.c_str() );
                           }
			   else if( iVariable == "LOWGAINMULTIPLIER" && atof( iLowGainPeds.c_str() ) > 0. )
			   {
			      cout << "VCalibrator::readLowGainCalibrationValues_fromCalibFile() warning: ";
			      cout << " low-gain multiplier need to be set for each telescope " << endl;
			      cout << "(ignored this entry)" << endl;
                           }
                        }
                     }
                  }
		  else if( iTel < (int)getNTel() )
		  {
		    if( iVariable == "LOWGAINPED" && atoi( iLowGainPeds.c_str() ) > 0 )
		    {
		       getRunParameter()->fPedLowGainFileNumber[iTel] = atoi( iLowGainPeds.c_str() );
		       fBlockTel[iTel] = true;
                    }
		    else if( iVariable == "LOWGAINMULTIPLIER" && atof( iLowGainPeds.c_str() ) > 0. && iTel == (int)iTelescopeSelect )
		    {
		       double iG = atof( iLowGainPeds.c_str() );
		       setMeanLowGainMultiplier( iG, iSumWindow );
		       if( is_stream.eof() ) setRMSLowGainMultiplier( 0, iSumWindow );
		       else
		       {
			  is_stream >> is_Temp;
			  setRMSLowGainMultiplier( atof( is_Temp.c_str() ), iSumWindow );
                       }
		       for( unsigned int j = 0; j < getLowGainMultiplier( iSumWindow ).size(); j++ )
		       {
// NOTE: set low gain multiplier for all summation windows
			  setLowGainMultiplier( iG, j, 9999 );
			  getLowGainMultiplierError( iSumWindow )[j] = 0.;
			  getCalData()->getLowGainMultiplierDistribution()->Fill( getLowGainMultiplier()[j] );
                       }
		       fBlockTel[iTel] = true;
                    }
                  }
              }
           }
        }
   }

   return iLinesFound;
}



int VCalibrator::getCalibrationRunNumbers_fromCalibFile()
{
// open file with calibration information
    string is_Temp;
    is_Temp = fRunPar->getDirectory_EVNDISPCalibrationData();
    is_Temp += getRunParameter()->fcalibrationfile;
    ifstream is;
    is.open( is_Temp.c_str(), ifstream::in);
    if( !is )
    {
        cout << "VCalibrator::getCalibrationRunNumbers error, calibration data file not found: ";
        cout <<  is_Temp << endl;
        exit( -1 );
    }

    string is_line;
    string iPed, iPad, iGain, iToff, iPix, iLowGainPeds, iLowGainMultiplier, iLowGainGains, iLowGainToff;
    int iTel = 0;
    int iCaliLines = 0;
    bool bReset = false;
    while( getline( is, is_line ) )
    {
        if(  is_line.size() > 0 )
        {
            istringstream is_stream( is_line );
// only line with '*' as first characters are valid lines
            is_stream >> is_Temp;
            if( is_Temp != "*" && is_Temp != "*V1" && is_Temp != "*V2" && is_Temp != "*V3" && is_Temp != "*V4" ) continue;

// get version number
            if( is_Temp == "*V2" )      fCalibrationfileVersion = 2;
            else if( is_Temp == "*V3" ) fCalibrationfileVersion = 3;
            else if( is_Temp == "*V4" ) fCalibrationfileVersion = 4;

// get and check run number
            is_stream >> is_Temp;
            if( atoi( is_Temp.c_str() ) != getRunNumber() ) continue;
            iCaliLines++;

            cout << "reading calibration information (version " << fCalibrationfileVersion << ")" << endl;
            cout << is_line << endl;

// get telescope numbers
            is_stream >> is_Temp;
            iTel = atoi( is_Temp.c_str() ) - 1;

// get ped file number
            is_stream >> iPed;

// get gain file number
            is_stream >> iGain;

// get toff file number
            is_stream >> iToff;

// get pixel status number
            is_stream >> iPix;

// get pad file number
            is_stream >> iPad;

// get low gain pedestal file number
            if( fCalibrationfileVersion > 1 )
            {
                is_stream >> iLowGainPeds;
                if( bReset )
                {
                    getRunParameter()->fPedLowGainFileNumber.clear();
                    for( unsigned int i = 0; i < getNTel(); i++ ) getRunParameter()->fPedLowGainFileNumber.push_back( 0 );
                    bReset = true;
                }
            }
            else iLowGainPeds = "-2";

// low gain gains and time offsets
            if( fCalibrationfileVersion > 3 )
            {
                is_stream >> iLowGainGains;
                is_stream >> iLowGainToff;
            }

// get low gain multiplier file name
            if( fCalibrationfileVersion > 2 )
            {
                is_stream >> iLowGainMultiplier;
            }

// iTel < 0: valid for all telescopes
            if( iTel < 0 )
            {
                for( unsigned int i = 0; i < getNTel(); i++ )
                {
                    if( !fBlockTel[i] )
                    {
                        if( iPed != "-1" )
                        {
                            getRunParameter()->fPedFileNumber[i] = atoi( iPed.c_str() );
                        }
                        if( iGain != "-1" )
                        {
                            getRunParameter()->fGainFileNumber[i] = atoi( iGain.c_str() );
                        }
                        if( iToff != "-1" )
                        {
                            getRunParameter()->fTOffFileNumber[i] = atoi( iToff.c_str() );
                        }
                        if( iPix != "-1" )
                        {
                            getRunParameter()->fPixFileNumber[i] = atoi( iPix.c_str() );
                        }
                        if( fCalibrationfileVersion > 1 && iLowGainPeds != "-1" )
                        {
                            getRunParameter()->fPedLowGainFileNumber[i] = atoi( iLowGainPeds.c_str() );
                        }
                        else if( iLowGainPeds == "-2"
			        && i < getRunParameter()->fPedLowGainFileNumber.size() && getRunParameter()->fPedLowGainFileNumber[i] > 0 )
                        {
                            ostringstream s_temp;
                            s_temp << getRunParameter()->fPedLowGainFileNumber[i];
                            fLowGainPedFileNameC[i] = s_temp.str();
                        }
			else if( iLowGainPeds == "-1" )
			{
			   getRunParameter()->fPedLowGainFileNumber[i] = atoi( iLowGainPeds.c_str() );
                        }
                        if( fCalibrationfileVersion > 2 && iLowGainMultiplier != "-1" )
                        {
                            fLowGainMultiplierNameC[i] = iLowGainMultiplier;
                        }
                        if( fCalibrationfileVersion > 3 && iLowGainGains != "-1" )
                        {
                            fLowGainGainFileNameC[i] = iLowGainGains;
                        }
                        if( fCalibrationfileVersion > 3 && iLowGainToff != "-1" )
                        {
                            getRunParameter()->fTOffLowGainFileNumber[i] = atoi( iLowGainToff.c_str() );
                        }
                    }
                }
            }
            else if( iTel < (int)getNTel() )
            {
                if( iPed != "-1" )
                {
                    getRunParameter()->fPedFileNumber[iTel] = atoi( iPed.c_str() );
                }
                if( iGain != "-1" )
                {
                    getRunParameter()->fGainFileNumber[iTel] = atoi( iGain.c_str() );
                }
                if( iToff != "-1" )
                {
                    getRunParameter()->fTOffFileNumber[iTel] = atoi( iToff.c_str() );
                }
                if( iPix != "-1" )
                {
                    getRunParameter()->fPixFileNumber[iTel] = atoi( iPix.c_str() );
                }
                else               fPixFileNameC[iTel] = "";
                if(  fCalibrationfileVersion > 1 && iLowGainPeds != "-1" )
                {
                    getRunParameter()->fPedLowGainFileNumber[iTel] = atoi( iLowGainPeds.c_str() );
                }
                if( fCalibrationfileVersion > 2 && iLowGainMultiplier != "-1" )
                {
                    fLowGainMultiplierNameC[iTel] = iLowGainMultiplier;
                }
                if( fCalibrationfileVersion > 3 && iLowGainGains != "-1" )
                {
                    getRunParameter()->fGainLowGainFileNumber[iTel] = atoi( iLowGainGains.c_str() );
                }
                if( fCalibrationfileVersion > 3 && iLowGainToff != "-1" )
                {
                    getRunParameter()->fTOffLowGainFileNumber[iTel] = atoi( iLowGainToff.c_str() );
                }
                fBlockTel[iTel] = true;
            }
        }
    }
    is.close();

    return iCaliLines;
}

void VCalibrator::readPixelstatus()
{
    if( fDebug ) cout << "VCalibrator::readPixelstatus()" << endl;

    if( fPixFileNameC[getTelID()].size() > 0 )
    {
        cout << "Telescope " << getTelID()+1 << ": ";
        cout << "reading pixel status from ";
        cout << "Telescope " << getTelID()+1 << ": ";
	cout << fPixFileNameC[getTelID()] << endl;
// open pedestal file
        ifstream is;
        is.open( fPixFileNameC[getTelID()].c_str(), ifstream::in);
        if( !is )
        {
            cout << "VCalibrator::readPixelstatus error: unable to open pixel file " << fPixFileNameC[getTelID()] << endl;
            cout << "\t exiting...." << endl;
            exit( -1 );
        }
        string is_line;
        string iTemp;
        string iTemp2;
        while( getline( is, is_line ) )
        {
            if(  is_line.size() > 0 )
            {
                istringstream is_stream( is_line );
                is_stream >> iTemp;
                is_stream >> iTemp2;
                if( atoi( iTemp.c_str() ) < (int)getChannelStatus().size() )
                {
                    getChannelStatus()[atoi( iTemp.c_str() )] = atoi( iTemp2.c_str() );
                }
                else
                {
                    cout << "VCalibrator::readPixelstatus() warning: index out of range " << iTemp << "\t";
                    cout << getChannelStatus().size() << endl;
                }
            }
        }
        is.close();
    }
}

/*!

     NOTE: READING LOW GAIN MULTIPLIERS FROM TREES NEED SOME WORK

     SHOULD BE AS WELL ONE LOW GAIN MULTIPLIER PER SUMMATION WINDOW

     TODO: check that this is read out correctly for all summation windows

*/
bool VCalibrator::readLowGainMultiplier( int iSumWindow )
{
// read low gain multipliers from a root file 
// (read for one summation only)
    if( fLowGainMultiplierNameC[getTelID()].size() > 0 )
    {
        cout << "Telescope " << getTelID()+1 << ": ";
        cout << "reading low gain multiplier for summation window " << iSumWindow;
        cout << " from ";
        cout << "Telescope " << getTelID()+1 << ": ";
	cout << fLowGainMultiplierNameC[getTelID()] << endl;

        TFile i_f( fLowGainMultiplierNameC[getTelID()].c_str() );
        if( i_f.IsZombie() )
        {
            cout << "VCalibrator::readLowGainMultiplier error: unable to open file " << i_f.GetName() << endl;
            cout << "\t exiting... " << endl;
            exit( -1 );
        }
	char ic[1000];
        sprintf( ic, "lowgainMultiplier_%d_%d", getTelID()+1, iSumWindow );
        TTree *t = (TTree*)gDirectory->Get( ic );
        if( !t )
        {
            cout << "VCalibrator::readLowGainMultiplier warning: unable to find low gain multiplier tree ";
            cout << "for telescope " << getTelID()+1 << " and summation window " << iSumWindow << endl;
            cout << "\t " << ic << endl;
	    cout << "\t exiting..." << endl;
	    exit( -1 );
        }
        unsigned int chanID = 0;
        double gainMult = 0.;
        double gainMultE = 0.;
        t->SetBranchAddress( "chanID", &chanID );
        t->SetBranchAddress( "gainMult", &gainMult );
        t->SetBranchAddress( "gainMultE", &gainMultE );

// calculate mean low gain multiplier; this value is used if there is no calibration value
        double iMeanLG = 0.;
        double iNMeanLG = 0.;
        double iMeanLG2 = 0.;
        for( int i = 0; i < t->GetEntries(); i++ )
        {
            t->GetEntry( i );

// skip channels with no error in gain multiplier
            if( gainMultE < 1.e-7 ) continue;

            if( chanID < getNChannels() )
            {
                iMeanLG += gainMult;
                iMeanLG2 += gainMult * gainMult;
                iNMeanLG++;
            }
        }
        if( iNMeanLG > 0. ) iMeanLG /= iNMeanLG;
        else                iMeanLG = getDetectorGeometry()->getLowGainMultiplier()[getTelID()];
// set low gain multiplier (PRELI: for all summation windows)
        setMeanLowGainMultiplier( iMeanLG );
        if( iNMeanLG > 1. ) setRMSLowGainMultiplier( sqrt( (iMeanLG2 - iNMeanLG * iMeanLG)/(iNMeanLG-1.)/iNMeanLG ) );

        for( int i = 0; i < t->GetEntries(); i++ )
        {
            t->GetEntry( i );

            if( chanID < getNChannels() )
            {
                if( gainMultE > 1.e-7 )
                {
                    getLowGainMultiplier( iSumWindow )[chanID] = gainMult;
                    getLowGainMultiplierError( iSumWindow )[chanID] = gainMultE;
                    getCalData()->getLowGainMultiplierDistribution()->Fill( gainMult );
                }
// set gain multipliers in channels without measurement to mean value over all channels
                else
                {
                    getLowGainMultiplier( iSumWindow )[chanID] = iMeanLG;
                    getLowGainMultiplierError( iSumWindow )[chanID] = 0.;
                    getCalData()->getLowGainMultiplierDistribution()->Fill( iMeanLG );
                }
            }
        }
        t->ResetBranchAddresses();
        i_f.Close();
    }
//////////////////////////////////////////////////////////////////
// read low gain multiplier from low gain calibration file
// (one value per telescope)
    else if( getRunParameter()->fLowGainCalibrationFile.size() > 0 && getRunParameter()->frunmode != 6 )
    {
	cout << "Telescope " << getTelID()+1 << ": ";
        cout << "reading low gain multiplier from " << getRunParameter()->fLowGainCalibrationFile << endl;
	readLowGainCalibrationValues_fromCalibFile( "LOWGAINMULTIPLIER", getTelID(), iSumWindow );
	getDetectorGeometry()->setLowGainMultiplier( getTelID(), getCalData()->getMeanLowGainMultiplier() );
    }
//////////////////////////////////////////////////////////////////
// fill low gain multipliers set in .cfg file
// (one value per telescope)
    else if( getDetectorGeometry()->isLowGainSet() )
    {
        for( unsigned int i = 0; i < getLowGainMultiplier( iSumWindow ).size(); i++ )
        {
            getLowGainMultiplier( iSumWindow )[i] = getDetectorGeometry()->getLowGainMultiplier()[getTelID()];
            getLowGainMultiplierError( iSumWindow )[i] = 0.;
            getCalData()->getLowGainMultiplierDistribution()->Fill( getLowGainMultiplier()[i] );
        }
    }

    return true;
}

/*

*/
bool VCalibrator::readCalibrationData( string iDSTfile )
{
   TFile iF( iDSTfile.c_str() );
   if( iF.IsZombie() )
   {
      cout << "VCalibrator::readCalibrationData error while opening DST tree in " << iDSTfile << endl;
      exit( -1 );
   }
   TTree *t = (TTree*)iF.Get( "calibration" );
   if( !t )
   {
      cout << "VCalibrator::readCalibrationData warning: failed reading calibration tree from file " << iDSTfile << endl;
      return false;
   }
   cout << "reading calibration data from dst file" << endl;

   int fTelID = 0;
   unsigned int nPixel = 0;
   float fPed_high[VDST_MAXCHANNELS];
   float fPedvar_high[VDST_MAXCHANNELS];
   float fPed_low[VDST_MAXCHANNELS];
   float fPedvar_low[VDST_MAXCHANNELS];
   float fConv_high[VDST_MAXCHANNELS];
   float fConv_low[VDST_MAXCHANNELS];

   t->SetBranchAddress( "TelID", &fTelID );
   t->SetBranchAddress( "NPixel", &nPixel );
   t->SetBranchAddress( "ped_high", fPed_high );
   t->SetBranchAddress( "pedvar_high", fPedvar_high );
   t->SetBranchAddress( "ped_low", fPed_low );
   t->SetBranchAddress( "pedvar_low", fPedvar_low );
   t->SetBranchAddress( "conv_high", fConv_high );
   t->SetBranchAddress( "conv_low", fConv_low );

// reset histograms with pedestal distributions
   if( getPedvarsDist() )        getPedvarsDist()->Reset();
   if( getPedDist() )            getPedDist()->Reset();
   if( getPedvarsLowGainDist() ) getPedvarsLowGainDist()->Reset();
   if( getPedLowGainDist() )     getPedLowGainDist()->Reset();

   if( getNTel() != (unsigned int)t->GetEntries() )
   {
      cout << "VCalibrator::readCalibrationData error: mismatch in number of telescopes: " ;
      cout << getNTel() << "\t" << t->GetEntries() << endl;
      exit( -1 );
   }
   for( int i = 0; i < t->GetEntries(); i++ )
   {
       t->GetEntry( i );

       setTelID( i );

// no calibration data available for this telescope
       if( nPixel == 0 ) continue;

// peds
       if( nPixel == getPeds( false ).size() )
       {
           for( unsigned int p = 0; p < nPixel; p++ )
	   {
	       getPeds( false )[p] = fPed_high[p];
	       if( getPedDist() ) getPedDist()->Fill( fPed_high[p] );
           }
       }
       else
       {
          cout << "bool VCalibrator::readCalibrationData( string iDSTfile )";
	  cout << " error: index out of range (peds, high gain): ";
	  cout << nPixel << "\t" << getPeds( false ).size();
	  cout << " (telescope " << getTelID()+1 << ")" << endl;
       }
       if( nPixel == getPeds( true ).size() )
       {
           for( unsigned int p = 0; p < nPixel; p++ )
	   {
	      getPeds( true )[p] = fPed_low[p];
	      if( getPedLowGainDist() ) getPedLowGainDist()->Fill( fPed_low[p] );
           }
       }
       else
       {
          cout << "bool VCalibrator::readCalibrationData( string iDSTfile )";
	  cout << "error: index out of range (peds, low gain): ";
	  cout << nPixel << "\t" << getPeds( true ).size();
	  cout << " (telescope " << getTelID()+1 << ")" << endl;
       }
// pedvars
       if( nPixel == getPedvars( false ).size() )
       {
           for( unsigned int p = 0; p < nPixel; p++ )
	   {
	       getPedvars( false )[p] = fPedvar_high[p];
	       if( getPedvarsDist() ) getPedvarsDist()->Fill( fPed_high[p] );
           }
       }
       else
       {
          cout << "bool VCalibrator::readCalibrationData( string iDSTfile ) error: ";
	  cout << "index out of range (pedvars, high gain): ";
	  cout << nPixel << "\t" << getPedvars( false ).size();
	  cout << " (telescope " << getTelID()+1 << ")" << endl;
       }
       if( nPixel == getPedvars( true ).size() )
       {
           for( unsigned int p = 0; p < nPixel; p++ ) 
	   {
	      getPedvars( true )[p] = fPedvar_low[p];
	      if( getPedvarsLowGainDist() ) getPedvarsLowGainDist()->Fill( fPed_low[p] );
           }
       }
       else
       {
          cout << "bool VCalibrator::readCalibrationData( string iDSTfile ) error: ";
	  cout << "index out of range (pedvars, low gain): ";
	  cout << nPixel << "\t" << getPedvars( true ).size();
	  cout << " (telescope " << getTelID()+1 << ")" << endl;
       }
// gains
       if( nPixel == getGains( false ).size() )
       {
          for( unsigned int p = 0; p < nPixel; p++ )
	  {
	     if( fConv_high[p] > 0. && !getRunParameter()->fIgnoreDSTGains )
	     {
		getGains( false )[p] = 1./fConv_high[p];
		if( getGainDist( false ) ) getGainDist( false )->Fill( 1./fConv_high[p] );
             }
	     else
	     {
	        getGains( false )[p] = 1.;
             }
          }
       }
       else
       {
          cout << "bool VCalibrator::readCalibrationData( string iDSTfile ) error: ";
	  cout << "index out of range (gains, high gain): ";
	  cout << nPixel << "\t" << getGains( false ).size();
	  cout << " (telescope " << getTelID()+1 << ")" << endl;
       }
       if( nPixel == getGains( true ).size() )
       {
          for( unsigned int p = 0; p < nPixel; p++ )
	  {
	     if( fConv_low[p] > 0. && !getRunParameter()->fIgnoreDSTGains )
	     {
		getGains( true )[p] = 1./fConv_low[p];
		if( getGainDist( true ) ) getGainDist( true )->Fill( 1./fConv_low[p] );
             }
	     else
	     {
	        getGains( true )[p] = 1.;
             }
          }
       }
       else
       {
          cout << "bool VCalibrator::readCalibrationData( string iDSTfile ) error: ";
	  cout << "index out of range (gains, low gain): ";
	  cout << nPixel << "\t" << getGains( true ).size();
	  cout << " (telescope " << getTelID()+1 << ")" << endl;
       }
   }

   iF.Close();
   return true;
}
