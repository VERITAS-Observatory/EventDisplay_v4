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

    for( int i = 0; i < intel; i++ )
    {
        hVMedian.push_back( 0 );
        hVSigma.push_back( 0 );
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


VTableCalculator::VTableCalculator( vector< TH2F* > iMedian, vector< TH2F* > iSigma, bool iEnergy, bool iPE )
{
    setDebug();

    setConstants( iPE );
// using lookup tables to calculate energies
    fEnergy = iEnergy;

    hMedian = 0;
    hSigma = 0;
    hMean = 0;
    hNevents = 0;

    hVMedian = iMedian;
    hVSigma = iSigma;

    Omode = 'r';

    fReadHistogramsFromFile = false;
}


/* CREATOR */
VTableCalculator::VTableCalculator( string fpara, string hname_add, char m, TDirectory *iDir, bool iEnergy, string iInterpolate, bool iPE )
{
    setDebug();

    setConstants( iPE );
// using lookup tables to calculate energies
    fEnergy = iEnergy;
    fReadHistogramsFromFile = false;

    fWrite1DHistograms = true;

    fName = fpara;

    fInterPolWidth = 1;
    fInterPolIter = 3;

    fBinning1DXlow = 0.;
    fBinning1DXhigh = 1.;
    if( fEnergy )
    {
        fBinning1DXlow =  1.;
        fBinning1DXhigh = 5.;
    }

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

    int i,j;
    char hname[1000];

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
        else           sprintf( htitle, "log_{10} %s (median) [GeV]", fpara.c_str() );
        hMedian->SetZTitle( htitle );
// sigma of median (16-84% (2sigma for Gauss))
        sprintf( hname, "%s_sigma_%s", fpara.c_str(), hname_add.c_str() );
        sprintf( htitle, "%s vs. dist. vs. log10 size (sigma)", fpara.c_str() );
        hSigma = new TH2F( hname, htitle, NumSize, amp_offset, amp_offset+NumSize*amp_delta, NumDist, 0., dist_delta*NumDist );
        hSigma->SetXTitle( "log_{10} size" );
        hSigma->SetYTitle( "distance [m]" );
        if( !fEnergy ) sprintf( htitle, "%s (2xsigma) [deg]", fpara.c_str() );
        else           sprintf( htitle, "log_{10} %s (2xsigma) [GeV]", fpara.c_str() );
        hSigma->SetZTitle( htitle );
// mean and rms
        sprintf( hname, "%s_mean_%s", fpara.c_str(), hname_add.c_str() );
        sprintf( htitle, "%s vs. dist. vs. log10 size (mean)", fpara.c_str() );
        hMean = new TProfile2D( hname, htitle, NumSize, amp_offset, amp_offset+NumSize*amp_delta, NumDist, 0., dist_delta*NumDist, fBinning1DXlow, fBinning1DXhigh );
        hMean->SetXTitle( "log_{10} size" );
        hMean->SetYTitle( "distance [m]" );
        if( !fEnergy ) sprintf( htitle, "%s (mean) [deg]", fpara.c_str() );
        else           sprintf( htitle, "log_{10} %s (mean) [GeV]", fpara.c_str() );
        hMean->SetZTitle( fpara.c_str() );
// number of events
        sprintf( hname, "%s_nevents_%s", fpara.c_str(), hname_add.c_str() );
        sprintf( htitle, "%s vs. dist. vs. log10 size (# of events)", fpara.c_str() );
        hNevents = new TH2F( hname, htitle, NumSize, amp_offset, amp_offset+NumSize*amp_delta, NumDist, 0., dist_delta*NumDist );
        hNevents->SetXTitle( "log_{10} size" );
        hNevents->SetYTitle( "distance [m]" );
        if( !fEnergy ) sprintf( htitle, "%s (2xsigma) [deg]", fpara.c_str() );
        else           sprintf( htitle, "log_{10} %s (2xsigma) [GeV]", fpara.c_str() );
        hNevents->SetZTitle( fpara.c_str() );
// 1d histograms for variable distribution
        char hisname[200];
        char histitle[200];
        int id;
        double is1, is2;
        double id1, id2;
        for (i=0;i<NumSize;i++)
        {
            vector< TH1F* > iH1;
            for (j=0;j<NumDist;j++)
            {
                id=i*1000+j;
                sprintf( hisname , "h%d",id);
                is1 = amp_offset + i*amp_delta;
                is2 = amp_offset + (i+1)*amp_delta;
                id1 = j*dist_delta;
                id2 = (j+1)*dist_delta;
                sprintf( histitle, "%.2f < log10 size < %.2f, %.1f < r < %.1f (%s)", is1, is2, id1, id2, hname_add.c_str() );
                iH1.push_back( new TH1F(hisname,histitle,HistBins,fBinning1DXlow,fBinning1DXhigh) );
                if( !fEnergy ) sprintf( histitle, "%s [deg]", fpara.c_str() );
                else           sprintf( histitle, "log_{10} %s [GeV]", fpara.c_str() );
                iH1.back()->SetXTitle( histitle );
            }
            Oh.push_back( iH1 );
        }
    }
/////////////////////////////////////////
// table reading
/////////////////////////////////////////
    else
    {
        fReadHistogramsFromFile = false;

        sprintf( hname, "%s_median_%s", fpara.c_str(), hname_add.c_str() );
        hMedianName = hname;
        sprintf( hname, "%s_sigma_%s", fpara.c_str(), hname_add.c_str() );
        hSigmaName = hname;
	sprintf( hname, "hNevents_energy_%s", hname_add.c_str() );
	hNeventsName = hname;
	sprintf( hname, "hMean_energy_%s", hname_add.c_str() );
	hMeanName = hname;
    }

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

        int i,j,k,id;
        float sum1,sum2,delta,med,sigma;
        int i1,i2,i3;

        char hisname[800];
        char histitle[800];

        for( i = 0; i < NumSize; i++ )
        {
            for( j = 0; j < NumDist; j++ )
            {
                sum1=0.;
// but ignore first bin (zero bin)
                for( k = 2; k < HistBins; k++ )
                {
                    sum1 += Oh[i][j]->GetBinContent(k);
                }

// require at least XX showers per bin
                if( sum1 > fMinShowerPerBin  )
                {
                    sum2 = 0.;
                    i1=i2=i3=0;
// calculate median and rms
// but ignore first bin (zero bin)
                    for( k = 2; k < HistBins - 2; k++ )
                    {
                        sum2 += Oh[i][j]->GetBinContent(k);
                        if( sum2 < 0.16 * sum1 ) i1=k;
                        if( sum2 < 0.50 * sum1 ) i2=k;
                        if( sum2 < 0.84 * sum1 ) i3=k;
                    }
                    delta = (fBinning1DXhigh-fBinning1DXlow)/(float)HistBins;

// the best estimate for the true median value lies in between the
// bin centers of the i2'th and i2+1'th bin -
// that points happens to be at i2+1 * delta.
                    med   = fBinning1DXlow + (i2+1) * delta;
                    sigma = (i3-i1)*delta;
                }
                else
                {
                    med  =0.;
                    sigma=0.;
                }
                hMedian->SetBinContent( i+1, j+1, med );
                hSigma->SetBinContent( i+1, j+1, sigma );
                if( Oh[i][j]->GetEntries() > 5 ) hNevents->SetBinContent( i+1, j+1, Oh[i][j]->GetEntries() );

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
            if( xtitle && hMedian ) hMedian->SetTitle( xtitle );
            if( hNevents && hMedian ) hMedian->SetEntries( hNevents->GetEntries() );
            if( hNevents && hSigma )  hSigma->SetEntries( hNevents->GetEntries() );
            if( hMedian ) hMedian->Write();
            if( hSigma ) hSigma->Write();
            if( hMean ) hMean->Write();
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
    int ir = 0;
    int is = 0;

///////////////////////////////////////////////////////////////////////////////////////
// fill the tables
//
// ntel and size/width etc arrays are expected to be values from same telescope types
// therefore: ntel = number of telescopes of same type 
// (fill one mscw table for each telescope type)
///////////////////////////////////////////////////////////////////////////////////////
    if( Omode=='w' || Omode=='W' )
    {
        for( tel = 0; tel < ntel; tel++ )
        {
            if( r[tel] >= 0. )
            {
// this cut only applies to mscw/mscl calculations (remove images with width/length 0)
                if( !fEnergy && w[tel] < 1.e-3 ) continue;

// calculate log energy (translate from TeV to GeV)
                if( fEnergy && w[tel] > 0. ) w[tel] = log10( w[tel] ) + 3.;

                is = SizeIndex(s[tel]);
                ir = DistIndex(r[tel]);
// reject showers in the first size bin
                if( ir >= 0 && is > 0 )
                {
                    Oh[is][ir]->Fill( w[tel], chi2 );
                    double id = r[tel];
                    if( id > NumDist*dist_delta ) id = NumDist*dist_delta-0.1;
// chi2 is here an external weight (from e.g. spectral weighting)
                    hMean->Fill( log10(s[tel]), id, w[tel] * chi2 );
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
            hSigma = (TH2F*)fOutDir->Get( hSigmaName.c_str() );
            fReadHistogramsFromFile = true;

            if( !hMedian || !hSigma )
            {
                cout << "VTableCalculator error: table histograms not found in " << gDirectory->GetName() << endl;
                exit( -1 );
            }
        }

        chi2 = 0.;
	dE   = 0.;
        int k = 0;
        double med = 0.;
        double sigma = 0.;
        double value = 0.;
        double weight = 0.;
// energy per telescope
        double a[VDST_MAXTELESCOPES];
        double b[VDST_MAXTELESCOPES];

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
                ir = 0;
		is = 0;
		if( hMedian && hSigma )
		{
		     is = hMedian->GetXaxis()->FindBin( log10( s[tel] ) );
		     ir = hMedian->GetYaxis()->FindBin( r[tel] ); 
		}
		else if( hVMedian.size() == (unsigned int)ntel && hVSigma.size() == (unsigned int)ntel && hVMedian[tel] && hVSigma[tel] )
                {
		     is = hVMedian[tel]->GetXaxis()->FindBin( log10( s[tel] ) );
		     ir = hVMedian[tel]->GetYaxis()->FindBin( r[tel] ); 
                }

// reject showers in the first size bin
                if( ir > 0 && is > 0 )
                {
// get expected value and sigma of expected value
                    if( hMedian && hSigma )
                    {
                        med = hMedian->GetBinContent( is, ir );
                        sigma = hSigma->GetBinContent( is, ir );
                    }
                    else if( hVMedian.size() == (unsigned int)ntel && hVSigma.size() == (unsigned int)ntel && hVMedian[tel] && hVSigma[tel] )
                    {
                        med   = hVMedian[tel]->GetBinContent( is, ir );
                        sigma = hVSigma[tel]->GetBinContent( is, ir );
			if( fDebug && fEnergy )
			{
			   cout << "\t  double VTableCalculator::calc() getting energy from table for tel " << tel;
			   cout << ", size " << s[tel] << " (bin " << is << ")";
			   cout << ", distance " << r[tel] << " (bin " << ir << ")";
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
// GeV -> TeV
                        if( fEnergy ) mt[tel] = pow(10.,med-3.);
                        else
                        {
                            mt[tel] = med;
                            if( st ) st[tel] = sigma;
                        }
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
                            k++;
                        }
///////////////////////////////////////////////////////////////////////////////////////////////////////
// energy calculation
                        else
                        {
                            if( sigma > 0. )
                            {
                                value  += med / sigma / sigma;
                                weight += 1./sigma/sigma;
                            }
                            else
                            {
                                value  += med;
                                weight += 1.;
                            }
// store energy per telescope
                            a[k] = med;
                            b[k] = sigma;
                            k++;
                        }
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
        if( weight > 0. )
        {
            value /= weight ;

// loop over number if images with valid entries
            if( k > 1 )
            {
                chi2 = 0.;
		dE   = 0.;
                double z1 = 0;
//////////////////////////////////////////////////////////////////////
// OLD dE definition
//                int z2 = 0;
//		double lin_value = TMath::Power( 10., value );
//////////////////////////////////////////////////////////////////////
                for( int j = 0; j < k; j++ )
                {
                    if( b[j] != 0. )
                    {
                        chi2 += ( value - a[j] ) * ( value - a[j] ) / b[j] / b[j];
			z1++;
                    }
//////////////////////////////////////////////////////////////////////
// OLD dE definition
//		    if( lin_value > 0. )
//		    {
//			dE    += ( TMath::Power( 10., a[j] ) - lin_value ) / lin_value;
//                        z2++;
//                    }
//////////////////////////////////////////////////////////////////////
                }
                if( z1 > 1 ) chi2 /= (z1-1.);
                else         chi2  = -99.;
//////////////////////////////////////////////////////////////////////
// OLD dE definition
//		if( z2 > 0 ) dE /=  z2;              // mean value: divide by # of participating telescopes
//		else         dE  = -99.;
// NEW dE definition
                dE = sqrt( 1./weight );
            }
            else
	    {
	       chi2 = -99;
	         dE = -99.;
            }

// internal work with GeV, translate here to TeV
            return (pow(10.,(value-3.)));
        }
        else return -99.;
    }

// should never reach this point
    return -99.;
}


/* compute size index */
int VTableCalculator::SizeIndex(double size)
{
    int i = (int)((log10(size)-amp_offset)/amp_delta);
    if (i>NumSize-1) i=-1;
    if (i<0) i=0;
    return i;
}


/* compute distance from telescopes index */
int VTableCalculator::DistIndex(double dist)
{
    int j = (int)(dist / dist_delta);

    if (j>NumDist-1) j=-1;

    if (j<0) j=0;

    return j;
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


void VTableCalculator::setVHistograms( vector< TH2F* >& hM, vector< TH2F* >& hS )
{
    hVMedian = hM;
    hVSigma  = hS;

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
        hSigma = (TH2F*)fOutDir->Get( hSigmaName.c_str() );
	hNevents = (TH2F*)fOutDir->Get( hNeventsName.c_str() );
	hMean = (TProfile2D*)fOutDir->Get( hMeanName.c_str() );

        return true;
    }
    if( !hMedian || !hSigma )
    {
        cout << "VTableCalculator error: table histograms not found in " << fOutDir->GetPath() << endl;
        exit( -1 );
    }
    return false;
}
