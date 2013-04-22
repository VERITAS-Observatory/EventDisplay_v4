/*! \file VTableCalculator.cpp
    \brief  general lookup table class (e.g. mscw, mscl, energy)

    Revision $Id$

*/

#include "VTableCalculator.h"

VTableCalculator::VTableCalculator( int intel, bool iEnergy, bool iPE )
{
    setDebug();

    setConstants( iPE );

    if( intel == 0 ) return;

    fEnergy = iEnergy;
    fUseMedianEnergy = false;

    for( int i = 0; i < intel; i++ )
    {
        hVMedian.push_back( 0 );
    }
    hMedian = 0;
    hSigma = 0;
    hMean = 0;
    hNevents = 0;

    Omode = 'r';

    fInterPolWidth = 1;
    fInterPolIter = 3;

    fBinning1DXlow = 0.;
    fBinning1DXhigh = 1.;
    if( fEnergy )
    {
        fBinning1DXlow =  1.;
        fBinning1DXhigh = 5.;
    }

    fReadHistogramsFromFile = false;

}


/* CREATOR */
VTableCalculator::VTableCalculator( string fpara, string hname_add, char m, TDirectory *iDir, bool iEnergy, string iInterpolate, bool iPE, bool iUseMedianEnergy )
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

    int i = 0;
    int j = 0;
    char hname[1000];

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// table writing
    if ((Omode=='w')||(Omode=='W'))
    {
        if( !fOutDir->IsWritable() )
        {
            cout << "VTableCalculator: error data directory in root file not writable" << endl;
            exit( -1 );
        }

/* HSTOGRAM BOOKING */
        char htitle[1000];
// median of variable
        sprintf( hname, "%s_median_%s", fpara.c_str(), hname_add.c_str() );
        sprintf( htitle, "%s vs. dist. vs. log10 size (median)", fpara.c_str() );
        hMedian = new TH2F( hname, htitle, NumSize, amp_offset, amp_offset+NumSize*amp_delta, NumDist, 0., dist_delta*NumDist );
        hMedian->SetXTitle( "log_{10} size" );
        hMedian->SetYTitle( "distance [m]" );
        if( !fEnergy ) sprintf( htitle, "%s (median) [deg]", fpara.c_str() );
        else           sprintf( htitle, "%s (median) [TeV]", fpara.c_str() );
        hMedian->SetZTitle( htitle );
// sigma of median (16-84% (2sigma for Gauss))
        sprintf( hname, "%s_sigma_%s", fpara.c_str(), hname_add.c_str() );
        sprintf( htitle, "%s vs. dist. vs. log10 size (sigma)", fpara.c_str() );
        hSigma = new TH2F( hname, htitle, NumSize, amp_offset, amp_offset+NumSize*amp_delta, NumDist, 0., dist_delta*NumDist );
        hSigma->SetXTitle( "log_{10} size" );
        hSigma->SetYTitle( "distance [m]" );
        if( !fEnergy ) sprintf( htitle, "%s (2xsigma) [deg]", fpara.c_str() );
        else           sprintf( htitle, "%s (2xsigma) [TeV]", fpara.c_str() );
        hSigma->SetZTitle( htitle );
// mean and rms
        sprintf( hname, "%s_mean_%s", fpara.c_str(), hname_add.c_str() );
        sprintf( htitle, "%s vs. dist. vs. log10 size (mean)", fpara.c_str() );
        hMean = new TProfile2D( hname, htitle, NumSize, amp_offset, amp_offset+NumSize*amp_delta, NumDist, 0., dist_delta*NumDist, fBinning1DXlow, fBinning1DXhigh );
        hMean->SetXTitle( "log_{10} size" );
        hMean->SetYTitle( "distance [m]" );
        if( !fEnergy ) sprintf( htitle, "%s (mean) [deg]", fpara.c_str() );
        else           sprintf( htitle, "%s (mean) [TeV]", fpara.c_str() );
        hMean->SetZTitle( htitle );
// number of events
        sprintf( hname, "%s_nevents_%s", fpara.c_str(), hname_add.c_str() );
        sprintf( htitle, "%s vs. dist. vs. log10 size (# of events)", fpara.c_str() );
        hNevents = new TH2F( hname, htitle, NumSize, amp_offset, amp_offset+NumSize*amp_delta, NumDist, 0., dist_delta*NumDist );
        hNevents->SetXTitle( "log_{10} size" );
        hNevents->SetYTitle( "distance [m]" );
        hNevents->SetZTitle( "# of events/bin" );
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

	if( fUseMedianEnergy )
	{
	   sprintf( hname, "%s_median_%s", fpara.c_str(), hname_add.c_str() );
	   hMedianName = hname;
        }
	else
	{
	   sprintf( hname, "%s_mean_%s", fpara.c_str(), hname_add.c_str() );
	   hMedianName = hname;
        }
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

       NumDist = 100;
       dist_delta = 20.;
       HistBins = 500;
    }
}


/* Total Destruction! */
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
    if( Omode=='w' || Omode=='W' )
    {
        TDirectory *iDir1D = 0;
// make output directory for 1D histograms
        if( fOutDir )
        {
            iDir1D = fOutDir->mkdir( "histos1D" );
        }

/* EVALUATION OF HISTOGRAMS */

        float med = 0.;
	float sigma = 0.;
	int id = 0;

        char hisname[800];
        char histitle[800];

	double i_a[] = { 0.16, 0.5, 0.84 };
	double i_b[] = { 0.0,  0.0, 0.0  };

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

                id=i*1000+j;
                sprintf( hisname , "h%d",id);
                sprintf( histitle, "h%d",id);
// write 1D histograms to file
                if( fOutDir && fWrite1DHistograms )
                {
                    fOutDir->cd();
                    iDir1D->cd();
                    Oh[i][j]->Write();
                }
            }
        }
// write 2D histograms to file
        if( fOutDir )
        {
            cout << "\t msc tables: evaluating " << fName << " histograms ";
            fOutDir->cd();
            if( xtitle && hMedian )   hMedian->SetTitle( xtitle );
            if( hNevents && hMedian ) hMedian->SetEntries( hNevents->GetEntries() );
            if( hNevents && hSigma )  hSigma->SetEntries( hNevents->GetEntries() );
            if( hMedian )  hMedian->Write();
            if( hSigma )   hSigma->Write();
            if( hMean )    hMean->Write();
            if( hNevents ) hNevents->Write();
            if( hMedian ) cout << "(" << hMedian->GetEntries() << " entries)";
            cout << endl;
        }
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
    if( Omode=='w' || Omode=='W' )
    {
// don't allow zero or negative weights
        if( chi2 <= 0. ) return -99.;
// loop over all telescopes
        for( tel = 0; tel < ntel; tel++ )
        {
            if( r[tel] >= 0. && s[tel] > 0. )
            {
// remove images with width/length 0, invalid energies
                if( w[tel] < 1.e-5 ) continue;
// check limits (to avoid under/overflows)
                if( log10( s[tel] ) > hMedian->GetXaxis()->GetXmax() ) continue;
                if( log10( s[tel] ) < hMedian->GetXaxis()->GetXmin() ) continue;
		if( r[tel] > hMedian->GetYaxis()->GetXmax() ) continue;
		if( r[tel] < hMedian->GetYaxis()->GetXmin() ) continue;

// calculate log energy (translate from TeV to GeV)
		int is = hMedian->GetXaxis()->FindBin( log10( s[tel] ) ) - 1;
		int ir = hMedian->GetYaxis()->FindBin( r[tel] ) - 1;
// reject showers in the first size bin
                if( ir >= 0 && is >= 0 && is < (int)Oh.size() && ir < (int)Oh[is].size() )
                {
		    if( !Oh[is][ir] )
		    {
		       if( !create1DHistogram( is, ir ) ) continue;
                    }
// check limits (to avoid under/overflows)
                    if( w[tel] < Oh[is][ir]->GetXaxis()->GetXmin() || w[tel] > Oh[is][ir]->GetXaxis()->GetXmax() ) continue;
// fill width/length/energy into a 1D and 2D histogram
// (chi2 is here an external weight (from e.g. spectral weighting))
                    Oh[is][ir]->Fill( w[tel], chi2 );
                    hMean->Fill( log10(s[tel]), r[tel], w[tel] * chi2 );
                }
		else
		{
		   cout << "VTableCalculator::calc(): warning index out of range: ";
		   cout << is << "\t" << ir << "\t";
		   cout << Oh.size() << "\t";
		   if( is >= 0 && is < (int)Oh.size() ) cout << Oh[is].size();
		   cout << endl;
                }
            }
        }
        return -99.;
    }
////////////////////////////
    else
    {
/////////////////////////////////////////////////////////
// read table
// compute mean or mean scaled width/length value
//

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
			    sigma2_tel.push_back( 1./(sigma*sigma) );
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
		dE   = 0.;
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
                dE = sqrt( 1./weight );
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


TH2F* VTableCalculator::getHistoSigma()
{
    if( !fReadHistogramsFromFile ) fReadHistogramsFromFile = readHistograms();

    return hSigma;
}


bool VTableCalculator::readHistograms()
{
    if( fOutDir )
    {
        hMedian = (TH2F*)fOutDir->Get( hMedianName.c_str() );

        return true;
    }
    if( !hMedian )
    {
        cout << "VTableCalculator error: table histograms not found in " << fOutDir->GetPath() << endl;
        exit( -1 );
    }
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
