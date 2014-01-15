#include "VLowGainCalibrator.h"

VLowGainCalibrator::VLowGainCalibrator(int run, int sw, bool isInnerHigh, TString dir) {

	fDEBUG=true;
	fRun=run;
	fWindow=sw;
	if( isInnerHigh ) {
		iChanMon_start=250;
		iChanMon_stop=499;
		iChan_start=0;
		iChan_stop=250;
	}
	else {
		iChanMon_start=0;
		iChanMon_stop=250;
		iChan_start=250;
		iChan_stop=499;
	}


	for(int tel=0; tel<fNTel; tel++) {
		TString name=TString::Format("hist_medians_Tel%d_run_%d", tel+1, run );
		fMonitorChargeHist[tel] = new TH1D(name.Data(), "monitor charge;qmon (median) ;entries", 1000,0,1000);
	}

	TString name=TString::Format("%s/%d.DST.root", dir.Data(), run);
	fInfile = new TFile( name.Data(), "read");
	fDsttree = (TTree*)fInfile->Get("dst");
	if(fDsttree) {
	
		fDsttree->SetBranchAddress("sumfirst",&sumfirst);
		fDsttree->SetBranchAddress("sum",&sum);
		fDsttree->SetBranchAddress("HiLo",&HiLo);
		fDsttree->SetBranchAddress("sumwindow",&sumwindow);
		fDsttree->SetBranchAddress("eventNumber",&eventNumber);
		fDsttree->SetBranchAddress("dead",&dead);
	}

	for(int tel=0; tel<fNTel; tel++) {
		name.Form( "%s/Tel_%d/%d.lped.root", dir.Data(), tel+1, run);
		fOutfile[tel] = new TFile( name.Data(), "recreate");
		name.Form( "slopes_%d_%d", tel+1, sw);
		fOuttree[tel] = new TTree( name.Data(), name.Data() );

		fOuttree[tel]->Branch("channel",	&fTree_Channel,		"channel/I");
		fOuttree[tel]->Branch("m",		&fTree_m, 		"m[2]/D");
		fOuttree[tel]->Branch("mErr", 		&fTree_mErr, 		"mErr[2]/D");
		fOuttree[tel]->Branch("chi2", 		&fTree_chi2, 		"chi2[2]/D");
		fOuttree[tel]->Branch("ndf", 		&fTree_ndf, 		"ndf[2]/I");
		fOuttree[tel]->Branch("status", 	&fTree_status, 		"status[2]/I");

	}
}


VLowGainCalibrator::~VLowGainCalibrator() { 
	for(int tel=0; tel<fNTel; tel++) {
		fMonitorChargeHist[tel]->Delete();
	}
}



void VLowGainCalibrator::setMonitorChargeOptions( int nLive_min, double sum_min, bool useMedian) {
	fNLiveMonitor_min = nLive_min;
	fUseMedian = useMedian;
	fSumMonitor_min = sum_min;

} 

void VLowGainCalibrator::setFitOptions( int n_min, double pure_min, double prob_min, double b_max ) {
	fFitPure_min = pure_min;
	fFitNPoints_min = n_min;
	fFitProb_min = prob_min ;
	fFitB_max = b_max;


		

}

/*
     fit function (to be wrapped in a TF1 objct )
     assume each light level for the monitoring charge is gaussian distributed 
     and above a linear background (pol1 = par[1] + par[2])
     par[0] is number of peaks.
*/
Double_t fpeaks( double *x, double *par ) 
{
    double result = par[1] + par[2]*x[0];
    for( int p=0; p<par[0]; p++ ) 
    {
	double norm  = par[3*p+3];
	double mean  = par[3*p+4];
	double sigma = par[3*p+5];
	result += norm*TMath::Gaus(x[0],mean,sigma);
    }
    return result;
}

double VLowGainCalibrator::calcMeanMonitorCharge(int tel, int ientry) {
	if(ientry>-1) fDsttree->GetEntry(ientry);
	int nLive=0;
	double mean = 0;
	for(int iChan=iChanMon_start; iChan<iChanMon_stop; iChan++) {
		if( dead[tel][iChan] || HiLo[tel][iChan] ) continue;
		if( isNewPixel( tel, iChan) ) continue;
		if( sum[tel][iChan] < fSumMonitor_min ) continue;		
		nLive++;
		mean+=sum[tel][iChan];
	}

	if ( nLive<100 ) return -9999;
	else return mean/nLive;

}

double VLowGainCalibrator::calcMedianMonitorCharge(int tel, int ientry) {
	if(ientry>-1) fDsttree->GetEntry(ientry);
	int nLive=0;
	vector<double> Qmon;
	for(int iChan=iChanMon_start; iChan<iChanMon_stop; iChan++) {
		if( dead[tel][iChan] || HiLo[tel][iChan] ) continue;
		if( isNewPixel( tel, iChan) ) continue;
		if( sum[tel][iChan] < fSumMonitor_min ) continue;		
		nLive++;
		Qmon.push_back(sum[tel][iChan] );
	}

	if ( nLive<100 ) return -9999;

	std::sort( Qmon.begin(), Qmon.end() );
	double median;
	if( nLive  % 2 ) median = ( Qmon.at( nLive/2-1 ) + Qmon.at(nLive/2) )/2.0;
	else median=Qmon.at( nLive/2) ;
	return median;

}
double VLowGainCalibrator::calcMonitorCharge(int tel, int ientry) {

	if( fUseMedian ) return calcMedianMonitorCharge( tel, ientry );
	else return calcMeanMonitorCharge( tel, ientry );

}
  

bool VLowGainCalibrator::makeMonitorChargeHists( ) {

	for(int tel=0; tel<fNTel; tel++) {
		fMonitorChargeHist[tel]->Reset();
	}

	for(int iEntry=0; iEntry<fDsttree->GetEntries() ; iEntry++) {
		fDsttree->GetEntry( iEntry );

		for(int tel=0; tel<4; tel++) {
	
			double median = calcMonitorCharge(tel );
 
			fMonitorChargeHist[tel]->Fill(median); 

		}

	}
	return true;

}


bool VLowGainCalibrator::findLightLevels() {
	for(int tel=0; tel<fNTel; tel++) {

		fillLightLevels( tel, 2);
//		if(checkLightLevels(tel ) == 2) { 
//			fillLightLevels(tel, 3);
//		}
	}
	return true;
}


/*

     fill the different light levels from the monitoring charge histograms
     
*/
void VLowGainCalibrator::fillLightLevels( int tel, int iPeakSignificance , bool iDraw )
{
    TH1D* ihist = fMonitorChargeHist[tel];
cout << ihist->GetNbinsX() << " " << ihist->GetEntries() << endl;
	TCanvas *c1 ;
    if(fDEBUG) iDraw=true;
    if( iDraw ) {
	c1 = new TCanvas("c1","c1",10,10,500,400);
	c1->Divide(1,2); c1->cd(1);
	ihist->Draw();
    }
// find peaks (there should be 7 if the flasher is ok)
    int iLightLevel = 7;

cout << "hi" << endl;
    TSpectrum *s = new TSpectrum( 2*iLightLevel );
cout << "hi" << endl;
    if( iDraw ) { 
	s->Search( ihist, iPeakSignificance, "", 0.05 ); 
	c1->cd(2);
    } else
        s->Search( ihist, iPeakSignificance, "goff", 0.05 ); 

    int iNfound = s->GetNPeaks();
    if( fDEBUG ) s->Print();
    else cout << "Found " << iNfound << " possible light levels in tel " << tel+1 << endl;


    bool rebin = false;
    int cnt = 0;
    TH1D *ihist2, *h2 ;
    while( iNfound > iLightLevel )
    {
	ihist2 = (TH1D*)ihist->Clone("ihist2");
	ihist2->Rebin( cnt+1 );
	
	if( iDraw ) 
	{
	    ihist2->Draw();
	    s->Search( ihist2, 2, "", 0.05 );
	} else 
	    s->Search( ihist2, 2, "goff", 0.05 );
	
	iNfound = s->GetNPeaks();
	
	cout << "Rebin histogram (" << cnt+1 << ")" << endl;
	if ( fDEBUG ) s->Print();

	rebin = true;

	// make sure that at some point the while loop is exited 
	if( cnt == 3 )
	{
	    cout << endl;
	    cout << "WARNING: Even after three rebinning attempts more then " << fNLightLevels[tel] << " light levels are found for tel " << tel+1  << "." << endl;
	    cout << "         Please manually inspect the histogram for goodness of this calibration run." << endl << endl;
	}
	else if( cnt >= 10 || cnt >= (ihist->GetEntries()/ihist->GetNbinsX()) ) 
	{
	    cout << "ERROR: Too many peaks found even after " << cnt << " rebinning attempts for tel " << tel+1  << "." << endl;
	    cout << "       Set number of peaks to zero! "<< endl;
	    cout << "       exiting... " << endl;
	    iNfound = 0;
	    exit(-1);
	}
	cnt++;
    }
    if( rebin ) h2 = (TH1D*)ihist2->Clone("h2");
    else 	h2 = (TH1D*)ihist->Clone("h2"); 
    if( !h2 ) 
    {
	cout << "ERROR: Could not find histogram to be used for fitting the light level for tel " << tel+1  << "." << endl;
	exit(-1);
    }

    
// estimate background using a linear fitting method
    TF1 *iline = new TF1("iline","pol1",0,1000);
    ihist->Fit("iline","qn");

    double par[300];
    par[0] = iline->GetParameter(0);
    par[1] = iline->GetParameter(1);

    if( par[0] > 10 )
    {
	cout << endl;
	cout << "WARNING: The estimated linear background seems high for tel " << tel+1  << "." << endl;
	cout << "         Please manually inspect the histogram for goodness of this calibration run" << endl << endl;
    }

// eliminate peaks with light level zero (sometimes found in the first bin) 
// and too small peaks at the background level
    int npeaks = 0;
    for( int i=0; i<iNfound; i++ )
    {
	double ix = s->GetPositionX()[i];
 	int ibin  = h2->GetXaxis()->FindBin( ix );
 	double iy = h2->GetBinContent( ibin );

	if( ibin <= 1 ) continue; 
	if( iy - TMath::Sqrt(iy) < iline->Eval(ix) ) continue;

	// fill paramter estimates for the gaussian fits
 	par[3*npeaks+3] = iy;
 	par[3*npeaks+4] = ix;
 	par[3*npeaks+5] = 3; 
 	npeaks++;
    }
    par[0]=npeaks;
    cout << "Finally used " << npeaks << " peaks for fitting the light level parameters" << endl; 

    if( npeaks > iLightLevel )
    {
	cout << "ERROR: Found more than 7 light levels (this should not happen) " << endl
	     << "       Please check if distinct light levels exist" << endl;
	exit( -1 );
    }
    else 
    {
	cout << "Now fitting: Be patient" << endl;

	TF1 *fit = new TF1( "fit", fpeaks, 0, 1000, 3+3*npeaks );
	TVirtualFitter::Fitter( h2, 3+3*npeaks );
	fit->SetParameters(par);
        fit->SetParLimits(0, npeaks, npeaks);
	fit->SetNpx(1000);

	if( iDraw ) h2->Fit("fit");
	else        h2->Fit("fit","N");

	TString iStatus( gMinuit->fCstatu );
	if( !iStatus.CompareTo("CONVERGED") )
	{
	    cout << "ERROR: Fit of the peaks did not converge! " << endl;
	    fNLightLevels[tel]=0;
	} 
	else 
	{
	    fNLightLevels[tel]=npeaks;
	    if( fDEBUG ) cout << "#light levels in tel " << tel+1 << " : " << npeaks << endl;

	    for( unsigned int i=0; i<fNLightLevels[tel]; i++ )
	    {
		fLightLevelMean[tel].push_back( fit->GetParameter(3*i+4) );
		fLightLevelMeanError[tel].push_back( fit->GetParError(3*i+4) );
		if( fDEBUG ) cout << "mean:   " << fLightLevelMean[tel][i] << " +/- " << fLightLevelMeanError[tel][i] << endl;

		fLightLevelWidth[tel].push_back( fit->GetParameter(3*i+5) );
		fLightLevelWidthError[tel].push_back( fit->GetParError(3*i+5) );
		if( fDEBUG ) cout << "sigma:  " << fLightLevelWidth[tel][i] << " +/- " << fLightLevelWidthError[tel][i] << endl;
	    }
	}
    }
    delete s;
}
	

int VLowGainCalibrator::checkLightLevels( int tel, int iPeakSignificance, bool iDraw )
{

    if( fNLightLevels[tel] == 0 )
    {
	cout << "error: No light levels found." << endl;
	return -1;
    } 

//  check if light levels are distinct
//  hardcoded condition: they should not overlap at the 2 sigma level 
    bool twopeaks = false;
    for( unsigned int i=0; i<fNLightLevels[tel]-1; i++ )
    {
	for( unsigned int k=(i+1); k<fNLightLevels[tel]; k++ )
	{
	    if( fLightLevelMean[tel][i] < fLightLevelMean[tel][k] )
	    {
		if( fLightLevelMean[tel][i] + 2*fLightLevelWidth[tel][i] < fLightLevelMean[tel][k] - 2*fLightLevelWidth[tel][k] ) continue;
		else 
		{
		    cout << "WARNING: Found two light levels close to each other. " << endl;
		    if(fDEBUG) cout << fLightLevelMean[tel][i] << " + 2x" << fLightLevelWidth[tel][i] << " and " << fLightLevelMean[tel][k] << " - 2x" << fLightLevelWidth[tel][k] << endl;
		    twopeaks=true;
		}
	    }
	    else if( fLightLevelMean[tel][i] > fLightLevelMean[tel][k] )
	    {
		if( fLightLevelMean[tel][i] - 2*fLightLevelWidth[tel][i] > fLightLevelMean[tel][k] + 2*fLightLevelWidth[tel][k] ) continue;
		else
		{
		    cout << "WARNING: Found two light levels close to each other. " << endl;
		    if(fDEBUG) cout << fLightLevelMean[tel][i] << " + 2x" << fLightLevelWidth[tel][i] << " and " << fLightLevelMean[tel][k] << " - 2x" << fLightLevelWidth[tel][k] << endl;
		    twopeaks=true;
		}
	    }
	    // don't compare the same entries/values (should not happen, but who knows...) 
	    else if( TMath::Abs( fLightLevelMean[tel][i] - fLightLevelMean[tel][k] ) < 1e-3 ) continue;
	    else 
		cout << "Warning: Wow, you should have never reached this point... " << endl;
	}
    }
    if( twopeaks ) return 2;
    else           return 1; 

}




bool VLowGainCalibrator::calculateMeanCharges() {
	//initialise vectors
	for(int tel=0; tel<fNTel; tel++) {
		for(int iChan=iChan_start; iChan<iChan_stop; iChan++) {
			for(int hilo=0; hilo<2; hilo++) {
				fN[tel][iChan][hilo].assign( fNLightLevels[tel], 0);
				fY[tel][iChan][hilo].assign( fNLightLevels[tel], 0);
				fY2[tel][iChan][hilo].assign( fNLightLevels[tel], 0);
				
			}//hilo
		}//chan
	}//tel

	for(int ientry=0; ientry<fDsttree->GetEntries(); ientry++) {

		fDsttree->GetEntry(ientry);

		for(int tel=0; tel<fNTel; tel++) {
			double qmon = calcMonitorCharge(tel);
			int level=-1;
			for(int ilevel=0; ilevel<fNLightLevels[tel]; ilevel++) {
				if( qmon < fLightLevelMean[tel][ilevel] + 2*fLightLevelWidth[tel][ilevel] &&  qmon > fLightLevelMean[tel][ilevel] - 2*fLightLevelWidth[tel][ilevel] ) { 
					level=ilevel;
					break;
				}
			} //levels
			if( level<0) continue;
			
			//first calculate mean/sdev of start time for hi/lo channels.
			
			double start[2];
			start[0]=0;
			start[1]=0;
			
			double start2[2];
			start2[0]=0;
			start2[1]=0;
						
			double N[2];
			N[0]=0;
			N[1]=0;

			for(int iChan=iChan_start; iChan<iChan_stop; iChan++) {

				if( dead[tel][iChan] ) continue;

				N	[ HiLo[tel][iChan] ] ++;

				start	[ HiLo[tel][iChan] ] += sumfirst[tel][iChan];
				start2	[ HiLo[tel][iChan] ] += sumfirst[tel][iChan]*sumfirst[tel][iChan];
			
			}//chan

			for(int hilo=0; hilo<2; hilo++) {
				if( N[hilo] ==0 ) continue ;
			 	start[hilo]/=N[hilo];
				start2[hilo]/=N[hilo];
				start2[hilo]=sqrt( start2[hilo] - start[hilo]*start[hilo] );
			}//hilo

			for(int iChan=iChan_start; iChan<iChan_stop; iChan++) {

				if( dead[tel][iChan] ) continue;
				if( fabs( sumfirst[tel][iChan] - start[ HiLo[tel][iChan] ] ) > 2*start2[ HiLo[tel][iChan] ] ) continue; 				
//		cout << tel << " " << iChan << " " << HiLo[tel][iChan] << " " << level << " " << sum[tel][iChan] << endl;

				fN [tel][iChan][ HiLo[tel][iChan] ][level]++;
				fY [tel][iChan][ HiLo[tel][iChan] ][level]+=sum[tel][iChan];
				fY2[tel][iChan][ HiLo[tel][iChan] ][level]+=sum[tel][iChan]*sum[tel][iChan];

			}//chan

		}//tel

	}//ientry

	//now fix normalization etc.

	for(int tel=0; tel<fNTel; tel++) {
		for(int iChan=iChan_start; iChan<iChan_stop; iChan++) {
			for(int hilo=0; hilo<2; hilo++) {
				for(int ilevel=0; ilevel<fNLightLevels[tel]; ilevel++) {
					if( fN[tel][iChan][hilo][ilevel]==0 ) continue;
					fY[tel][iChan][hilo][ilevel] /=  fN[tel][iChan][hilo][ilevel];
					fY2[tel][iChan][hilo][ilevel] /= fN[tel][iChan][hilo][ilevel];
				
				}//level
			}//hilo
		}//chan
	}//tel
	return true;
}

bool VLowGainCalibrator::doTheFit() {

	for(int tel=0; tel<fNTel; tel++) {
		for(int iChan=iChan_start; iChan<iChan_stop; iChan++) {

		fTree_Channel=iChan;	

			for(int hilo=0; hilo<2; hilo++) {
				TGraphErrors t(0);
				TF1 * f = new TF1 ("f", "[0]*x+[1]", 0, 1000);
				int N=0;
				for(int i=0; i<fNLightLevels[tel]; i++) {
					if(fN[tel][iChan][0][i]+ fN[tel][iChan][1][i] == 0 || fN[tel][iChan][hilo][i] / ( fN[tel][iChan][0][i]+ fN[tel][iChan][1][i] ) < fFitPure_min ) continue; 
					t.SetPoint(N, fLightLevelMean[tel][i], fY[tel][iChan][hilo][i] );
					double eX, eY;
					eX= fLightLevelWidth[tel][i];
					eY=sqrt( fY2[tel][iChan][hilo][i] - fY[tel][iChan][hilo][i] * fY[tel][iChan][hilo][i] );
					t.SetPointError(N, eX, eY );
					N++;
				}// light levels
				
				if( N < fFitNPoints_min ) {
					fTree_status[hilo] = NO_POINTS;

					fTree_m[hilo] = -1;
					fTree_mErr[hilo] =99999;
					fTree_chi2[hilo]= 99999;
					fTree_ndf[hilo]	= -1;
					
				}
				else {
					t.Fit(f);
					
					fTree_m[hilo] = f->GetParameter(0);
					fTree_mErr[hilo] = f->GetParError(0);
					double b = f->GetParameter(1);
					double berr = f->GetParError(1);

					fTree_chi2[hilo]= f->GetChisquare();
					fTree_ndf[hilo]	= f->GetNDF();
					double p = TMath::Prob(fTree_chi2[hilo], fTree_ndf[hilo]);

					if( p< fFitProb_min ) fTree_status[hilo] = BAD_CHI2;
					else if( TMath::Abs( b ) / berr  > fFitB_max ) fTree_status[hilo] = NOT_PROPORTIONAL;
					else fTree_status[hilo] = GOOD;

				}
				delete f;
			
			}//hilo
	
			fOuttree[tel]->Fill();
		}//chan
	}//tel

	return true;
}

bool VLowGainCalibrator::terminate( ) {
	for(int tel=0; tel<fNTel; tel++) {
		fOutfile[tel]->cd();
		fOuttree[tel]->Write();
		fOutfile[tel]->Close();
	}
	return true;
}


bool VLowGainCalibrator::isNewPixel( int tel, int iChan) { 
	//catches runs that had test pixels (with the new PMTs) in T3. The gain/gain vs HV was different and we don't want to use them to determine the monitor charge.
	if(tel!=2) return false;
	if( fRun< 54239 || fRun > 63195) return false;
	if( iChan==280 || iChan==281 || iChan==340 || iChan==341 || iChan==342 || iChan==406 || iChan==408 ) return true;
	if( iChan==291 || iChan==292 || iChan==253 || iChan==254 || iChan==419 || iChan==420 )	return true;
	return false;
}


bool VLowGainCalibrator::fIsOk() {
	for(int tel=0; tel<fNTel; tel++) {
		if( !fDsttree ) return false;	
		if( !fOutfile[tel] ) return false; 
	}
	return true;
}



