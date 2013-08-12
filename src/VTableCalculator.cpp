/*! \file VTableCalculator.cpp
    \brief  general lookup table class (e.g. mscw, mscl, energy)


*/

#include "VTableCalculator.h"

VTableCalculator::VTableCalculator( int intel, bool iEnergy, bool iPE )
{
    setDebug();

    setConstants( iPE );

    if( intel == 0 ) return;

    fEnergy = iEnergy;
    fUseMedianEnergy = 0;

    for( int i = 0; i < intel; i++ )
    {
        hVMedian.push_back( 0 );
    }
    hMedian = 0;
    hMean = 0;

    Omode = 'r';
    fwrite = false;
    if ((Omode=='w')||(Omode=='W')) fwrite = true;

    fInterPolWidth = 1;
    fInterPolIter = 3;

    fBinning1DXlow = 1.e-5;
    fBinning1DXhigh = 1.+1.e-5;
    if( fEnergy )
    {
        fBinning1DXlow =  1.;
        fBinning1DXhigh = 5.;
    }

    fReadHistogramsFromFile = false;

}


VTableCalculator::VTableCalculator( string fpara, string hname_add, char m, TDirectory *iDir, bool iEnergy, bool iPE, int iUseMedianEnergy )
{
    setDebug();

    setConstants( iPE );
// using lookup tables to calculate energies
    fEnergy = iEnergy;
    fUseMedianEnergy = iUseMedianEnergy;
    fReadHistogramsFromFile = false;

    fWrite1DHistograms = true;
    fHName_Add = hname_add;

    fName = fpara;

    fInterPolWidth = 1;
    fInterPolIter = 3;

    setBinning();

    fOutDir = iDir;
    if( !fOutDir )
    {
        cout << "VTableCalculator: error data directory in root file does not exist " << fOutDir << "\t" << fpara << endl;
        exit( -1 );
    }
    if( !fOutDir->cd() )
    {
        cout << "VTableCalculator: error accessing data directory in root file " << fOutDir << "\t" << fpara << endl;
        exit( -1 );
    }
    Omode  = m;
    fwrite = false;
    if ((Omode=='w')||(Omode=='W')) fwrite = true;

    int i = 0;
    int j = 0;
    char hname[1000];

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// table writing
    if( fwrite )
    {
        if( !fOutDir->IsWritable() )
        {
            cout << "VTableCalculator: error data directory in root file not writable" << endl;
            exit( -1 );
        }

/* HSTOGRAM BOOKING */
        char htitle[1000];
// median of variable
        sprintf( hname, "%s_median_%s", fpara.c_str(), fHName_Add.c_str() );
        sprintf( htitle, "%s vs. dist. vs. log10 size (median)", fpara.c_str() );
        hMedian = new TH2F( hname, htitle, NumSize, amp_offset, amp_offset+NumSize*amp_delta, NumDist, 0., dist_delta*NumDist );
        hMedian->SetXTitle( "log_{10} size" );
        hMedian->SetYTitle( "distance [m]" );
        if( !fEnergy ) sprintf( htitle, "%s (median) [deg]", fpara.c_str() );
        else           sprintf( htitle, "%s (median) [TeV]", fpara.c_str() );
        hMedian->SetZTitle( htitle );
// mean and rms
        sprintf( hname, "%s_mean_%s", fpara.c_str(), fHName_Add.c_str() );
        sprintf( htitle, "%s vs. dist. vs. log10 size (mean)", fpara.c_str() );
        hMean = new TProfile2D( hname, htitle, NumSize, amp_offset, amp_offset+NumSize*amp_delta, NumDist, 0., dist_delta*NumDist, fBinning1DXlow, fBinning1DXhigh );
        hMean->SetXTitle( "log_{10} size" );
        hMean->SetYTitle( "distance [m]" );
        if( !fEnergy ) sprintf( htitle, "%s (mean) [deg]", fpara.c_str() );
        else           sprintf( htitle, "%s (mean) [TeV]", fpara.c_str() );
        hMean->SetZTitle( htitle );
// 1d histograms for variable distribution
        for (i=0;i<NumSize;i++)
        {
            vector< TH1F* > iH1;
            for (j=0;j<NumDist;j++) iH1.push_back( 0 );	        
            Oh.push_back( iH1 );
        }
    }
/////////////////////////////////////////
// table reading
/////////////////////////////////////////
    else
    {
        fReadHistogramsFromFile = false;

	if( fUseMedianEnergy == 1)
	{
	   sprintf( hname, "%s_median_%s", fpara.c_str(), fHName_Add.c_str() );
        }
	else if( fUseMedianEnergy == 2 )
	{
	   if( fEnergy ) sprintf( hname, "%s_mpv_%s", fpara.c_str(), fHName_Add.c_str() );
	   else          sprintf( hname, "%s_median_%s", fpara.c_str(), fHName_Add.c_str() );
	}
	else
	{
	   sprintf( hname, "%s_mean_%s", fpara.c_str(), fHName_Add.c_str() );
        }
	hMedianName = hname;
    }

}

void VTableCalculator::setBinning()
{
    fBinning1DXlow = 0.;
    fBinning1DXhigh = 1.;
    if( fEnergy )
    {
        fBinning1DXlow =  0.01;
        fBinning1DXhigh = 250.;
        HistBins = int( fBinning1DXhigh / 0.005 );
    }
}
 
bool VTableCalculator::create1DHistogram( int i, int j )
{
   if( i >= 0 && j >= 0 && i < (int)Oh.size() && j < (int)Oh[i].size() && !Oh[i][j] )
   {
        if( !fOutDir->cd() ) return false;
        char hisname[200];
        char histitle[200];
        int id = i*1000+j;

        sprintf( hisname , "h%d",id);
        double is1 = hMedian->GetXaxis()->GetBinLowEdge( i + 1 );
        double is2 = hMedian->GetXaxis()->GetBinLowEdge( i + 1 ) + hMedian->GetXaxis()->GetBinWidth( i+1 );
        double id1 = hMedian->GetYaxis()->GetBinLowEdge( j + 1 );
        double id2 = hMedian->GetYaxis()->GetBinLowEdge( j + 1 ) + hMedian->GetYaxis()->GetBinWidth( j+1 );
        sprintf( histitle, "%.2f < log10 size < %.2f, %.1f < r < %.1f (%s)", is1, is2, id1, id2, fHName_Add.c_str() );
	Oh[i][j] = new TH1F(hisname,histitle,HistBins,fBinning1DXlow,fBinning1DXhigh);
	Oh[i][j]->SetXTitle( fName.c_str() );
   }
   else
   {
      return false;
   }
   return true;
}


void VTableCalculator::setConstants( bool iPE )
{
    NumSize = 55;
    amp_offset = 1.5;
    amp_delta = 0.1;
    NumDist = 80;
    dist_delta = 15.;
    HistBins = 500;
    xlow = 0.;
    xhigh = 1.;
    fMinShowerPerBin = 5.;

// binning is different for MC with values in PE
    if( iPE )
    {
       NumSize = 45;
       amp_offset = 1.0;
       amp_delta = 0.15;

       NumDist = 75;
       dist_delta = 20.;
       HistBins = 500;
    }
}


void VTableCalculator::terminate( TDirectory *iOut, char *xtitle )
{
    if( iOut != 0 ) fOutDir = iOut;

    if( !fOutDir->cd() )
    {
        cout << "Error: unable to reach writing directory ( VTableCalculator::terminate())" << endl;
        return;
    }

/////////////////////////////////////////////////////////////////////////////////////////////
// table writing
/////////////////////////////////////////////////////////////////////////////////////////////
    if( fwrite )
    {
        TDirectory *iDir1D = 0;
// make output directory for 1D histograms
        if( fOutDir )
        {
            iDir1D = fOutDir->mkdir( "histos1D" );
        }

///////////////////////////////////
// 2D histograms
// number of events
        char hname[1000];
        char htitle[1000];
        sprintf( hname, "%s_nevents_%s", fName.c_str(), fHName_Add.c_str() );
        sprintf( htitle, "%s vs. dist. vs. log10 size (# of events)", fName.c_str() );
        TH2F *hNevents = new TH2F( hname, htitle, NumSize, amp_offset, amp_offset+NumSize*amp_delta, NumDist, 0., dist_delta*NumDist );
        hNevents->SetXTitle( "log_{10} size" );
        hNevents->SetYTitle( "distance [m]" );
        hNevents->SetZTitle( "# of events/bin" );
// most probable of variable
        sprintf( hname, "%s_mpv_%s", fName.c_str(), fHName_Add.c_str() );
        sprintf( htitle, "%s vs. dist. vs. log10 size (mpv)", fName.c_str() );
        TH2F *hMPV = new TH2F( hname, htitle, NumSize, amp_offset, amp_offset+NumSize*amp_delta, NumDist, 0., dist_delta*NumDist );
        hMPV->SetXTitle( "log_{10} size" );
        hMPV->SetYTitle( "distance [m]" );
        if( !fEnergy ) sprintf( htitle, "%s (mpv) [deg]", fName.c_str() );
        else           sprintf( htitle, "%s (mpv) [TeV]", fName.c_str() );
        hMPV->SetZTitle( htitle );
// sigma of median (16-84% (2sigma for Gauss))
        sprintf( hname, "%s_sigma_%s", fName.c_str(), fHName_Add.c_str() );
        sprintf( htitle, "%s vs. dist. vs. log10 size (sigma)", fName.c_str() );
        TH2F *hSigma = new TH2F( hname, htitle, NumSize, amp_offset, amp_offset+NumSize*amp_delta, NumDist, 0., dist_delta*NumDist );
        hSigma->SetXTitle( "log_{10} size" );
        hSigma->SetYTitle( "distance [m]" );
        if( !fEnergy ) sprintf( htitle, "%s (2xsigma) [deg]", fName.c_str() );
        else           sprintf( htitle, "%s (2xsigma) [TeV]", fName.c_str() );
        hSigma->SetZTitle( htitle );

///////////////////////////////////
// EVALUATION OF HISTOGRAMS 
	cout << "\t msc tables: evaluating " << fName << " histograms ";

        float med = 0.;
	float sigma = 0.;
	int id = 0;

        char hisname[800];
        char histitle[800];

	double i_a[] = { 0.16, 0.5, 0.84 };
	double i_b[] = { 0.0,  0.0, 0.0  };

// loop over all size bin and distance bins
        for( int i = 0; i < NumSize; i++ )
        {
            for( int j = 0; j < NumDist; j++ )
            {
		if( !Oh[i][j] ) continue;

	        if( Oh[i][j]->GetEntries() > 5 )
		{
		   Oh[i][j]->GetQuantiles( 3, i_b, i_a );
		   med   = i_b[1];
		   sigma = i_b[2] - i_b[0];
                }
		else
		{
		   med = 0.;
		   sigma = 0.;
                }
                hMedian->SetBinContent( i+1, j+1, med );
		hMedian->SetBinError( i+1, j+1, sigma );
                hSigma->SetBinContent( i+1, j+1, sigma );
                if( Oh[i][j]->GetEntries() > 5 ) hNevents->SetBinContent( i+1, j+1, Oh[i][j]->GetEntries() );
		else if( fEnergy )
		{
		   hMean->SetBinContent( i+1, j+1, 0. );
                }
		if( fEnergy )
		{
		   fillMPV( hMPV, i+1, j+1, Oh[i][j], med, sigma );
		   hMPV->SetBinError( i+1, j+1, sigma );
                }

                id=i*1000+j;
                sprintf( hisname , "h%d",id);
                sprintf( histitle, "h%d",id);
// write 1D histograms to file
                if( fOutDir && fWrite1DHistograms && Oh[i][j]->GetEntries() > 0 )
                {
                    fOutDir->cd();
                    iDir1D->cd();
                    Oh[i][j]->Write();
                }
		delete Oh[i][j];
            }
        }
// write 2D histograms to file
        if( fOutDir && hNevents->GetEntries() > 0 )
        {
            fOutDir->cd();
            if( xtitle && hMedian )   hMedian->SetTitle( xtitle );
            if( xtitle && hMPV )      hMPV->SetTitle( xtitle );
            if( hNevents && hMedian ) hMedian->SetEntries( hNevents->GetEntries() );
            if( hNevents && hSigma )  hSigma->SetEntries( hNevents->GetEntries() );
// reduce size of the 2D histograms
            TH2F *h = 0;
	    string n = hMedian->GetName();
	    h = VHistogramUtilities::reduce2DHistogramSize( hMedian, n + "_new" );
	    if( h && hMedian )
	    {
               cout << "(" << hMedian->GetEntries() << " entries)";
	       delete hMedian;
	       if( h ) 
	       {
		  h->SetName( n.c_str() );
	          h->Write();
               }
	       delete h;
            }
	    n = hMPV->GetName();
	    h = VHistogramUtilities::reduce2DHistogramSize( hMPV, n + "_new" );
	    if( h && hMPV )
	    {
	       delete hMPV;
	       if( h )
	       {
		  h->SetName( n.c_str() );
	          h->Write();
               }
	       delete h;
            }
	    n = hSigma->GetName();
	    h = VHistogramUtilities::reduce2DHistogramSize( hSigma, n + "_new" );
	    if( h && hSigma )
	    {
	       delete hSigma;
	       if( h )
	       {
		  h->SetName( n.c_str() );
	          h->Write();
               }
	       delete h;
            }
	    n = hMean->GetName();
	    h = VHistogramUtilities::reduce2DHistogramSize( hMean, n + "_new" );
	    if( h && hMean )
	    {
	       delete hMean;
	       if( h )
	       {
		  h->SetName( n.c_str() );
	          h->Write();
               }
	       delete h;
            }
	    n = hNevents->GetName();
	    h = VHistogramUtilities::reduce2DHistogramSize( hNevents, n + "_new" );
	    if( h && hNevents )
	    {
	       delete hNevents;
	       if( h )
	       {
		  h->SetName( n.c_str() );
	          h->Write();
               }
	       delete h;
            }
            cout << endl;
        }
	else cout << "(no entries)" << endl;
    }
}


/*!

     Histogram filling

     \param ntel   number of telescopes
     \param r      core distance from each telescopes [ntel]
     \param s      size per telescopes [ntel]
     \param chi2   external weight (table filling only)
     \param dE      not used

     Width/length calculation:

\param w       width/length per telescope [ntel]
\param mt      expected width/length per telescope [ntel]
\param chi2    always 0 (meaningless)
\param dE      always 0 (meaningless)
\param st      expected sigma for width/length per telescope [ntel]

return value is mean scaled width/length

Energy calculation:

\param w       MCenergy [ntel] (is telescope independent, but easier to implement like that)
\param mt      lookup table energy per telescope [ntel]
\param chi2    scatter of energies per telescope
\param dE      (mean) energy resolution 

return value is energy (linear scale)

*/
double VTableCalculator::calc( int ntel, double *r, double *s, double *w, double *mt, double &chi2, double &dE, double *st )
{
    int tel = 0;

///////////////////////////////////////////////////////////////////////////////////////
// fill the tables
//
// ntel and size/width etc arrays are expected to be values from same telescope types
// therefore: ntel = number of telescopes of same type 
// (fill one mscw table for each telescope type)
///////////////////////////////////////////////////////////////////////////////////////
    if( fwrite )
    {
        int i_Oh_size = (int)Oh.size();
// don't allow zero or negative weights
        if( chi2 <= 0. ) return -99.;
// loop over all telescopes
        double i_logs = 0.;
        for( tel = 0; tel < ntel; tel++ )
        {
            if( s[tel] > 0. && r[tel] >= 0. && w[tel] > fBinning1DXlow && w[tel] < fBinning1DXhigh )
            {
// check limits (to avoid under/overflows)
		if( r[tel] > hMedian->GetYaxis()->GetXmax() ) continue;
                i_logs = log10( s[tel] );
                if( i_logs > hMedian->GetXaxis()->GetXmax() ) continue;


// calculate log energy (translate from TeV to GeV)
		int is = hMedian->GetXaxis()->FindBin( i_logs ) - 1;
		int ir = hMedian->GetYaxis()->FindBin( r[tel] ) - 1;
// reject showers in the first size bin
                if( ir >= 0 && is >= 0 && is < i_Oh_size && ir < (int)Oh[is].size() )
                {
		    if( !Oh[is][ir] )
		    {
		       if( !create1DHistogram( is, ir ) ) continue;
                    }
// fill width/length/energy into a 1D and 2D histogram
// (chi2 is here an external weight (from e.g. spectral weighting))
                    Oh[is][ir]->Fill( w[tel], chi2 );
                    hMean->Fill( i_logs, r[tel], w[tel] * chi2 );
                }
            }
        }
        return -99.;
    }
/////////////////////////////////////////////////////////
// END OF writing/filling lookup tables
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// read table
// compute mean or mean scaled width/length value
//
    else
    {

// tables are accessed for the first time: get the from the file
        if( !fReadHistogramsFromFile )
        {
            cout << "read tables from " << fOutDir->GetPath() << endl;
            hMedian = (TH2F*)fOutDir->Get( hMedianName.c_str() );
            fReadHistogramsFromFile = true;

            if( !hMedian )
            {
                cout << "VTableCalculator error: table histograms not found in " << gDirectory->GetName() << endl;
                exit( -1 );
            }
        }

        chi2 = 0.;
	dE   = 0.;
        double med = 0.;
        double sigma = 0.;
        double value = 0.;
        double weight = 0.;
// energy per telescope
        vector< double > energy_tel;
        vector< double > sigma2_tel;
	vector< double > sigma_tel;

// reset everything
        for( tel = 0; tel < ntel; tel++ )
        {
            mt[tel] = -99.;
            if( st ) st[tel] = -99.;
        }

////////////////////////////////////////////////////
// loop over all telescopes
////////////////////////////////////////////////////
        for( tel = 0; tel < ntel; tel++ )
        {
            if( r[tel] >= 0. && s[tel] > 0 )
            {
// get expected value and sigma of expected value
                    if( hMedian )
                    {
                        med   = interpolate( hMedian, log10( s[tel] ), r[tel], false );
			sigma = interpolate( hMedian, log10( s[tel] ), r[tel], true );
                    }
                    else if( hVMedian.size() == (unsigned int)ntel && hVMedian[tel] )
                    {
                        med   = interpolate( hVMedian[tel], log10( s[tel] ), r[tel], false );
                        sigma = interpolate( hVMedian[tel], log10( s[tel] ), r[tel], true );
			if( fDebug && fEnergy )
			{
			   cout << "\t  double VTableCalculator::calc() getting energy from table for tel " << tel;
			   cout << ", size " << s[tel];
			   cout << " , distance " << r[tel];
			   cout << endl;
                        } 
                    }
                    else
                    {
                        med = 0.;
                        sigma = 0.;
			if( fDebug )
			{
			   cout << "\t  double VTableCalculator::calc() med equal zero for tel " << tel;
			   cout << ", size " << s[tel] << ", distance " << r[tel] << endl;
                        } 
                    }
// accept only values > 0
// (expeced width/length should be > 0)
// (log10 energy should be > 0, good reason why we work in GeV here)
                    if( med > 0. )
                    {
                        mt[tel] = med;
			if( st ) st[tel] = sigma;
                    }
                    else
                    {
                        mt[tel] = -99.;
                        if( st ) st[tel] = -99.;
			if( fDebug )
			{
			   cout << "\t  double VTableCalculator::calc() med equal zero from tables for tel " << tel;
			   cout << ", size " << s[tel] << ", distance " << r[tel] << endl;
                        } 
                    }
// weighted mean
                    if( med > 0. )
                    {
///////////////////////////////////////////////////////////////////////////////////////////////////////
// mean scaled calculation
                        if( !fEnergy && sigma > 0. && w )
                        {
// handle showers with (width==0.) correctly 
			    if( w[tel] > 0. )
			    {
			       value  += (w[tel]-med)/sigma * (med*med)/(sigma*sigma);
                            }
			    weight += (med*med)/(sigma*sigma);
                        }
///////////////////////////////////////////////////////////////////////////////////////////////////////
// energy calculation
                        else if( fEnergy && sigma > 0. )
                        {
// store energy per telescope
			    energy_tel.push_back( med );
// store expected relative error
			    sigma_tel.push_back( sigma / med );
// use relative error as weighting (otherwise: significant bias towards lower energies
			    sigma2_tel.push_back( med/(sigma*sigma) );
// add addional weight for events inside or outside the light pool
			    if( r[tel] < 140. ) sigma2_tel.back() = sigma2_tel.back()*100.;
			    else                sigma2_tel.back() = sigma2_tel.back()*100.*exp( -1.*(r[tel]-140.)/50.);
                        }
			else
			{
			   chi2 = -99;
			   dE = -99.;
			   return -99.;
			} 
                    }
                }
        }
////////////////////////////////////////////////////////////////
// mean scaled value 
// (MSCW/MSCL)
        if( !fEnergy )
        {
            if( weight > 0 ) return value/weight;
            else return -99.;
        }
///////////////////////////////////////////////////////////////
// Energy calculation only
///////////////////////////////////////////////////////////////
// calculate mean energy
        if( energy_tel.size() > 0 )
        {
// Occasionally one energy is significantly off and distorts the mean.
// therefore: get rid of N sigma outliers
// use robust statistics (median and mean absolute error)
// Note: applied only to larger events > 4 telescopes
            double median = TMath::Median( energy_tel.size(), &energy_tel[0] );
	    double meanAbsoluteError = VStatistics::getMeanAbsoluteError( energy_tel );
	    weight = 0.;
	    for( unsigned int j = 0; j < energy_tel.size(); j++ )
	    {
	       if( energy_tel.size() < 5 || TMath::Abs( energy_tel[j] - median ) < meanAbsoluteError * 5 )
	       {
	          value  += energy_tel[j] * sigma2_tel[j];
		  weight += sigma2_tel[j];
               }
            }
	    if( weight > 0. ) value /= weight;

// loop over number if images with valid entries
            if( energy_tel.size() > 1 )
            {
                chi2 = 0.;
                double z1 = 0;
                for( unsigned int j = 0; j < energy_tel.size(); j++ )
                {
                    if( sigma2_tel[j] != 0. )
                    {
                        chi2 += ( value - energy_tel[j] ) * ( value - energy_tel[j] ) * sigma2_tel[j];
			z1++;
                    }
                }
                if( z1 > 1 ) chi2 /= (z1-1.);
                else         chi2  = -99.;
		dE   = 0.;
		z1 = 0.;
		for( unsigned int j = 0; j < sigma_tel.size(); j++ )
		{
		    dE += sigma_tel[j];
		    z1++;
                }
		if( z1 > 0. ) dE = dE / z1;
		else          dE = 0.;
// changed 2013/08/10
//                dE = sqrt( 1./weight );
            }
            else
	    {
	       chi2 = -99;
	         dE = -99.;
            }
            return value;
        }
        else return -99.;
    }

// should never reach this point
    return -99.;
}

void VTableCalculator::setInterpolationConstants( int iwidth, int iinter )
{
    fInterPolWidth = iwidth;
    fInterPolIter = iinter;
}


double VTableCalculator::getWeightMeanBinContent( TH2F *h, int ix0, int iy0, double x, double y )
{
    if( !h ) return 0.;

    if( h->GetBinContent( ix0, iy0 ) == 0. ) return 0.;

    double ibc = 0.;

// get bin centers
    double i_bc_x0 = h->GetXaxis()->GetBinCenter( ix0 );
    double i_bc_y0 = h->GetYaxis()->GetBinCenter( iy0 );
    int ix1 = ix0;
    int iy1 = iy0;
    if( x < i_bc_x0 && ix0 > 1 )        ix1 = ix0 - 1;
    else if( ix0 < h->GetNbinsX() - 1 ) ix1 = ix0 + 1;
    if( y < i_bc_y0 && iy0 > 1 )        iy1 = iy0 - 1;
    else if( iy0 < h->GetNbinsY() - 1 ) iy1 = iy0 + 1;
    double i_bc_x1 = h->GetXaxis()->GetBinCenter( ix1 );
    double i_bc_y1 = h->GetYaxis()->GetBinCenter( iy1 );

    double weight = 0.;
    double dist = 0.;

// first bin (x,y is inside this bin)
    dist = sqrt( (i_bc_x0-x)*(i_bc_x0-x) + (i_bc_y0-y)*(i_bc_y0-y) );
// return bin content if x,y is very close to bin center
    if( fabs( dist ) < 1.e-5 ) return h->GetBinContent( ix0, iy0 );
    dist = 1./dist;

    weight += dist;
    ibc += h->GetBinContent( ix0, iy0 ) * dist;

// second bin
    if( h->GetBinContent( ix1, iy0 ) > 0. )
    {
        dist = sqrt( (i_bc_x1-x)*(i_bc_x1-x) + (i_bc_y0-y)*(i_bc_y0-y) );
        if( dist > 0. )
        {
            weight += 1./dist;
            ibc += h->GetBinContent( ix1, iy0 ) / dist;
        }
    }

// third bin
    if( h->GetBinContent( ix1, iy1 ) > 0. && dist > 0. )
    {
        dist = sqrt( (i_bc_x1-x)*(i_bc_x1-x) + (i_bc_y1-y)*(i_bc_y1-y) );
        if( dist > 0. )
        {
            weight += 1./dist;
            ibc += h->GetBinContent( ix1, iy1 ) / dist;
        }
    }

// fourth bin
    if( h->GetBinContent( ix0, iy1 ) > 0. )
    {
        dist = sqrt( (i_bc_x0-x)*(i_bc_x0-x) + (i_bc_y1-y)*(i_bc_y1-y) );
        if( dist > 0. )
        {
            weight += 1./dist;
            ibc += h->GetBinContent( ix0, iy1 ) / dist;
        }
    }

    if( weight > 0. ) return ibc / weight;

    return 0.;
}


void VTableCalculator::setVHistograms( vector< TH2F* >& hM )
{
    hVMedian = hM;

    fReadHistogramsFromFile = true;
}


TH2F* VTableCalculator::getHistoMedian()
{
    if( !fReadHistogramsFromFile ) fReadHistogramsFromFile = readHistograms();

    return hMedian;
}


bool VTableCalculator::readHistograms()
{
    if( fOutDir )
    {
        hMedian = (TH2F*)fOutDir->Get( hMedianName.c_str() );

        if( hMedian ) return true;
	else          return false;
    }
    hMedian = 0;
    return false;
}

double VTableCalculator::interpolate( TH2F* h, double x, double y, bool iError )
{
   if( !h ) return 0.;

   int i_x = h->GetXaxis()->FindBin( x );
   int i_y = h->GetYaxis()->FindBin( y );
// handle under and overflows ( bin nBinsX+1 is needed)
   if( i_x == 0 || i_y == 0 || i_x == h->GetNbinsX() || i_y == h->GetNbinsY() )
   {
      if( iError ) return h->GetBinError( i_x, i_y );
      else         return h->GetBinContent( i_x, i_y );
   }
   if( x < h->GetXaxis()->GetBinCenter( i_x ) ) i_x--;
   if( y < h->GetYaxis()->GetBinCenter( i_y ) ) i_y--;

   double e1 = 0.;
   double e2 = 0.;
   double v = 0.;

// first interpolate on distance axis, then on size axis
   if( !iError )
   {
      e1 = VStatistics::interpolate( h->GetBinContent( i_x, i_y ), h->GetYaxis()->GetBinCenter( i_y ),
				     h->GetBinContent( i_x, i_y+1 ), h->GetYaxis()->GetBinCenter( i_y + 1 ),
				     y, false, 0.5, 1.e-5 );
      e2 = VStatistics::interpolate( h->GetBinContent( i_x+1, i_y ), h->GetYaxis()->GetBinCenter( i_y ),
				     h->GetBinContent( i_x+1, i_y+1 ), h->GetYaxis()->GetBinCenter( i_y + 1 ),
				     y, false, 0.5, 1.e-5 );
      v = VStatistics::interpolate( e1, h->GetXaxis()->GetBinCenter( i_x ),
                                    e2, h->GetXaxis()->GetBinCenter( i_x + 1 ),
				    x, false, 0.5, 1.e-5 );
   }
   else
   {
      e1 = VStatistics::interpolate( h->GetBinError( i_x, i_y ), h->GetYaxis()->GetBinCenter( i_y ),
				     h->GetBinError( i_x, i_y+1 ), h->GetYaxis()->GetBinCenter( i_y + 1 ),
				     y, false );
      e2 = VStatistics::interpolate( h->GetBinError( i_x+1, i_y ), h->GetYaxis()->GetBinCenter( i_y ),
				     h->GetBinError( i_x+1, i_y+1 ), h->GetYaxis()->GetBinCenter( i_y + 1 ),
				     y, false );
      
      v = VStatistics::interpolate( e1, h->GetXaxis()->GetBinCenter( i_x ), e2, h->GetXaxis()->GetBinCenter( i_x + 1 ), x, false );
   }
// final check on consistency of results 
// (don't expect to reconstruct anything below 1 GeV)
   if( e1 > 1.e-3 && e2 < 1.e-3 ) return e1;
   if( e1 < 1.e-3 && e2 > 1.e-3 ) return e2;

   return v;
}

/*

     search most probable value of energy distribution for a give size/radius bin

     (these distributions are at low energies often very skewed)

*/
void VTableCalculator::fillMPV( TH2F *h, int i, int j, TH1F *h1D, double iMedianValue, double iSigmaValue )
{
   if( !h || !h1D ) return;

// only fit well filled histograms -> otherwise will median
   if( h1D->GetEntries() <= 50 || iMedianValue <= 0. ) 
   {
       h->SetBinContent( i, j, iMedianValue );
       return;
   }
// don't do anything if difference between mean and median is <15%
   if( iMedianValue > 0. && TMath::Abs( (iMedianValue-h1D->GetMean())/iMedianValue ) < 0.15 )
   {
      h->SetBinContent( i, j, iMedianValue );
      return;
   }

/////////////////////////////////////////
// try a Landau fit
   TF1 iLandau( "iLandau", "TMath::Landau(x,[0],[1],0)*[2]", iMedianValue/3., iMedianValue*3. );
   iLandau.SetParameters( iMedianValue, iSigmaValue, h1D->GetEntries() );
// do not allow the most probably to more than x3 off the median
   iLandau.SetParLimits( 0, iMedianValue/3., iMedianValue*3. ); 
   h1D->Fit( &iLandau, "QMNR" );
// require >10% fit probability to use Landau most probable value
   if( TMath::Prob( iLandau.GetChisquare(), iLandau.GetNDF() ) > 0.1
    && iLandau.GetParameter( 0 ) > 0. && iMedianValue / iLandau.GetParameter( 0 ) < 2.5 )
   {
      h->SetBinContent( i, j, iLandau.GetParameter( 0 ) );

      if( fDebug )
      {
	 cout << "\t\t Landau interpolation for energy tables: " << i << "\t" << j << "\t";
	 cout << TMath::Prob( iLandau.GetChisquare(), iLandau.GetNDF() ) << ", median " << iMedianValue;
	 cout << ", fit: " << iLandau.GetParameter( 0 ) << "\t" << iLandau.GetParameter( 1 );
	 cout << "\t" << iMedianValue / iLandau.GetParameter( 0 );
	 cout << "\t" << iLandau.GetParameter( 0 ) /  iLandau.GetParameter( 1 ) << endl;
      }
   }
   else
   {
     h->SetBinContent( i, j, iMedianValue ); 
   }

}

